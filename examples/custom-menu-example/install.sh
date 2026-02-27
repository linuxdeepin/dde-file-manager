#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# install.sh — Install custom menu examples to user directories (no root needed).

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"

# Installation target directories
MENUS_DIR="${XDG_DATA_HOME}/deepin/dde-file-manager/context-menus"
SCRIPTS_DIR="${XDG_DATA_HOME}/dde-custom-menu-example/scripts"

echo "==> 安装 DDE 文件管理器自定义菜单示例"
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

# ── 3. Install menu conf files (replace __SCRIPTS_DIR__ placeholder) ─────────
echo "安装菜单配置文件到: $MENUS_DIR"
for conf in "$SCRIPT_DIR/menus/"*.conf; do
    DEST="${MENUS_DIR}/$(basename "$conf")"
    sed "s|__SCRIPTS_DIR__|${SCRIPTS_DIR}|g" "$conf" > "$DEST"
    echo "  已安装: $(basename "$conf")"
done

echo
echo "==> 安装完成！"
echo
echo "当前安装路径："
echo "  菜单配置: ${MENUS_DIR}"
echo "  辅助脚本: ${SCRIPTS_DIR}"
echo
echo "示例菜单说明："
echo "  compress-here  — 右键文件/目录 → '压缩到...' → 选择 .tar.gz 或 .zip"
echo "  copy-path      — 右键文件/目录 → '复制路径'（需要 xclip 或 xsel）"
echo "  open-terminal  — 右键空白区域或目录 → '在此处打开终端'"
echo
echo "配置文件会被文件管理器自动加载，无需重启。"
echo "若未立即生效，可执行："
echo "  killall dde-file-manager 2>/dev/null; dde-file-manager &"
