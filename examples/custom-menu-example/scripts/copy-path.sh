#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# copy-path.sh — Copy the absolute path of a file or directory to the clipboard.
#
# Usage: copy-path.sh <path>
# Requires: xclip or xsel

set -euo pipefail

TARGET_PATH="${1:-}"

if [ -z "$TARGET_PATH" ]; then
    echo "Error: no path specified" >&2
    exit 1
fi

# Resolve to absolute path (handles relative/symlink cases)
ABS_PATH="$(realpath -- "$TARGET_PATH")"

notify() {
    notify-send --app-name="自定义菜单" "$1" "$2" 2>/dev/null || true
}

if command -v xclip &>/dev/null; then
    printf '%s' "$ABS_PATH" | xclip -selection clipboard
    notify "路径已复制" "$ABS_PATH"
elif command -v xsel &>/dev/null; then
    printf '%s' "$ABS_PATH" | xsel --clipboard --input
    notify "路径已复制" "$ABS_PATH"
else
    notify "缺少依赖" "未找到 xclip 或 xsel，请执行: sudo apt install xclip"
    exit 1
fi
