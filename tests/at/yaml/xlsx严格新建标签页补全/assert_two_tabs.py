#!/usr/bin/env python3
"""Assert helper: verify 2 page tabs named level3 exist after the new-tab action.

YouQu's element_numbers assert matches by name only (ignores role), so
$//level3/ would also count the page-tab-list and breadcrumb button. This
script uses pyatspi to filter by role=page tab AND name=level3, asserting
exactly 2 such tabs exist -- proving a new tab was created at the deep dir.
"""
import sys

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


app = _get_app()
if app is None:
    sys.exit("dde-file-manager not found in AT-SPI tree")

page_tabs = []
for role, name, obj in _walk(app):
    if role == "page tab" and name == "level3":
        page_tabs.append(obj)

if len(page_tabs) != 2:
    sys.exit(f"expected 2 page tabs named level3, got {len(page_tabs)}")

print("OK: 2 page tabs named level3")
