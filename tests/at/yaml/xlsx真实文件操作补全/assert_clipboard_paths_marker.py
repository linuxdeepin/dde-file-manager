#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: assert_clipboard_paths_marker.py <fixture-dir> <marker>", file=sys.stderr)
        return 2

    fixture = Path(sys.argv[1]).resolve()
    marker = Path(sys.argv[2]).resolve()
    expected = {str(path.resolve()) for path in fixture.iterdir()}
    clipboard = subprocess.run(
        ["xclip", "-selection", "clipboard", "-o"],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    actual = {line for line in clipboard.splitlines() if line}

    if actual != expected:
        print(f"expected {len(expected)} paths, got {len(actual)}", file=sys.stderr)
        print(f"missing: {sorted(expected - actual)[:10]}", file=sys.stderr)
        print(f"extra: {sorted(actual - expected)[:10]}", file=sys.stderr)
        return 1

    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(f"paths={len(actual)}\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
