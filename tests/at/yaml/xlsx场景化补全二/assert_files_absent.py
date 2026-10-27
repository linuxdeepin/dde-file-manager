#!/usr/bin/env python3
"""Assert that specific files do NOT exist (after delete/redo operations).

Usage: assert_files_absent.py <marker> <path1> [path2 ...]
  Each path must NOT exist for the assertion to pass.
"""
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: assert_files_absent.py <marker> <path1> [path2 ...]", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    targets = [Path(p) for p in sys.argv[2:]]

    present = [str(t) for t in targets if t.exists()]
    if present:
        print(f"unexpected files still exist: {present}", file=sys.stderr)
        return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    names = ", ".join(Path(p).name for p in sys.argv[2:])
    marker_path.write_text(f"absent: {names}\n", encoding="utf-8")
    print(f"OK: all {len(targets)} files absent")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
