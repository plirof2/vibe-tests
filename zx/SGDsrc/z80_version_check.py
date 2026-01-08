import os

def check_z80_version(file_path):
    """
    Determines the version of a Z80 snapshot file based on its header.
    Returns: A string describing the version or an error.
    """
    try:
        with open(file_path, 'rb') as f:
            # Read the first 32 bytes (enough for PC and additional header length)
            header = f.read(32)
            
            # Safety check: File must be at least 32 bytes to be a valid Z80 snapshot
            if len(header) < 32:
                return "Invalid/Too small"

            # Check PC in first header (bytes 6-7, little-endian)
            pc = int.from_bytes(header[6:8], 'little')
            
            if pc != 0:
                return "Version 1"
            else:
                # Read additional header length (bytes 30-31, little-endian)
                add_header_len = int.from_bytes(header[30:32], 'little')
                
                if add_header_len == 23:
                    return "Version 2"
                elif add_header_len == 54 or add_header_len == 55:
                    return "Version 3"
                else:
                    return f"Unknown (Header Len: {add_header_len})"
                    
    except Exception as e:
        return f"Error: {str(e)}"


def main():
    target_folder = "ROMSMINE"
    
    # Check if the root folder exists
    if not os.path.exists(target_folder):
        print(f"❌ Error: The folder '{target_folder}' was not found.")
        print("Please create it or ensure the script is run from the correct directory.")
        return

    # Recursively find all .z80 files (case-insensitive)
    z80_files = []
    for root, dirs, files in os.walk(target_folder):
        for file in files:
            if file.lower().endswith('.z80'):
                full_path = os.path.join(root, file)
                # Store relative path for nicer display
                rel_path = os.path.relpath(full_path, os.getcwd())
                z80_files.append((rel_path, full_path))

    if not z80_files:
        print(f"📂 No .z80 files found in '{target_folder}' or any of its subfolders.")
        return

    print(f"🔍 Found {len(z80_files)} .z80 file(s) in '{target_folder}' and subfolders.\n")
    
    # Sort by relative path for consistent output
    z80_files.sort(key=lambda x: x[0])

    print(f"{'Filename':<60} | {'Version'}")
    print("-" * 80)

    for display_name, full_path in z80_files:
        version = check_z80_version(full_path)
        print(f"{display_name:<60} | {version}")

if __name__ == "__main__":
    main()