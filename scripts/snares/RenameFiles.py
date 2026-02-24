import os
import argparse
import sys
import uuid

def rename_files_with_prefix(sample_name):
    current_dir = os.getcwd()
    script_name = os.path.basename(sys.argv[0])

    original_files = [f for f in os.listdir(current_dir)
                      if os.path.isfile(f) and f != script_name]
    original_files.sort()

    temp_files = []
    for f in original_files:
        temp_name = f"__tmp_{uuid.uuid4().hex}.tmp"
        os.rename(f, temp_name)
        temp_files.append(temp_name)

    for idx, temp_name in enumerate(temp_files, start=1):
        final_idx = idx - 1
        if final_idx == 0:
            final_idx = 1
        final_name = f"{sample_name}{final_idx}.wav"

        if os.path.isdir(final_name):
            count = 1
            new_final_name = final_name
            while os.path.isdir(new_final_name):
                new_final_name = f"{final_name}_{count}"
                count += 1
            final_name = new_final_name

        if os.path.isfile(final_name):
            os.remove(final_name)

        os.rename(temp_name, final_name)
        print(f"Renamed: {temp_name} -> {final_name}")

def main():
    parser = argparse.ArgumentParser(
        description="Rename all files in current dir (excluding this script) to "
                    "<SampleName><Number>.wav, numbering starting at 1 with -1 adjustment."
    )
    parser.parse_args()

    sample_name = input("Sample Name? ").strip()
    if not sample_name:
        print("Sample name cannot be empty.")
        return

    rename_files_with_prefix(sample_name)

if __name__ == "__main__":
    main()
