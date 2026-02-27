#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# uninstall.sh — Remove custom menu examples from user directories.

set -e

XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"

MENUS_DIR="${XDG_DATA_HOME}/deepin/dde-file-manager/context-menus"
SCRIPTS_DIR="${XDG_DATA_HOME}/dde-custom-menu-example/scripts"

echo "==> 卸载 DDE 文件管理器自定义菜单示例"
echo

# ── 1. Remove menu conf files ────────────────────────────────────────────────
echo "移除菜单配置文件..."
for conf in compress-here.conf copy-path.conf open-terminal-here.conf; do
    rm -fv "${MENUS_DIR}/${conf}"
done

# ── 2. Remove helper scripts ─────────────────────────────────────────────────
echo "移除辅助脚本..."
for script in compress-here.sh copy-path.sh open-terminal-here.sh; do
    rm -fv "${SCRIPTS_DIR}/${script}"
done

# Remove the scripts directory if empty
if [ -d "$SCRIPTS_DIR" ] && [ -z "$(ls -A "$SCRIPTS_DIR")" ]; then
    rmdir -v "$SCRIPTS_DIR"
fi
if [ -d "$(dirname "$SCRIPTS_DIR")" ] && [ -z "$(ls -A "$(dirname "$SCRIPTS_DIR")")" ]; then
    rmdir -v "$(dirname "$SCRIPTS_DIR")"
fi

echo
echo "==> 卸载完成！"
echo
echo "菜单更改会自动生效（文件管理器热重载）。"
