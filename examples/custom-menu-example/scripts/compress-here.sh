#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# compress-here.sh — Compress selected files/directories to .tar.gz or .zip.
#
# Usage: compress-here.sh <format> <file1> [file2 ...]
#   format: "targz" or "zip"

set -euo pipefail

FORMAT="${1:-targz}"
shift

if [ "$#" -eq 0 ]; then
    echo "Error: no files specified" >&2
    exit 1
fi

# Determine output directory (parent dir of the first selected item)
DEST_DIR="$(dirname "$1")"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
OUTPUT_BASE="${DEST_DIR}/archive_${TIMESTAMP}"

notify() {
    notify-send --app-name="自定义菜单" "$1" "$2" 2>/dev/null || true
}

# All selected items share the same parent directory (typical file manager selection).
# cd into that directory and pass only basenames so the archive contains no leading
# absolute path components (e.g. "1.txt" instead of "/home/uos/1.txt").
PARENT_DIR="$(dirname "$1")"
NAMES=()
for item in "$@"; do
    NAMES+=("$(basename "$item")")
done

case "$FORMAT" in
    targz)
        OUTPUT="${OUTPUT_BASE}.tar.gz"
        if (cd "$PARENT_DIR" && tar czf "$OUTPUT" -- "${NAMES[@]}"); then
            notify "压缩完成" "已创建: $(basename "$OUTPUT")"
        else
            notify "压缩失败" "创建 $(basename "$OUTPUT") 时出错，请检查是否安装了 tar"
            exit 1
        fi
        ;;
    zip)
        OUTPUT="${OUTPUT_BASE}.zip"
        if ! command -v zip &>/dev/null; then
            notify "缺少依赖" "未找到 zip 命令，请执行: sudo apt install zip"
            exit 1
        fi
        if (cd "$PARENT_DIR" && zip -r "$OUTPUT" -- "${NAMES[@]}"); then
            notify "压缩完成" "已创建: $(basename "$OUTPUT")"
        else
            notify "压缩失败" "创建 $(basename "$OUTPUT") 时出错"
            exit 1
        fi
        ;;
    *)
        echo "Error: unknown format '$FORMAT'" >&2
        exit 1
        ;;
esac
