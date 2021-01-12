#!/bin/bash
# this file is used to auto update ts file.

lupdate -pro dde-file-manager-plugins.pro -ts translations/dde-file-manager-plugins.ts -no-obsolete
lupdate -pro dde-file-manager-plugins.pro -ts translations/dde-file-manager-plugins_zh_CN.ts -no-obsolete
