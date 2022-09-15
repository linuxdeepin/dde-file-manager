#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

DESKTOP_SOURCE_FILE=dde-file-manager.desktop
DESKTOP_TS_DIR=translations/dde-file-manager-desktop/

deepin-desktop-ts-convert desktop2ts $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR
