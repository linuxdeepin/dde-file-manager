#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 4:
        print("usage: search_and_mark.py <marker> <keyword> <expected...>", file=sys.stderr)
        return 2

    marker = Path(sys.argv[1])
    keyword = sys.argv[2]
    expected = sys.argv[3:]

    subprocess.run(
        ["python3", "tests/at/yaml/xlsx严格搜索补全/search_helper.py", keyword],
        check=True,
    )
    subprocess.run(
        [
            "python3",
            "tests/at/yaml/xlsx严格搜索补全/assert_search_results.py",
            str(len(expected)),
            *expected,
        ],
        check=True,
    )

    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text("ok\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
