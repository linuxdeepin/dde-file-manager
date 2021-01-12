#!/bin/bash

DESKTOP_TEMP_FILE=dde-file-manager.desktop.tmp
DESKTOP_SOURCE_FILE=dde-file-manager.desktop

DESKTOP_MIPS_TEMP_FILE=mips/dde-file-manager.desktop.tmp
DESKTOP_MIPS_SOURCE_FILE=mips/dde-file-manager.desktop

DESKTOP_TS_DIR=translations/dde-file-manager-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR $DESKTOP_TEMP_FILE
mv $DESKTOP_TEMP_FILE $DESKTOP_SOURCE_FILE

/usr/bin/deepin-desktop-ts-convert ts2desktop $DESKTOP_MIPS_SOURCE_FILE $DESKTOP_TS_DIR $DESKTOP_MIPS_TEMP_FILE
mv $DESKTOP_MIPS_TEMP_FILE $DESKTOP_MIPS_SOURCE_FILE
