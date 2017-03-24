#!/bin/bash
# this file is used to auto update ts file.

lupdate -pro usb-device-formatter.pro -ts translations/usb-device-formatter.ts -no-obsolete
lupdate -pro usb-device-formatter.pro -ts translations/usb-device-formatter_zh_CN.ts -no-obsolete
