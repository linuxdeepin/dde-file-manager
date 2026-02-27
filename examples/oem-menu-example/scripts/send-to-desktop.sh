#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# send-to-desktop.sh — Create a symbolic link to a file or directory on the
#                       user's desktop.
#
# Usage: send-to-desktop.sh <path>

set -euo pipefail

TARGET="${1:-}"

if [ -z "$TARGET" ]; then
    echo "Error: no path specified" >&2
    exit 1
fi

# Resolve to absolute path (handles symlinks / relative paths)
ABS_TARGET="$(realpath -- "$TARGET")"

if [ ! -e "$ABS_TARGET" ]; then
    echo "Error: target does not exist: $ABS_TARGET" >&2
    exit 1
fi

# Locate desktop directory (honours XDG_DESKTOP_DIR if set)
DESKTOP_DIR="${XDG_DESKTOP_DIR:-$HOME/Desktop}"
if [ ! -d "$DESKTOP_DIR" ]; then
    # Fallback: try common Chinese desktop path
    DESKTOP_DIR="$HOME/桌面"
fi
if [ ! -d "$DESKTOP_DIR" ]; then
    notify-send --app-name="OEM菜单" "发送到桌面失败" "未找到桌面目录: $DESKTOP_DIR" 2>/dev/null || true
    exit 1
fi

LINK_NAME="$(basename "$ABS_TARGET")"
LINK_PATH="${DESKTOP_DIR}/${LINK_NAME}"

# If the link already exists, append a numeric suffix to avoid collision
COUNTER=1
while [ -e "$LINK_PATH" ]; do
    LINK_PATH="${DESKTOP_DIR}/${LINK_NAME}_${COUNTER}"
    COUNTER=$((COUNTER + 1))
done

ln -s "$ABS_TARGET" "$LINK_PATH"

notify-send --app-name="OEM菜单" "已发送到桌面" "$(basename "$LINK_PATH")" 2>/dev/null || true
