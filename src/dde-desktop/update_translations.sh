#!/bin/bash
# this file is used to auto update ts file.

lupdate -pro dde-desktop.pro -ts translations/dde-desktop.ts -no-obsolete
lupdate -pro dde-desktop.pro -ts translations/dde-desktop_zh_CN.ts -no-obsolete
