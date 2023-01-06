#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

process=`ps ax -o 'cmd' |grep 'dde-file-manager$'`;
processd=`ps ax -o 'cmd' |grep 'dde-file-manager -d$'`;
  
if [[ "$process" == ""&&"$processd" == "" ]]; then
    dde-file-manager trash:///
else
    file-manager.sh trash:///
fi
