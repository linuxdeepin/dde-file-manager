#!/bin/bash

DESKTOP_COMPUTER_TEMP_FILE=data/applications/dde-computer.desktop.tmp
DESKTOP_COMPUTER_FILE=data/applications/dde-computer.desktop
DESKTOP_COMPUTER_TS_DIR=translations/dde-computer-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR $DESKTOP_COMPUTER_TEMP_FILE
mv $DESKTOP_COMPUTER_TEMP_FILE $DESKTOP_COMPUTER_FILE


DESKTOP_TRASH_TEMP_FILE=data/applications/dde-trash.desktop.tmp
DESKTOP_TRASH_FILE=data/applications/dde-trash.desktop
DESKTOP_TRASH_TS_DIR=translations/dde-trash-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR $DESKTOP_TRASH_TEMP_FILE
mv $DESKTOP_TRASH_TEMP_FILE $DESKTOP_TRASH_FILE
