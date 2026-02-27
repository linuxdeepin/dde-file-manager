#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# install.sh — Install OEM menu examples to the user's OEM menu extensions
#              directory (no root required).
#
# Installation paths (user-level):
#   Desktop entries : ~/.local/share/deepin/dde-file-manager/oem-menuextensions/
#   Helper scripts  : ~/.local/share/dde-oem-menu-example/scripts/

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"

# Installation target directories
MENUS_DIR="${XDG_DATA_HOME}/deepin/dde-file-manager/oem-menuextensions"
SCRIPTS_DIR="${XDG_DATA_HOME}/dde-oem-menu-example/scripts"

echo "==> 安装 DDE 文件管理器 OEM 菜单示例"
echo

# ── 1. Create directories ────────────────────────────────────────────────────
echo "创建目录..."
mkdir -p "$MENUS_DIR"
mkdir -p "$SCRIPTS_DIR"

# ── 2. Install helper scripts ────────────────────────────────────────────────
echo "安装辅助脚本到: $SCRIPTS_DIR"
for script in "$SCRIPT_DIR/scripts/"*.sh; do
    cp -v "$script" "$SCRIPTS_DIR/"
    chmod +x "${SCRIPTS_DIR}/$(basename "$script")"
done

# ── 3. Install .desktop files (replace __SCRIPTS_DIR__ placeholder) ──────────
echo "安装菜单配置文件到: $MENUS_DIR"
for desktop in "$SCRIPT_DIR/menus/"*.desktop; do
    DEST="${MENUS_DIR}/$(basename "$desktop")"
    sed "s|__SCRIPTS_DIR__|${SCRIPTS_DIR}|g" "$desktop" > "$DEST"
    echo "  已安装: $(basename "$desktop")"
done

echo
echo "==> 安装完成！"
echo
echo "当前安装路径："
echo "  菜单配置: ${MENUS_DIR}"
echo "  辅助脚本: ${SCRIPTS_DIR}"
echo
echo "示例菜单说明："
echo "  oem-send-to-desktop — 右键文件/目录 → '发送到桌面'（创建符号链接）"
echo "  oem-file-hash       — 右键任意文件  → '计算哈希值' → 选择 MD5 或 SHA-256"
echo "  oem-set-wallpaper   — 右键图片文件  → '设为桌面壁纸'"
echo
echo "配置文件会被文件管理器自动加载，无需重启。"
echo "若未立即生效，可执行："
echo "  killall dde-file-manager 2>/dev/null; dde-file-manager &"
