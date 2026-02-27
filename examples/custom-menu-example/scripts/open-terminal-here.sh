#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# open-terminal-here.sh — Open a terminal emulator in the specified directory.
#
# Usage: open-terminal-here.sh <directory-or-file-path>
# If a file path is given, opens the terminal in its parent directory.
# Supported terminals (tried in order): deepin-terminal, xfce4-terminal,
#   gnome-terminal, konsole, xterm.

set -euo pipefail

TARGET="${1:-$HOME}"

# If a file is passed, use its parent directory
if [ -f "$TARGET" ]; then
    TARGET="$(dirname "$TARGET")"
fi

# Ensure the target is a valid directory
if [ ! -d "$TARGET" ]; then
    TARGET="$HOME"
fi

notify() {
    notify-send --app-name="自定义菜单" "$1" "$2" 2>/dev/null || true
}

open_terminal() {
    local dir="$1"

    if command -v deepin-terminal &>/dev/null; then
        deepin-terminal --work-directory "$dir" &
        return 0
    fi

    if command -v xfce4-terminal &>/dev/null; then
        xfce4-terminal --working-directory="$dir" &
        return 0
    fi

    if command -v gnome-terminal &>/dev/null; then
        gnome-terminal --working-directory="$dir" &
        return 0
    fi

    if command -v konsole &>/dev/null; then
        konsole --workdir "$dir" &
        return 0
    fi

    if command -v xterm &>/dev/null; then
        xterm -e "cd \"$dir\" && exec \"\$SHELL\"" &
        return 0
    fi

    notify "未找到终端" "请安装 deepin-terminal 或 xterm 后重试"
    return 1
}

open_terminal "$TARGET"
