#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 获取 XDG 目录
XDG_TEMPLATES_DIR=$(xdg-user-dir TEMPLATES 2>/dev/null || echo "$HOME/Templates")
XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"

echo "==> 安装 DDE 文件管理器模板示例"
echo

# 创建目录
echo "创建目录..."
mkdir -p "$XDG_TEMPLATES_DIR"
mkdir -p "$XDG_DATA_HOME/templates"

# 安装普通文件模板
echo "安装普通文件模板到: $XDG_TEMPLATES_DIR"
cp -v "$SCRIPT_DIR/templates/simple/"* "$XDG_TEMPLATES_DIR/"

# 安装 Desktop Entry 模板
echo "安装 Desktop Entry 模板到: $XDG_DATA_HOME/templates"
cp -v "$SCRIPT_DIR/templates/desktop-entry/"* "$XDG_DATA_HOME/templates/"

echo
echo "==> 安装完成！"
echo
echo "请执行以下命令使模板生效："
echo "  killall dde-file-manager"
echo "  systemctl --user restart dde-shell-plugin@org.deepin.ds.desktop.service"
echo
echo "或者注销后重新登录。"
