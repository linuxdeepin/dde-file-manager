#!/usr/bin/env python3
"""Assert a DConfig boolean value, then write a PASS marker.

Usage: assert_dconfig_value.py <app_id> <resource> <key> <expected_bool> <marker>
"""
import json
import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 6:
        print("usage: assert_dconfig_value.py <app_id> <resource> <key> <expected_bool> <marker>", file=sys.stderr)
        return 2

    app_id, resource, key, expected_str, marker_path = sys.argv[1:]
    expected = expected_str.lower() == "true"

    result = subprocess.run(
        ["dde-dconfig", "--get", "-a", app_id, "-r", resource, "-k", key],
        text=True, capture_output=True,
    )
    if result.returncode != 0:
        print(f"dde-dconfig get failed: {result.stderr}", file=sys.stderr)
        return 1

    raw = result.stdout.strip()
    if raw.startswith('"') and raw.endswith('"'):
        raw = raw[1:-1]
    try:
        actual = json.loads(raw)
    except json.JSONDecodeError:
        print(f"cannot parse dconfig value: {raw!r}", file=sys.stderr)
        return 1

    if actual != expected:
        print(f"{key}: expected {expected}, got {actual}", file=sys.stderr)
        return 1

    marker = Path(marker_path)
    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(f"{key}={expected}\n", encoding="utf-8")
    print(f"OK: {key}={actual}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
