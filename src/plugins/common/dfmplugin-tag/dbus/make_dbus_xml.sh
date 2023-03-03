#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

echo "-->make XML of TagDBus"
qdbuscpp2xml -M -S tagdbus.h -o ./org.deepin.filemanager.tag.xml
