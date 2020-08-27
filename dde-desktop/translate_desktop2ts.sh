#!/bin/bash

DESKTOP_COMPUTER_SOURCE_FILE=data/applications/dde-computer.desktop
DESKTOP_COMPUTER_TS_DIR=translations/dde-computer-desktop/

/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_COMPUTER_SOURCE_FILE $DESKTOP_COMPUTER_TS_DIR


DESKTOP_TRASH_SOURCE_FILE=data/applications/dde-trash.desktop
DESKTOP_TRASH_TS_DIR=translations/dde-trash-desktop/

/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_TRASH_SOURCE_FILE $DESKTOP_TRASH_TS_DIR

DESKTOP_HOME_SOURCE_FILE=data/applications/dde-home.desktop
DESKTOP_HOME_TS_DIR=translations/dde-home-desktop/

/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_HOME_SOURCE_FILE $DESKTOP_HOME_TS_DIR
