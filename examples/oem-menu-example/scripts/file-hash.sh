#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# file-hash.sh — Compute and display the checksum of a file.
#
# Usage: file-hash.sh <md5|sha256> <file-path>
# The result is shown via a desktop notification and copied to the clipboard
# (if xclip or xsel is available).

set -euo pipefail

ALGO="${1:-}"
FILE_PATH="${2:-}"

if [ -z "$ALGO" ] || [ -z "$FILE_PATH" ]; then
    echo "Usage: file-hash.sh <md5|sha256> <file-path>" >&2
    exit 1
fi

if [ ! -f "$FILE_PATH" ]; then
    echo "Error: not a regular file: $FILE_PATH" >&2
    exit 1
fi

notify() {
    notify-send --app-name="OEM菜单" "$1" "$2" 2>/dev/null || true
}

copy_to_clipboard() {
    local text="$1"
    if command -v xclip &>/dev/null; then
        printf '%s' "$text" | xclip -selection clipboard
        return 0
    elif command -v xsel &>/dev/null; then
        printf '%s' "$text" | xsel --clipboard --input
        return 0
    fi
    return 1
}

case "$ALGO" in
    md5)
        if command -v md5sum &>/dev/null; then
            HASH="$(md5sum -- "$FILE_PATH" | awk '{print $1}')"
        else
            notify "缺少工具" "未找到 md5sum 命令"
            exit 1
        fi
        LABEL="MD5"
        ;;
    sha256)
        if command -v sha256sum &>/dev/null; then
            HASH="$(sha256sum -- "$FILE_PATH" | awk '{print $1}')"
        else
            notify "缺少工具" "未找到 sha256sum 命令"
            exit 1
        fi
        LABEL="SHA-256"
        ;;
    *)
        echo "Error: unsupported algorithm '$ALGO'. Use md5 or sha256." >&2
        exit 1
        ;;
esac

FILE_NAME="$(basename "$FILE_PATH")"

if copy_to_clipboard "$HASH"; then
    notify "${LABEL}: ${FILE_NAME}" "${HASH}
(已复制到剪贴板)"
else
    notify "${LABEL}: ${FILE_NAME}" "$HASH"
fi
