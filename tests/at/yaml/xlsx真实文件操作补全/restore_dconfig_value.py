#!/usr/bin/env python3
import json
import subprocess
import sys
from pathlib import Path

APP_ID = "org.deepin.dde.file-manager"


def main():
    for name in sys.argv[1:]:
        path = Path(name)
        if not path.exists():
            continue
        data = json.loads(path.read_text(encoding="utf-8"))
        subprocess.run(
            [
                "dde-dconfig",
                "--set",
                "-a",
                APP_ID,
                "-r",
                data["resource"],
                "-k",
                data["key"],
                "-v",
                json.dumps(data["value"], ensure_ascii=False, separators=(",", ":")),
            ],
            text=True,
            capture_output=True,
            check=False,
        )
        path.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
