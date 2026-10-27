#!/usr/bin/env python3
"""Assert that the file-view list in dde-file-manager contains exactly the
expected item names (and no others), then write a PASS marker.

Usage: assert_file_view_items.py <marker> <expected_name1> [<expected_name2> ...]
"""
import sys
from pathlib import Path

import pyatspi


def _get_app():
    desktop = pyatspi.Registry.getDesktop(0)
    for app in desktop:
        try:
            if app.name == "dde-file-manager":
                return app
        except Exception:
            pass
    return None


def _walk(obj, depth=0, max_depth=18):
    try:
        yield (obj.getRoleName(), obj.name or "", obj)
        if depth < max_depth:
            for i in range(obj.childCount):
                ch = obj.getChildAtIndex(i)
                if ch is not None:
                    yield from _walk(ch, depth + 1, max_depth)
    except Exception:
        pass


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: assert_file_view_items.py <marker> <expected_name1> [...]", file=sys.stderr)
        return 2

    marker = Path(sys.argv[1])
    expected = sys.argv[2:]

    app = _get_app()
    if app is None:
        print("dde-file-manager not found in AT-SPI tree", file=sys.stderr)
        return 1

    # Find the file-view list: a list node whose children include file-like names
    candidates = []
    for role, name, obj in _walk(app):
        if role != "list":
            continue
        n = obj.childCount
        if n == 0:
            continue
        child_names = []
        for i in range(n):
            try:
                ch = obj.getChildAtIndex(i)
                child_names.append(ch.name or "")
            except Exception:
                pass
        file_like = any(
            ("." in cn) or (cn in ("subdir", "Desktop", "Documents", "Downloads", "WeChat"))
            for cn in child_names
        )
        if file_like:
            candidates.append((obj, child_names))

    if not candidates:
        print("file-view list not found", file=sys.stderr)
        return 1

    best = None
    best_score = -1
    for obj, child_names in candidates:
        score = sum(1 for en in expected if en in child_names)
        if score > best_score:
            best_score = score
            best = (obj, child_names)

    if best is None:
        print("no matching file-view list candidate found", file=sys.stderr)
        return 1

    obj, child_names = best
    missing = [en for en in expected if en not in child_names]
    if missing:
        print(f"missing expected items {missing}; got {child_names}", file=sys.stderr)
        return 1

    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text("ok\n", encoding="utf-8")
    print(f"OK: file-view contains {expected}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
