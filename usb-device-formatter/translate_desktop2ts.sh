#!/bin/bash

DESKTOP_SOURCE_FILE=usb-device-formatter.desktop
DESKTOP_TS_DIR=translations/usb-device-formatter-desktop/

/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR
