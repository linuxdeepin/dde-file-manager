#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto update ts file.

lupdate -pro disk-mount.pro -ts translations/dde-disk-mount-plugin.ts -no-obsolete
lupdate -pro disk-mount.pro -ts translations/dde-disk-mount-plugin_zh_CN.ts -no-obsolete
