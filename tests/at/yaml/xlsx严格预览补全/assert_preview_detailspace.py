#!/usr/bin/env python3
import shutil
import subprocess
import sys
from pathlib import Path


def main():
    if len(sys.argv) != 3:
        raise RuntimeError("usage: assert_preview_detailspace.py <exact:N|min:N> <marker>")

    mode, raw_count = sys.argv[1].split(":", 1)
    expected_count = int(raw_count)
    marker = Path(sys.argv[2])
    dump_dir = Path("/tmp/dde-at-fixtures/case_1807263/dump")
    if dump_dir.exists():
        shutil.rmtree(dump_dir)

    subprocess.check_call([
        "youqu", "at", "dump", "dtk",
        "--app", "dde-file-manager",
        "--output", str(dump_dir),
        "--no-record",
    ])
    runtime = dump_dir / "runtime.yaml"
    text = runtime.read_text(encoding="utf-8")
    count = text.count("dfmplugin_detailspace::DetailSpaceWidget")
    if mode == "exact" and count != expected_count:
        raise RuntimeError(f"expected exactly {expected_count} detailspace widgets, got {count}")
    if mode == "min" and count < expected_count:
        raise RuntimeError(f"expected at least {expected_count} detailspace widgets, got {count}")
    if mode not in {"exact", "min"}:
        raise RuntimeError(f"unknown mode: {mode}")

    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(f"detailspace_count={count}\n", encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(exc, file=sys.stderr)
        sys.exit(1)
