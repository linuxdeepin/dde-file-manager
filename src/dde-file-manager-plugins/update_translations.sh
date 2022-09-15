#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto update ts file.

lupdate -pro dde-file-manager-plugins.pro -ts translations/dde-file-manager-plugins.ts -no-obsolete
lupdate -pro dde-file-manager-plugins.pro -ts translations/dde-file-manager-plugins_zh_CN.ts -no-obsolete
