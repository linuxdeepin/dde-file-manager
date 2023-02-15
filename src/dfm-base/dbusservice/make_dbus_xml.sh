#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export INTERFACES_PATH=../../plugins/desktop/core/ddplugin-dbusregister

echo "-->make XML of DeviceManagerDBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/devicemanagerdbus.h -o ./devicemanagerdbus.xml

echo "-->make XML of OperationsStackManagerDbus"
qdbuscpp2xml -M -S $INTERFACES_PATH/operationsstackmanagerdbus.h -o ./operationsstackmanagerdbus.xml

echo "-->make XML of FileManager1DBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/filemanager1dbus.h -o ./filemanager1dbus.xml

echo "-->make XML of VaultManagerDBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/vaultmanagerdbus.h -o ./vaultmanagerdbus.xml
