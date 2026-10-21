#!/usr/bin/env python3
"""Assert clipboard contains all expected substrings, then write PASS marker.

Usage: assert_clipboard_contains.py <marker> <expected_substring> [expected_substring2 ...]
"""
import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: assert_clipboard_contains.py <marker> <expected_substring> [...]", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    expected = sys.argv[2:]

    result = subprocess.run(
        ["xclip", "-selection", "clipboard", "-o"],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        result = subprocess.run(
            ["xsel", "--clipboard", "--output"],
            capture_output=True, text=True,
        )
    clipboard = result.stdout.strip()

    for exp in expected:
        if exp not in clipboard:
            print(f"clipboard does not contain '{exp}': got '{clipboard[:200]}'", file=sys.stderr)
            return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text(f"clipboard ok: {clipboard[:100]}\n", encoding="utf-8")
    print("OK: clipboard contains expected paths")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
