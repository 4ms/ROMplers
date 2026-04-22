import argparse
import glob
import os
import re
import struct
import sys

# Detailed user instructions for audio file preparation
INSTRUCTIONS = """
Input File Requirements:
- Files must be 44.1kHz / 16-bit **signed** PCM RAW WAV files.
- Use Audacity (or similar) to export files with the following settings:
    • Normalize all audio to -1 dB before export.
    • Trim unnecessary silence or length.
    • Add fade-outs to the ends of files.
    • Ensure the file starts at a zero crossing at the beginning of the transient.

How to Export in Audacity:
1. File > Export > Export Audio...
2. Format: "Other uncompressed files"
3. Header: "RAW (header-less)"
4. Encoding: "**Signed** 16-bit PCM"
"""

# Int16 literals per line in the generated array. xxd defaults to 12 bytes/line;
# 12 int16 values per line is a similar visual density.
VALUES_PER_LINE = 12


def sanitize_array_name(filename):
    base = os.path.splitext(os.path.basename(filename))[0]
    return re.sub(r'\W', '_', base)


def raw_to_int16_array(raw_path, array_name):
    """Read a 16-bit little-endian signed PCM file and emit a C++ int16_t[]
    array declaration. Declaring the backing storage as int16_t (rather than
    unsigned char) means Sample::operator[] gets a naturally-aligned halfword
    load with no UB and no need for alignas."""
    with open(raw_path, "rb") as f:
        raw = f.read()

    count = len(raw) // 2
    if count * 2 != len(raw):
        print(f"Warning: {raw_path} has an odd byte count; dropping trailing byte.")
    values = struct.unpack(f"<{count}h", raw[:count * 2])

    lines = [f"inline constexpr int16_t {array_name}[] = {{"]
    for i in range(0, len(values), VALUES_PER_LINE):
        chunk = values[i:i + VALUES_PER_LINE]
        formatted = ", ".join(f"{v:6d}" for v in chunk)
        lines.append(f"  {formatted},")
    lines.append("};")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Convert .raw* files to a single C++ header file for use in a drum machine.",
        epilog="Use --info to view detailed instructions on preparing your audio files."
    )
    parser.add_argument("--input", "-i", type=str, default=".",
                        help="Input directory containing .raw* files (default: current directory)")
    parser.add_argument("--output", "-o", type=str,
                        help="Output directory where the .hpp file will be saved")
    parser.add_argument("--prefix", "-p", type=str, default="",
                        help="Prefix prepended to each generated array name (e.g. 'SK' -> 'SKKick')")
    parser.add_argument("--info", action="store_true",
                        help="Show detailed instructions for preparing your audio files")
    args = parser.parse_args()

    if args.info:
        print(INSTRUCTIONS)
        sys.exit(0)

    drum_name = input("Drum Machine Name? ").strip()
    clean_name = drum_name.replace("_", "")

    # Determine output file path
    if args.output:
        output_dir = args.output
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        output_file = os.path.join(output_dir, f"{clean_name}Samples.hpp")
    else:
        output_file = os.path.join(os.getcwd(), f"{clean_name}Samples.hpp")

    input_dir = os.path.abspath(args.input)
    if not os.path.isdir(input_dir):
        print(f"Input directory '{input_dir}' does not exist.")
        return

    pattern = os.path.join(input_dir, "*.raw*")
    files = glob.glob(pattern)
    if not files:
        print(f"No .raw* files found in '{input_dir}'.")
        return

    simplified_names = []
    for filepath in files:
        basename = os.path.basename(filepath)
        # Remove .raw or .raw.* suffix
        name = re.sub(r"\.raw(\..*)?$", "", basename, flags=re.IGNORECASE)
        simplified_names.append(name)

    with open(output_file, "w") as out_f:
        out_f.write("#include <cstdint>\n")
        out_f.write(f"// {drum_name} Samples:\n")
        out_f.write("//\n")
        for i, name in enumerate(simplified_names, start=1):
            out_f.write(f"// {i}. {name}\n")
        out_f.write(f"//\n// Total # of Samples = {len(files)}\n\n")

        for f in files:
            array_name = args.prefix + sanitize_array_name(f)
            out_f.write(raw_to_int16_array(f, array_name) + "\n\n")

    print(f"Samples written to {output_file}")


if __name__ == "__main__":
    main()
