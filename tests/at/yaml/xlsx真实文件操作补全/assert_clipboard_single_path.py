#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path


def main() -> int:
    expected = str(Path(sys.argv[1]).resolve())
    marker = Path(sys.argv[2]).resolve()
    clipboard = subprocess.run(
        ["xclip", "-selection", "clipboard", "-o"],
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()

    if clipboard != expected:
        print(f"expected clipboard path: {expected}", file=sys.stderr)
        print(f"actual clipboard path: {clipboard}", file=sys.stderr)
        return 1

    marker.write_text("ok\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
