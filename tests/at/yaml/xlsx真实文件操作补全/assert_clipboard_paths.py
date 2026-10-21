#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path


def main() -> int:
    fixture = Path(sys.argv[1]).resolve()
    expected = {str(path.resolve()) for path in fixture.iterdir()}
    clipboard = subprocess.run(
        ["xclip", "-selection", "clipboard", "-o"],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    actual = {line for line in clipboard.splitlines() if line}

    if actual != expected:
        print(f"expected {len(expected)} paths, got {len(actual)}", file=sys.stderr)
        print(f"missing: {sorted(expected - actual)[:5]}", file=sys.stderr)
        print(f"extra: {sorted(actual - expected)[:5]}", file=sys.stderr)
        return 1
    (fixture / "PASS_1939833").write_text("ok\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
