#!/usr/bin/env python3
"""Assert that lsblk reports at least one disk, then write a PASS marker.

Usage: assert_lsblk_disks.py <marker> [min_count]
"""
import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: assert_lsblk_disks.py <marker> [min_count]", file=sys.stderr)
        return 2

    marker_path = Path(sys.argv[1])
    min_count = int(sys.argv[2]) if len(sys.argv) > 2 else 1

    result = subprocess.run(
        ["lsblk", "-dn", "-o", "NAME"],
        text=True, capture_output=True,
    )
    if result.returncode != 0:
        print(f"lsblk failed: {result.stderr}", file=sys.stderr)
        return 1

    disks = [line.strip() for line in result.stdout.strip().splitlines() if line.strip()]
    if len(disks) < min_count:
        print(f"expected >= {min_count} disks, got {len(disks)}: {disks}", file=sys.stderr)
        return 1

    marker_path.parent.mkdir(parents=True, exist_ok=True)
    marker_path.write_text(f"disks={len(disks)}\n", encoding="utf-8")
    print(f"OK: found {len(disks)} disks: {disks}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
