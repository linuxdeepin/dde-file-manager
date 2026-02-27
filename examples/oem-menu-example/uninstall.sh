#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# uninstall.sh — Remove OEM menu examples from user directories.

set -e

XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"

MENUS_DIR="${XDG_DATA_HOME}/deepin/dde-file-manager/oem-menuextensions"
SCRIPTS_DIR="${XDG_DATA_HOME}/dde-oem-menu-example/scripts"

echo "==> 卸载 DDE 文件管理器 OEM 菜单示例"
echo

# ── 1. Remove .desktop files ─────────────────────────────────────────────────
echo "移除菜单配置文件..."
for desktop in oem-send-to-desktop.desktop oem-file-hash.desktop oem-set-wallpaper.desktop; do
    rm -fv "${MENUS_DIR}/${desktop}"
done

# ── 2. Remove helper scripts ─────────────────────────────────────────────────
echo "移除辅助脚本..."
for script in send-to-desktop.sh file-hash.sh set-wallpaper.sh; do
    rm -fv "${SCRIPTS_DIR}/${script}"
done

# Remove the scripts directory if now empty
if [ -d "$SCRIPTS_DIR" ] && [ -z "$(ls -A "$SCRIPTS_DIR")" ]; then
    rmdir -v "$SCRIPTS_DIR"
fi
PARENT_SCRIPTS_DIR="$(dirname "$SCRIPTS_DIR")"
if [ -d "$PARENT_SCRIPTS_DIR" ] && [ -z "$(ls -A "$PARENT_SCRIPTS_DIR")" ]; then
    rmdir -v "$PARENT_SCRIPTS_DIR"
fi

echo
echo "==> 卸载完成！"
echo
echo "菜单更改会自动生效（文件管理器热重载）。"
