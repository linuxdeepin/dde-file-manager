#!/bin/bash
# this file is used to auto update ts file.

lupdate -pro disk-mount.pro -ts translations/dde-disk-mount-plugin.ts -no-obsolete
lupdate -pro disk-mount.pro -ts translations/dde-disk-mount-plugin_zh_CN.ts -no-obsolete
