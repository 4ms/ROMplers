import subprocess
import glob
import argparse
import os
import re

def sanitize_array_name(filename):
    # Remove extension and non-alphanumeric characters, keep letters, digits, and underscores
    base = os.path.splitext(os.path.basename(filename))[0]
    # Replace any characters not valid in C identifiers with underscore
    sanitized = re.sub(r'\W', '_', base)
    return sanitized

def fix_array_names(xxd_output, new_name):
    """
    xxd -i outputs something like:

    unsigned char Cowbell_raw[] = {
      0x00, 0x01, ...
    };
    unsigned int Cowbell_raw_len = 123;

    We want to replace all occurrences of <oldname>_raw with <new_name>
    and <oldname>_raw_len with <new_name>_len.
    """
    # Find the original array name (likely the first line)
    match = re.search(r"unsigned char (\w+)\[\] = {", xxd_output)
    if not match:
        return xxd_output  # Could not find pattern, return unchanged

    old_name = match.group(1)

    # Replace all occurrences of old_name and old_name_len with new_name and new_name_len
    fixed_output = re.sub(rf"\b{re.escape(old_name)}\b", new_name, xxd_output)
    return fixed_output

def main():
    parser = argparse.ArgumentParser(description="Convert .raw* files to a combined C++ header")
    parser.add_argument("--input", "-i", type=str, default=".",
                        help="Input directory containing .raw* files (default: current directory)")
    parser.add_argument("--output", "-o", type=str,
                        help="Output .hpp file path. If omitted, will be derived from drum machine name.")
    args = parser.parse_args()

    drum_name = input("Drum Machine Name? ").strip()
    clean_name = drum_name.replace("_", "")

    if args.output:
        output_file = args.output
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

    # Prepare simplified names for comment list
    simplified_names = []
    for filepath in files:
        basename = os.path.basename(filepath)
        # Remove .raw or .raw.* suffix
        name = re.sub(r"\.raw(\..*)?$", "", basename, flags=re.IGNORECASE)
        simplified_names.append(name)

    # Write header comment with filenames and total count
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
            out_f.write(fixed_output)
            out_f.write("\n")

    print(f"Samples written to {output_file}")

if __name__ == "__main__":
    main()
