#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto update ts file.

lupdate -pro dde-file-manager-lib.pro -ts translations/dde-file-manager.ts -no-obsolete
lupdate -pro dde-file-manager-lib.pro -ts translations/dde-file-manager_zh_CN.ts -no-obsolete