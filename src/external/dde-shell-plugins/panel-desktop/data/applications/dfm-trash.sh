#!/bin/bash

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 检查是否没有传入任何文件
if [ $# -eq 0 ]; then
    exec xdg-open trash:///
    exit 0
fi

# 使用 jq 一次性构建完整的 JSON，避免循环和多次进程调用
# --args 将所有位置参数作为字符串数组传递给 jq
JSON_DATA=$(jq -n --args '{action: "trash", params: {sources: $ARGS.positional}}' -- "$@")

# 执行主脚本
exec file-manager.sh --event "$JSON_DATA"