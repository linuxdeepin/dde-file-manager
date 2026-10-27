#!/usr/bin/env python3

import json
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 5:
        print("usage: assert_config_value.py <json-file> <section> <key> <marker>", file=sys.stderr)
        return 2

    cfg = Path(sys.argv[1]).expanduser()
    section = sys.argv[2]
    key = sys.argv[3]
    marker = Path(sys.argv[4]).resolve()
    data = json.loads(cfg.read_text(encoding="utf-8")) if cfg.exists() else {}
    value = data.get(section, {}).get(key)
    if value is not True:
        print(f"{section}.{key} is not enabled: {value!r}", file=sys.stderr)
        return 1

    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(f"{section}.{key}=true\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
