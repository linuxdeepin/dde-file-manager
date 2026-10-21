#!/usr/bin/env python3
"""Strict search assertion helper.

Counts the list items in the file-view list and checks that exactly the
expected file names appear (and no others). Provenance: live pyatspi capture
2026-09-16 on dde-file-manager opened at /tmp/dde-at-fixtures/batch26/searchtest
shows the file view as an un-named AT-SPI list whose direct children are
role=list item nodes named with the file/directory display names.

Usage: assert_search_results.py <expected_count> <expected_name1> [<expected_name2> ...]
  e.g. assert_search_results.py 1 at_search_marker.txt

Exits non-zero if the file-view list is not found or the result set does not
exactly match the expected names.
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


if len(sys.argv) < 3:
    sys.exit("usage: assert_search_results.py <expected_count> <expected_name1> [...]")

expected_count = int(sys.argv[1])
expected_names = sys.argv[2:]

app = _get_app()
if app is None:
    sys.exit("dde-file-manager not found in AT-SPI tree")

# The file-view list is the one whose children include names containing file
# extensions (.txt, .sh, .md) or known directory names (subdir). We look for a
# list node with at least one child whose name contains a '.' (extension) and
# no more than a small number of children to avoid menu/filter lists.
candidates = []
for role, name, obj in _walk(app):
    if role != "list":
        continue
    n = obj.childCount
    if n == 0:
        continue
    # collect child names
    child_names = []
    for i in range(n):
        try:
            ch = obj.getChildAtIndex(i)
            child_names.append(ch.name or "")
        except Exception:
            pass
    # Filter: file-view list contains at least one child with a file-like name
    # (contains '.' or equals a known fixture directory name)
    file_like = any(
        ("." in cn) or (cn in ("subdir", "Desktop", "Documents"))
        for cn in child_names
    )
    if file_like:
        candidates.append((obj, child_names))

if not candidates:
    sys.exit("file-view list not found")

# Pick the candidate whose child set matches expected_names best
best = None
best_score = -1
for obj, child_names in candidates:
    score = sum(1 for en in expected_names if en in child_names)
    if score > best_score:
        best_score = score
        best = (obj, child_names)

if best is None:
    sys.exit("no matching file-view list candidate found")

obj, child_names = best
if len(child_names) != expected_count:
    sys.exit(
        f"expected {expected_count} results, got {len(child_names)}: {child_names}"
    )

missing = [en for en in expected_names if en not in child_names]
if missing:
    sys.exit(f"missing expected results {missing}; got {child_names}")

extra = [cn for cn in child_names if cn not in expected_names]
if extra:
    sys.exit(f"unexpected extra results {extra}; expected {expected_names}")

print(f"OK: {expected_count} results match {expected_names}")
