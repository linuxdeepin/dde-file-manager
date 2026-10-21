#!/usr/bin/env python3
"""Strict search helper: focus the file-manager search box, clear it, type
an ASCII keyword, press Return, then wait for results to render.

Provenance: live pyatspi capture 2026-09-16 on dde-file-manager opened at
/tmp/dde-at-fixtures/batch26/searchtest. The search input is the text element
named DLineEditChildLineEdit inside the SearchEdit form. The search edit is
initially collapsed (bbox 0,0,0,0); Ctrl+F expands it (bbox becomes non-zero).
After expansion, the EditableText.insertText API reliably sets the content
(xdotool type and pyatspi KEY_STRING do NOT work on this Qt QLineEdit, but
insertText does). Return is then sent via xdotool to submit the search.

Usage: search_helper.py <keyword>
Exits non-zero if the search box cannot be focused or the keyword does not
end up in the box.
"""
import subprocess
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


def _find_named(app, name, role=None):
    for r, n, o in _walk(app):
        if n == name and (role is None or r == role):
            return o
    return None


keyword = sys.argv[1] if len(sys.argv) > 1 else ""
if not keyword:
    sys.exit("usage: search_helper.py <keyword>")

app = _get_app()
if app is None:
    sys.exit("dde-file-manager not found in AT-SPI tree")

# Ensure the search edit is expanded: Ctrl+F
subprocess.run(["xdotool", "key", "ctrl+f"], check=False)
time.sleep(1.5)

search_text = _find_named(app, "DLineEditChildLineEdit", "text")
if search_text is None:
    sys.exit("DLineEditChildLineEdit (search text) not found")

# Check if the search edit has a non-zero bbox now
comp = search_text.queryComponent()
ext = comp.getExtents(pyatspi.DESKTOP_COORDS)
if ext.width == 0:
    # Try Ctrl+F again
    subprocess.run(["xdotool", "key", "ctrl+f"], check=False)
    time.sleep(1.5)
    search_text = _find_named(app, "DLineEditChildLineEdit", "text")
    if search_text is None:
        sys.exit("DLineEditChildLineEdit disappeared")
    comp = search_text.queryComponent()
    ext = comp.getExtents(pyatspi.DESKTOP_COORDS)
    if ext.width == 0:
        sys.exit("search edit text element has zero bbox even after Ctrl+F")

# Click to focus
cx = ext.x + ext.width // 2
cy = ext.y + ext.height // 2
subprocess.run(["xdotool", "mousemove", str(cx), str(cy), "click", "1"], check=False)
time.sleep(0.5)

# Clear existing content via EditableText
try:
    ti = search_text.queryText()
    old_len = ti.characterCount
    if old_len > 0:
        ei = search_text.queryEditableText()
        ei.deleteText(0, old_len)
        time.sleep(0.2)
except Exception as e:
    sys.exit(f"clear search failed: {e}")

# Insert keyword via EditableText
try:
    ei = search_text.queryEditableText()
    result = ei.insertText(0, keyword, len(keyword))
    if not result:
        sys.exit(f"insertText returned false for keyword {keyword!r}")
    time.sleep(0.3)
except Exception as e:
    sys.exit(f"insertText failed: {e}")

# Verify content
try:
    ti = search_text.queryText()
    actual = ti.getText(0, ti.characterCount)
except Exception:
    actual = ""
if actual != keyword:
    sys.exit(f"search box content mismatch: expected {keyword!r}, got {actual!r}")

# Press Return to submit search
subprocess.run(["xdotool", "key", "Return"], check=False)
time.sleep(3.0)
print(f"OK: searched {keyword!r}")
