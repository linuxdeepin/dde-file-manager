#!/usr/bin/env python3
"""Assert that specific files exist at expected paths after undo/restore operations.

Usage: assert_files_restored.py <marker> <path1> [path2 ...]
  Each path must exist for the assertion to pass.
"""
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: assert_files_restored.py <marker> <path1> [path2 ...]", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    targets = [Path(p) for p in sys.argv[2:]]

    missing = [str(t) for t in targets if not t.exists()]
    if missing:
        print(f"missing expected files: {missing}", file=sys.stderr)
        return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    names = ", ".join(t.name for t in targets)
    marker_path.write_text(f"restored: {names}\n", encoding="utf-8")
    print(f"OK: all {len(targets)} files exist")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
