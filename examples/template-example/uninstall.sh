#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

# 获取 XDG 目录
XDG_TEMPLATES_DIR=$(xdg-user-dir TEMPLATES 2>/dev/null || echo "$HOME/Templates")
XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"

echo "==> 卸载 DDE 文件管理器模板示例"
echo

# 卸载普通文件模板
echo "卸载普通文件模板..."
rm -fv "$XDG_TEMPLATES_DIR/README.md"
rm -fv "$XDG_TEMPLATES_DIR/TODO.md"
rm -fv "$XDG_TEMPLATES_DIR/shell-script.sh"

# 卸载 Desktop Entry 模板
echo "卸载 Desktop Entry 模板..."
rm -fv "$XDG_DATA_HOME/templates/python-script.py"
rm -fv "$XDG_DATA_HOME/templates/python-script.desktop"
rm -fv "$XDG_DATA_HOME/templates/cpp-source.cpp"
rm -fv "$XDG_DATA_HOME/templates/cpp-source.desktop"

echo
echo "==> 卸载完成！"
echo
echo "请执行以下命令使更改生效："
echo "  killall dde-file-manager"
echo "  systemctl --user restart dde-shell-plugin@org.deepin.ds.desktop.service"
