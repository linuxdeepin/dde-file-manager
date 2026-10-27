#!/usr/bin/env python3
import os
import subprocess
import sys
from pathlib import Path


PACKAGE = "dde-file-manager"


def run(args):
    return subprocess.check_output(args, text=True, stderr=subprocess.DEVNULL)


def is_elf(path):
    try:
        with open(path, "rb") as f:
            return f.read(4) == b"\x7fELF"
    except OSError:
        return False


def check_elf(path):
    header = run(["readelf", "-h", path])
    segments = run(["readelf", "-lW", path])
    dynamic = run(["readelf", "-dW", path])
    symbols = run(["readelf", "-sW", path])

    checks = {
        "PIE": "Type:                              DYN" in header or "类型:                              DYN" in header,
        "RELRO": "GNU_RELRO" in segments,
        "BIND_NOW": "BIND_NOW" in dynamic or "NOW" in dynamic,
        "NX": any("GNU_STACK" in line and " RWE " not in line and " RW " in line for line in segments.splitlines()),
        "STACK_CANARY": "__stack_chk_fail" in symbols,
    }
    return [name for name, ok in checks.items() if not ok]


def main():
    case_id = os.environ.get("DDE_AT_CASE_ID", "1880111")
    marker = Path(f"/tmp/dde-at-fixtures/case_{case_id}/PASS_{case_id}")
    package_files = run(["dpkg-query", "-L", PACKAGE]).splitlines()
    elf_files = [path for path in package_files if os.path.isfile(path) and is_elf(path)]
    if not elf_files:
        raise RuntimeError(f"no ELF files found in package {PACKAGE}")

    failures = []
    for path in elf_files:
        missing = check_elf(path)
        if missing:
            failures.append(f"{path}: {', '.join(missing)}")

    if failures:
        raise RuntimeError("security hardening check failed\n" + "\n".join(failures))

    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(f"checked {len(elf_files)} ELF files\n", encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(exc, file=sys.stderr)
        sys.exit(1)
