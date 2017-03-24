#!/bin/bash
# this file is used to auto update ts file.

lupdate -pro dde-file-manager-lib.pro -ts translations/dde-file-manager.ts -no-obsolete
lupdate -pro dde-file-manager-lib.pro -ts translations/dde-file-manager_zh_CN.ts -no-obsolete