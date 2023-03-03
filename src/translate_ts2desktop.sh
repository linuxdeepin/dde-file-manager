#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# dde-computer.desktop
DESKTOP_COMPUTER_TEMP_FILE=apps/dde-desktop/data/applications/dde-computer.desktop.tmp
DESKTOP_COMPUTER_FILE=apps/dde-desktop/data/applications/dde-computer.desktop
DESKTOP_COMPUTER_TS_DIR=../translations/dde-computer-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR $DESKTOP_COMPUTER_TEMP_FILE
mv $DESKTOP_COMPUTER_TEMP_FILE $DESKTOP_COMPUTER_FILE

# dde-trash.desktop
DESKTOP_TRASH_TEMP_FILE=apps/dde-desktop/data/applications/dde-trash.desktop.tmp
DESKTOP_TRASH_FILE=apps/dde-desktop/data/applications/dde-trash.desktop
DESKTOP_TRASH_TS_DIR=../translations/dde-trash-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR $DESKTOP_TRASH_TEMP_FILE
mv $DESKTOP_TRASH_TEMP_FILE $DESKTOP_TRASH_FILE

# dde-home.desktop
DESKTOP_HOME_TEMP_FILE=apps/dde-desktop/data/applications/dde-home.desktop.tmp
DESKTOP_HOME_FILE=apps/dde-desktop/data/applications/dde-home.desktop
DESKTOP_HOME_TS_DIR=../translations/dde-home-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_HOME_FILE $DESKTOP_HOME_TS_DIR $DESKTOP_HOME_TEMP_FILE
mv $DESKTOP_HOME_TEMP_FILE $DESKTOP_HOME_FILE

# dde-file-manager.desktop
DESKTOP_DDE_FILE_MANAGER_TEMP_FILE=src/apps/dde-file-manager/dde-file-manager.desktop.tmp
DESKTOP_DDE_FILE_MANAGER_FILE=src/apps/dde-file-manager/dde-file-manager.desktop
DESKTOP_DDE_FILE_MANAGER_TS_DIR=../translations/dde-file-manager-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_DDE_FILE_MANAGER_FILE $DESKTOP_DDE_FILE_MANAGER_TS_DIR $DESKTOP_DDE_FILE_MANAGER_TEMP_FILE
mv $DESKTOP_DDE_FILE_MANAGER_TEMP_FILE $DESKTOP_DDE_FILE_MANAGER_FILE

# dde-open.desktop
# DESKTOP_DDE_OPEN_TEMP_FILE=apps/dde-file-manager/dde-open.desktop.tmp
# DESKTOP_DDE_OPEN_FILE=apps/dde-file-manager/dde-open.desktop
# DESKTOP_DDE_OPEN_TS_DIR=../translations/dde-open-desktop/

# /usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_DDE_OPEN_FILE $DESKTOP_DDE_OPEN_TS_DIR $DESKTOP_DDE_OPEN_TEMP_FILE
# mv $DESKTOP_DDE_OPEN_TEMP_FILE $DESKTOP_DDE_OPEN_FILE