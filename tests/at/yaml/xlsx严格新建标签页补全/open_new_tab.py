#!/usr/bin/env python3
"""Setup helper: open the titlebar option menu and click 新建标签页.

Provenance: live pyatspi capture 2026-09-16 on dde-file-manager opened at a
deep directory. The first DTitlebarDWindowOptionButton (in DMainWindowTitlebar)
opens the full DTitlebarMainMenu containing 新建窗口/新建标签页/...; clicking
新建标签页 creates a new tab at the current deep-directory path.
"""
import sys
import time

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


def _find_all(app, pred):
    out = []
    for role, name, obj in _walk(app):
        try:
            if pred(obj, role, name):
                out.append(obj)
        except Exception:
            pass
    return out


time.sleep(3)

app = _get_app()
if app is None:
    sys.exit("dde-file-manager not found in AT-SPI tree")

option_buttons = _find_all(
    app,
    lambda o, r, n: n == "DTitlebarDWindowOptionButton" and r == "button",
)
if not option_buttons:
    sys.exit("DTitlebarDWindowOptionButton not found")

clicked = False
for btn in option_buttons:
    try:
        btn.queryAction().doAction(0)
        time.sleep(1.2)
    except Exception:
        continue
    new_tab_items = _find_all(
        app,
        lambda o, r, n: n == "新建标签页" and r == "menu item",
    )
    if new_tab_items:
        new_tab_items[0].queryAction().doAction(0)
        time.sleep(1.0)
        clicked = True
        break

if not clicked:
    sys.exit("新建标签页 menu item not found in any option-button menu")
