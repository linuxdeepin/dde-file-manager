#!/bin/sh

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#dde-file-manager
data="ZGRlLWZpbGUtbWFuYWdlcg=="

for i in "$@"
do
        arg_base64=`echo -n "$i"|base64 -w 0`
        data=$data" "$arg_base64
done

target=$XDG_RUNTIME_DIR/dde-file-manager

if [ ! -S $target ];then
        target=/tmp/dde-file-manager
        if [ ! -S $target ];then
                dde-file-manager "$@" &
                exit "$?"
        fi
fi

echo $data | socat - $target

if [ $? != 0 ]; then
        dde-file-manager "$@" &
fi
