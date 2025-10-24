#!/bin/bash

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 检查是否没有传入任何文件
if [ $# -eq 0 ]; then
    exec xdg-open trash:///
    exit 0
fi

# 将传入的文件路径转换为 JSON 数组格式
SOURCES_JSON=""
for file in "$@"; do
    if [[ -n "$SOURCES_JSON" ]]; then
        SOURCES_JSON="$SOURCES_JSON,"
    fi
    # 对路径进行 JSON 转义
    ESCAPED_FILE=$(printf '%s' "$file" | jq -R .)
    SOURCES_JSON="$SOURCES_JSON$ESCAPED_FILE"
done

# 构建完整的 JSON
JSON_DATA="{\"action\":\"trash\",\"params\":{\"sources\":[$SOURCES_JSON]}}"

# 执行主脚本
exec file-manager.sh --event "$JSON_DATA"