#!/usr/bin/env python3
"""Assert trash contains exactly N items, then write PASS marker.

Usage: assert_trash_count.py <marker> <expected_count>
"""
import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: assert_trash_count.py <marker> <expected_count>", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    expected = int(sys.argv[2])

    result = subprocess.run(
        ["gio", "trash", "--list"],
        capture_output=True, text=True,
    )
    lines = [line for line in result.stdout.strip().splitlines() if line.strip()]
    actual = len(lines)

    if actual != expected:
        print(f"expected {expected} trash items, got {actual}", file=sys.stderr)
        for line in lines:
            print(f"  trash: {line}", file=sys.stderr)
        return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text(f"trash_count={actual}\n", encoding="utf-8")
    print(f"OK: trash has {actual} items")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
