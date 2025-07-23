import subprocess
import glob
import argparse
import os
import re
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

def sanitize_array_name(filename):
    base = os.path.splitext(os.path.basename(filename))[0]
    sanitized = re.sub(r'\W', '_', base)
    return sanitized

def fix_array_names(xxd_output, new_name):
    match = re.search(r"unsigned char (\w+)\[\] = {", xxd_output)
    if not match:
        return xxd_output
    old_name = match.group(1)
    fixed_output = re.sub(rf"\b{re.escape(old_name)}\b", new_name, xxd_output)
    return fixed_output

def main():
    parser = argparse.ArgumentParser(
        description="Convert .raw* files to a single C++ header file for use in a drum machine.",
        epilog="Use --info to view detailed instructions on preparing your audio files."
    )
    parser.add_argument("--input", "-i", type=str, default=".",
                        help="Input directory containing .raw* files (default: current directory)")
    parser.add_argument("--output", "-o", type=str,
                        help="Output .hpp file path. If omitted, will be derived from drum machine name.")
    parser.add_argument("--info", action="store_true",
                        help="Show detailed instructions for preparing your audio files")
    args = parser.parse_args()

    if args.info:
        print(INSTRUCTIONS)
        sys.exit(0)

    drum_name = input("Drum Machine Name? ").strip()
    clean_name = drum_name.replace("_", "")
    output_file = args.output or os.path.join(os.getcwd(), f"{clean_name}Samples.hpp")

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
        out_f.write(f"// {drum_name} Samples:\n")
        out_f.write("//\n")
        for i, name in enumerate(simplified_names, start=1):
            out_f.write(f"// {i}. {name}\n")
        out_f.write(f"//\n// Total # of Samples = {len(files)}\n\n")

        for f in files:
            result = subprocess.run(["xxd", "-i", f], capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error converting file {f}: {result.stderr}")
                continue
            simple_name = sanitize_array_name(f)
            fixed_output = fix_array_names(result.stdout, simple_name)
            out_f.write(fixed_output + "\n")

    print(f"Samples written to {output_file}")

if __name__ == "__main__":
    main()
