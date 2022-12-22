#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# dde-computer.desktop
DESKTOP_COMPUTER_FILE=apps/dde-desktop/data/applications/dde-computer.desktop
DESKTOP_COMPUTER_TS_DIR=../translations/dde-computer-desktop/
/usr/bin/deepin-desktop-ts-convert init $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR
/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR

# dde-home.desktop
DESKTOP_HOME_FILE=apps/dde-desktop/data/applications/dde-home.desktop
DESKTOP_HOME_TS_DIR=../translations/dde-home-desktop/
/usr/bin/deepin-desktop-ts-convert init $DESKTOP_HOME_FILE $DESKTOP_HOME_TS_DIR
/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_HOME_FILE $DESKTOP_HOME_TS_DIR

# dde-trash.desktop
DESKTOP_TRASH_FILE=apps/dde-desktop/data/applications/dde-trash.desktop
DESKTOP_TRASH_TS_DIR=../translations/dde-trash-desktop/
/usr/bin/deepin-desktop-ts-convert init $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR
/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR

# dde-file-manager.desktop
DESKTOP_DDE_FILE_MANAGER_FILE=src/apps/dde-file-manager/dde-file-manager.desktop
DESKTOP_DDE_FILE_MANAGER_TS_DIR=../translations/dde-file-manager-desktop/
/usr/bin/deepin-desktop-ts-convert init $DESKTOP_DDE_FILE_MANAGER_FILE $DESKTOP_DDE_FILE_MANAGER_TS_DIR
/usr/bin/deepin-desktop-ts-convert desktop2ts  $DESKTOP_DDE_FILE_MANAGER_FILE $DESKTOP_DDE_FILE_MANAGER_TS_DIR

# dde-open.desktop
# DESKTOP_DDE_OPEN_FILE=apps/dde-file-manager/dde-open.desktop
# DESKTOP_DDE_OPEN_TS_DIR=../translations/dde-open-desktop/
# /usr/bin/deepin-desktop-ts-convert init $DESKTOP_DDE_OPEN_FILE $DESKTOP_DDE_OPEN_TS_DIR
# /usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_DDE_OPEN_FILE $DESKTOP_DDE_OPEN_TS_DIR