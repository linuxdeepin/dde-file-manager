#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto update ts file.

lupdate -pro dde-desktop.pro -ts translations/dde-desktop.ts -no-obsolete
lupdate -pro dde-desktop.pro -ts translations/dde-desktop_zh_CN.ts -no-obsolete
