#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

echo "=====disk====="
# qdbuscpp2xml -M -S ../disk/diskmanager.h -o disk.xml
qdbusxml2cpp -i ../disk/diskmanager.h -c DiskAdaptor -l DiskManager -a ./dbusadaptor/disk_adaptor disk.xml
qdbusxml2cpp -c DiskInterface -p ./dbusinterface/disk_interface disk.xml

