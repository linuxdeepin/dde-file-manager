#!/usr/bin/env python3
"""Assert directory contains exactly N direct children, then write PASS marker.

Usage: assert_dir_file_count.py <marker> <dir> <expected_count> [glob_pattern]
"""
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 4:
        print("usage: assert_dir_file_count.py <marker> <dir> <expected_count> [glob_pattern]", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    directory = Path(sys.argv[2])
    expected = int(sys.argv[3])
    pattern = sys.argv[4] if len(sys.argv) > 4 else "*"

    if not directory.is_dir():
        print(f"directory does not exist: {directory}", file=sys.stderr)
        return 1

    children = [child for child in directory.iterdir() if child.name.startswith("PASS_") is False]
    matched = [child for child in children if child.match(pattern)]
    actual = len(matched)

    if actual != expected:
        print(f"expected {expected} items matching '{pattern}' in {directory}, got {actual}", file=sys.stderr)
        for child in matched:
            print(f"  found: {child.name}", file=sys.stderr)
        return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text(f"file_count={actual}\n", encoding="utf-8")
    print(f"OK: {directory} has {actual} items matching '{pattern}'")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
