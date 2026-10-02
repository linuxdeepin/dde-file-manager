#!/usr/bin/env python3

import json
import shutil
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 5:
        print("usage: set_dfm_json_bool.py <section> <key> <true|false> <backup>", file=sys.stderr)
        return 2

    section, key, raw_value, backup = sys.argv[1:]
    value = raw_value.lower() == "true"
    cfg = Path.home() / ".config/deepin/dde-file-manager.json"
    backup_path = Path(backup)

    cfg.parent.mkdir(parents=True, exist_ok=True)
    if not backup_path.exists():
        if cfg.exists():
            shutil.copy2(cfg, backup_path)
        else:
            backup_path.write_text("{}\n", encoding="utf-8")

    data = json.loads(cfg.read_text(encoding="utf-8")) if cfg.exists() else {}
    data.setdefault(section, {})[key] = value
    cfg.write_text(json.dumps(data, ensure_ascii=False, indent=4) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
