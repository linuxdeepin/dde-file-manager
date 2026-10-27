#!/usr/bin/env python3
"""Assert fixture files exist with correct attributes for semantic search tests.

Usage:
  assert_semantic_search.py <marker> <check_type> <args...>

check_type=timestamp:
  assert_semantic_search.py <marker> timestamp <file> <YYYYMMDDHHMM>
check_type=extension:
  assert_semantic_search.py <marker> extension <file> <ext>
check_type=size_gt:
  assert_semantic_search.py <marker> size_gt <file> <bytes>
check_type=exists:
  assert_semantic_search.py <marker> exists <file> [<file>...]
check_type=mixed:
  assert_semantic_search.py <marker> mixed <file>:ts:<YYYYMMDDHHMM> [<file>:ext:<ext>] [<file>:size_gt:<bytes>] ...
"""
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path


def check_timestamp(path: str, ts: str) -> bool:
    expected = datetime.strptime(ts, "%Y%m%d%H%M")
    actual = datetime.fromtimestamp(Path(path).stat().st_mtime)
    actual = actual.replace(second=0, microsecond=0)
    return actual == expected


def check_extension(path: str, ext: str) -> bool:
    return Path(path).suffix.lower() == f".{ext.lower()}"


def check_size_gt(path: str, min_bytes: str) -> bool:
    return Path(path).stat().st_size > int(min_bytes)


def main() -> int:
    if len(sys.argv) < 4:
        print("usage: assert_semantic_search.py <marker> <check_type> <args...>", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    check_type = sys.argv[2]
    args = sys.argv[3:]

    if check_type == "timestamp":
        path, ts = args
        ok = Path(path).exists() and check_timestamp(path, ts)
        if not ok:
            print(f"FAIL: {path} timestamp != {ts}", file=sys.stderr)
            return 1
    elif check_type == "extension":
        path, ext = args
        ok = Path(path).exists() and check_extension(path, ext)
        if not ok:
            print(f"FAIL: {path} extension != {ext}", file=sys.stderr)
            return 1
    elif check_type == "size_gt":
        path, min_bytes = args
        ok = Path(path).exists() and check_size_gt(path, min_bytes)
        if not ok:
            print(f"FAIL: {path} size <= {min_bytes}", file=sys.stderr)
            return 1
    elif check_type == "exists":
        for path in args:
            if not Path(path).exists():
                print(f"FAIL: {path} does not exist", file=sys.stderr)
                return 1
    elif check_type == "mixed":
        for item in args:
            parts = item.split(":")
            if len(parts) < 3:
                print(f"FAIL: bad mixed spec: {item}", file=sys.stderr)
                return 1
            file_path = parts[0]
            check = parts[1]
            value = ":".join(parts[2:])
            if not Path(file_path).exists():
                print(f"FAIL: {file_path} does not exist", file=sys.stderr)
                return 1
            if check == "ts":
                if not check_timestamp(file_path, value):
                    print(f"FAIL: {file_path} ts != {value}", file=sys.stderr)
                    return 1
            elif check == "ext":
                if not check_extension(file_path, value):
                    print(f"FAIL: {file_path} ext != {value}", file=sys.stderr)
                    return 1
            elif check == "size_gt":
                if not check_size_gt(file_path, value):
                    print(f"FAIL: {file_path} size <= {value}", file=sys.stderr)
                    return 1
            elif check == "exists":
                pass
    else:
        print(f"unknown check_type: {check_type}", file=sys.stderr)
        return 2

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text("ok\n", encoding="utf-8")
    print(f"OK: {check_type}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
