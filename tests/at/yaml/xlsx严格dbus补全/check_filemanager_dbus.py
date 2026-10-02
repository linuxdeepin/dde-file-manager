#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path


MARKER = Path("/tmp/dde-at-fixtures/case_1828783/PASS_1828783")

SYSTEM_NAMES = [
    "org.deepin.Filemanager.MountControl",
    "org.deepin.Filemanager.UsbRepair",
]

SESSION_NAMES = [
    "org.deepin.Filemanager.Daemon",
    "org.deepin.Filemanager.TextIndex",
    "org.deepin.Filemanager.OcrIndex",
]


def run(args):
    return subprocess.check_output(args, text=True, stderr=subprocess.STDOUT)


def list_names(bus):
    output = run([
        "gdbus", "call", f"--{bus}",
        "--dest", "org.freedesktop.DBus",
        "--object-path", "/org/freedesktop/DBus",
        "--method", "org.freedesktop.DBus.ListNames",
    ])
    return output


def introspect(bus, name):
    run([
        "gdbus", "introspect", f"--{bus}",
        "--dest", name,
        "--object-path", "/",
    ])


def main():
    system = list_names("system")
    session = list_names("session")

    missing = [name for name in SYSTEM_NAMES if name not in system]
    missing += [name for name in SESSION_NAMES if name not in session]
    if missing:
        raise RuntimeError(f"missing Filemanager DBus names: {missing}")

    for name in SYSTEM_NAMES:
        introspect("system", name)
    for name in SESSION_NAMES:
        introspect("session", name)

    MARKER.parent.mkdir(parents=True, exist_ok=True)
    MARKER.write_text("\n".join(SYSTEM_NAMES + SESSION_NAMES) + "\n", encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(exc, file=sys.stderr)
        sys.exit(1)
