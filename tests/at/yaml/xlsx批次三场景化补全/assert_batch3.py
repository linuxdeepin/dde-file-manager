#!/usr/bin/env python3
"""Assert helper for batch-3 scenario tests.

Usage:
  assert_batch3.py <marker> <check_type> <args...>

check_type=desktop_file:
  assert_batch3.py <marker> desktop_file <file>
    Validates a .desktop file has required fields: Name, Exec, Icon, Type.
check_type=desktop_fields:
  assert_batch3.py <marker> desktop_fields <file> <field>:<value> [<field>:<value>...]
    Validates specific fields in a .desktop file match expected values.
check_type=exists:
  assert_batch3.py <marker> exists <file> [<file>...]
check_type=not_exists:
  assert_batch3.py <marker> not_exists <file> [<file>...]
"""
import os
import sys
from pathlib import Path


def parse_desktop_file(path):
    """Parse a .desktop file and return dict of key=value from [Desktop Entry] section."""
    fields = {}
    in_entry = False
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line == "[Desktop Entry]":
                in_entry = True
                continue
            if line.startswith("[") and line.endswith("]"):
                in_entry = False
                continue
            if in_entry and "=" in line:
                key, _, value = line.partition("=")
                fields[key.strip()] = value.strip()
    return fields


def main():
    if len(sys.argv) < 3:
        print("usage: assert_batch3.py <marker> <check_type> <args...>", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    check_type = sys.argv[2]
    args = sys.argv[3:]

    if check_type == "desktop_file":
        if len(args) < 1:
            print("FAIL: desktop_file needs <file>", file=sys.stderr)
            return 2
        f = Path(args[0])
        if not f.exists():
            print(f"FAIL: {args[0]} does not exist", file=sys.stderr)
            return 1
        fields = parse_desktop_file(args[0])
        required = ["Name", "Exec", "Type"]
        for req in required:
            if req not in fields or not fields[req]:
                print(f"FAIL: .desktop file missing required field: {req}", file=sys.stderr)
                return 1
        if "Icon" not in fields:
            print("WARN: .desktop file has no Icon field", file=sys.stderr)
    elif check_type == "desktop_fields":
        if len(args) < 2:
            print("FAIL: desktop_fields needs <file> <field>:<value> [...]", file=sys.stderr)
            return 2
        f = Path(args[0])
        if not f.exists():
            print(f"FAIL: {args[0]} does not exist", file=sys.stderr)
            return 1
        fields = parse_desktop_file(args[0])
        for spec in args[1:]:
            parts = spec.split(":", 1)
            if len(parts) != 2:
                print(f"FAIL: bad spec: {spec}", file=sys.stderr)
                return 1
            key, expected = parts[0].strip(), parts[1].strip()
            actual = fields.get(key, "")
            if actual != expected:
                print(f"FAIL: field {key}={actual!r} != {expected!r}", file=sys.stderr)
                return 1
    elif check_type == "exists":
        for f in args:
            if not Path(f).exists():
                print(f"FAIL: {f} does not exist", file=sys.stderr)
                return 1
    elif check_type == "not_exists":
        for f in args:
            if Path(f).exists():
                print(f"FAIL: {f} still exists", file=sys.stderr)
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
