#!/usr/bin/env python3
"""Assert file existence/non-existence and optional attributes, then write PASS marker.

Usage: assert_file_state.py <marker> <path> <should_exist> [check_exec] [check_hidden]
  should_exist: true|false
  check_exec: true|false (optional, only when should_exist=true)
  check_hidden: true|false (optional, only when should_exist=true)
"""
import os
import stat
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 4:
        print("usage: assert_file_state.py <marker> <path> <should_exist> [check_exec] [check_hidden]", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    target = Path(sys.argv[2])
    should_exist = sys.argv[3].lower() == "true"
    check_exec = len(sys.argv) > 4 and sys.argv[4].lower() == "true"
    check_hidden = len(sys.argv) > 5 and sys.argv[5].lower() == "true"

    exists = target.exists()
    if should_exist and not exists:
        print(f"expected {target} to exist but it does not", file=sys.stderr)
        return 1
    if not should_exist and exists:
        print(f"expected {target} to NOT exist but it does", file=sys.stderr)
        return 1

    if should_exist and check_exec:
        mode = target.stat().st_mode
        if not (mode & (stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)):
            print(f"expected {target} to be executable but it is not (mode={oct(mode)})", file=sys.stderr)
            return 1

    if should_exist and check_hidden:
        if not target.name.startswith("."):
            print(f"expected {target} to be hidden (dotfile) but name is {target.name}", file=sys.stderr)
            return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text(f"{target.name}: exists={exists}\n", encoding="utf-8")
    print(f"OK: {target.name} exists={exists}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
