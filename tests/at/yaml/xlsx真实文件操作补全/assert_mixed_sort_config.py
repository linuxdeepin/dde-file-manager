#!/usr/bin/env python3

import json
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: assert_mixed_sort_config.py <marker>", file=sys.stderr)
        return 2

    cfg = Path.home() / ".config/deepin/dde-file-manager.json"
    data = json.loads(cfg.read_text(encoding="utf-8")) if cfg.exists() else {}
    value = data.get("ApplicationAttribute", {}).get("FileAndDirMixedSort")
    if value is not True:
        print(f"FileAndDirMixedSort is not enabled: {value!r}", file=sys.stderr)
        return 1

    marker = Path(sys.argv[1]).resolve()
    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text("FileAndDirMixedSort=true\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
