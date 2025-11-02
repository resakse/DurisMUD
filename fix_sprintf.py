#!/usr/bin/env python3
"""
Arih: Automated sprintf to snprintf converter for DurisMUD buffer overflow fixes - 20251103

This script finds sprintf() calls and converts them to snprintf() with proper buffer size.
"""

import re
import sys
from pathlib import Path

def find_buffer_size(content, line_num, var_name):
    """Find the buffer size declaration for a variable."""
    # Look backwards from the sprintf line for buffer declaration
    lines = content.split('\n')

    # Search from current function backwards
    for i in range(line_num - 1, max(0, line_num - 100), -1):
        line = lines[i]

        # Look for: char varname[SIZE]
        match = re.search(rf'\b{re.escape(var_name)}\s*\[\s*(\w+)\s*\]', line)
        if match:
            return match.group(1)

        # Look for: char varname[1234]
        match = re.search(rf'\b{re.escape(var_name)}\s*\[\s*(\d+)\s*\]', line)
        if match:
            return match.group(1)

    return None

def fix_sprintf_in_file(filepath):
    """Convert sprintf to snprintf in a single file."""
    with open(filepath, 'r', encoding='latin-1', errors='ignore') as f:
        content = f.read()

    original_content = content
    lines = content.split('\n')
    modified_lines = []
    changes = 0

    for i, line in enumerate(lines):
        # Find sprintf calls
        match = re.search(r'\bsprintf\s*\(\s*(\w+)\s*,', line)

        if match:
            buffer_name = match.group(1)
            buffer_size = find_buffer_size(content, i, buffer_name)

            if buffer_size:
                # Replace sprintf with snprintf
                new_line = re.sub(
                    r'\bsprintf\s*\(\s*' + re.escape(buffer_name) + r'\s*,',
                    f'snprintf({buffer_name}, {buffer_size},',
                    line
                )
                modified_lines.append(new_line)
                if new_line != line:
                    changes += 1
                    print(f"Line {i+1}: {buffer_name} -> snprintf({buffer_name}, {buffer_size}, ...)")
            else:
                # Can't find buffer size, leave as is
                modified_lines.append(line)
                print(f"Line {i+1}: WARNING: Could not find buffer size for '{buffer_name}'")
        else:
            modified_lines.append(line)

    if changes > 0:
        new_content = '\n'.join(modified_lines)
        with open(filepath, 'w', encoding='latin-1') as f:
            f.write(new_content)
        print(f"\nFixed {changes} sprintf calls in {filepath}")
        return changes
    else:
        print(f"No changes made to {filepath}")
        return 0

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_sprintf.py <file1.c> [file2.c ...]")
        sys.exit(1)

    total_changes = 0
    for filepath in sys.argv[1:]:
        print(f"\n=== Processing {filepath} ===")
        changes = fix_sprintf_in_file(filepath)
        total_changes += changes

    print(f"\n=== Total: Fixed {total_changes} sprintf calls across {len(sys.argv)-1} files ===")

if __name__ == "__main__":
    main()
