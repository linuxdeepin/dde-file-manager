#!/bin/bash

# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

sys_name=$(grep "^SystemName=" /etc/os-version | cut -d'=' -f2)
sys_ver=$(grep "^MajorVersion=" /etc/os-version | cut -d'=' -f2)

# this is for adapt the new ApplicationManager.
# open computer/trash root with 'gio open',
# the cgroups of the opened app is not belongs to dde-file-manager,
# which cause the opened apps in dock panel is not dde-file-manager,
# and the new opened dde-file-manager window also wrapped into trash/computer entry.
# the 'xdg-open' in V23 accept the computer/trash scheme,
# So, after opening a directory with xdg-open,
# the application's cgroup can be categorized into the file manager as expected.

if [ "$sys_name" = "Deepin" ] && [ "$sys_ver" = "23" ]; then
	echo "open $1 with xdg-open."
	xdg-open $1
else
	echo "open $1 with gio open."
	gio open $1
fi

