#!/usr/bin/env python3
import json
import subprocess
import sys
from pathlib import Path

APP_ID = "org.deepin.dde.file-manager"


def get_value(resource, key):
    result = subprocess.run(
        ["dde-dconfig", "--get", "-a", APP_ID, "-r", resource, "-k", key],
        text=True,
        capture_output=True,
        check=True,
    )
    raw = result.stdout.strip()
    if raw.startswith('"') and raw.endswith('"'):
        raw = raw[1:-1]
    return json.loads(raw)


def set_value(resource, key, value):
    subprocess.run(
        [
            "dde-dconfig",
            "--set",
            "-a",
            APP_ID,
            "-r",
            resource,
            "-k",
            key,
            "-v",
            json.dumps(value, ensure_ascii=False, separators=(",", ":")),
        ],
        text=True,
        capture_output=True,
        check=True,
    )


def main():
    if len(sys.argv) != 5:
        raise SystemExit("usage: set_dconfig_value.py <resource> <key> <json-value> <backup>")

    resource, key, value_text, backup_path = sys.argv[1:]
    backup = Path(backup_path)
    backup.parent.mkdir(parents=True, exist_ok=True)

    previous = get_value(resource, key)
    backup.write_text(
        json.dumps({"resource": resource, "key": key, "value": previous}, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    set_value(resource, key, json.loads(value_text))


if __name__ == "__main__":
    main()
