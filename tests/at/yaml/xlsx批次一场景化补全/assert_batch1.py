#!/usr/bin/env python3
"""Assert helper for batch-1 scenario tests.

Usage:
  assert_batch1.py <marker> <check_type> <args...>

check_type=exists:
  assert_batch1.py <marker> exists <file> [<file>...]
check_type=not_exists:
  assert_batch1.py <marker> not_exists <file> [<file>...]
check_type=permissions:
  assert_batch1.py <marker> permissions <file> <perm_str>
      perm_str e.g. "executable", "hidden", "not_executable", "not_hidden"
check_type=mixed:
  assert_batch1.py <marker> mixed <spec> [<spec>...]
      spec: <file>:exists: | <file>:not_exists: | <file>:perm:<perm_str>
check_type=trash_empty:
  assert_batch1.py <marker> trash_empty
      checks ~/.local/share/Trash/files is empty
check_type=trash_contains:
  assert_batch1.py <marker> trash_contains <basename> [<basename>...]
check_type=trash_not_contains:
  assert_batch1.py <marker> trash_not_contains <basename> [<basename>...]
"""
import os
import sys
from pathlib import Path


def get_trash_dir() -> Path:
    return Path.home() / ".local/share/Trash/files"


def check_perm(path: str, perm: str) -> bool:
    p = Path(path)
    if not p.exists():
        return False
    mode = p.stat().st_mode
    if perm == "executable":
        return bool(mode & 0o111)
    if perm == "not_executable":
        return not bool(mode & 0o111)
    if perm == "hidden":
        return p.name.startswith(".")
    if perm == "not_hidden":
        return not p.name.startswith(".")
    return False


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: assert_batch1.py <marker> <check_type> <args...>", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    check_type = sys.argv[2]
    args = sys.argv[3:]

    if check_type == "exists":
        for f in args:
            if not Path(f).exists():
                print(f"FAIL: {f} does not exist", file=sys.stderr)
                return 1
    elif check_type == "not_exists":
        for f in args:
            if Path(f).exists():
                print(f"FAIL: {f} still exists", file=sys.stderr)
                return 1
    elif check_type == "permissions":
        if len(args) < 2:
            print("FAIL: permissions needs <file> <perm>", file=sys.stderr)
            return 2
        if not check_perm(args[0], args[1]):
            print(f"FAIL: {args[0]} perm != {args[1]}", file=sys.stderr)
            return 1
    elif check_type == "mixed":
        for spec in args:
            parts = spec.split(":")
            if len(parts) < 2:
                print(f"FAIL: bad spec: {spec}", file=sys.stderr)
                return 1
            fpath = parts[0]
            check = parts[1]
            value = ":".join(parts[2:]).rstrip(":") if len(parts) > 2 else ""
            if check == "exists":
                if not Path(fpath).exists():
                    print(f"FAIL: {fpath} does not exist", file=sys.stderr)
                    return 1
            elif check == "not_exists":
                if Path(fpath).exists():
                    print(f"FAIL: {fpath} still exists", file=sys.stderr)
                    return 1
            elif check == "perm":
                if not check_perm(fpath, value):
                    print(f"FAIL: {fpath} perm != {value}", file=sys.stderr)
                    return 1
            elif check == "trash_not_contains":
                trash = get_trash_dir()
                if trash.exists() and any(p.name == fpath for p in trash.iterdir()):
                    print(f"FAIL: {fpath} still in trash", file=sys.stderr)
                    return 1
            else:
                print(f"FAIL: unknown check: {check}", file=sys.stderr)
                return 1
    elif check_type == "trash_empty":
        trash = get_trash_dir()
        if trash.exists() and any(trash.iterdir()):
            print(f"FAIL: trash not empty: {list(trash.iterdir())[:5]}", file=sys.stderr)
            return 1
    elif check_type == "trash_contains":
        trash = get_trash_dir()
        names = set()
        if trash.exists():
            names = {p.name for p in trash.iterdir()}
        for n in args:
            if n not in names:
                print(f"FAIL: {n} not in trash", file=sys.stderr)
                return 1
    elif check_type == "trash_not_contains":
        trash = get_trash_dir()
        names = set()
        if trash.exists():
            names = {p.name for p in trash.iterdir()}
        for n in args:
            if n in names:
                print(f"FAIL: {n} still in trash", file=sys.stderr)
                return 1
    else:
        print(f"unknown check_type: {check_type}", file=sys.stderr)
        return 2

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text("ok\n", encoding="utf-8")
    print(f"OK: {check_type}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
