#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export INTERFACES_PATH=../../plugins/desktop/core/ddplugin-dbusregister

echo "-->make adaptor and interface of DeviceManagerDBus"
qdbusxml2cpp -i $INTERFACES_PATH/devicemanagerdbus.h -c DeviceManagerAdaptor -l DeviceManagerDBus -a $INTERFACES_PATH/dbus_adaptor/devicemanagerdbus_adaptor devicemanagerdbus.xml
qdbusxml2cpp -c DeviceManagerInterface -p dbus_interface/devicemanagerdbus_interface devicemanagerdbus.xml

echo "-->make adaptor and interface of OperationsStackManagerDbus"
qdbusxml2cpp -i $INTERFACES_PATH/operationsstackmanagerdbus.h -c OperationsStackManagerAdaptor -l OperationsStackManagerDbus -a $INTERFACES_PATH/dbus_adaptor/operationsstackmanagerdbus_adaptor operationsstackmanagerdbus.xml
qdbusxml2cpp -c OperationsStackManagerInterface -p dbus_interface/operationsstackmanagerdbus_interface operationsstackmanagerdbus.xml

echo "-->make adaptor and interface of FileManager1DBus"
qdbusxml2cpp -i $INTERFACES_PATH/filemanager1dbus.h -c FileManager1Adaptor -l FileManager1DBus -a $INTERFACES_PATH/dbus_adaptor/filemanager1dbus_adaptor filemanager1dbus.xml

echo "-->make adaptor and interface of VaultManagerDBus"
qdbusxml2cpp -i $INTERFACES_PATH/vaultmanagerdbus.h -c VaultManagerAdaptor -l VaultManagerDBus -a $INTERFACES_PATH/dbus_adaptor/vaultmanagerdbus_adaptor vaultmanagerdbus.xml
qdbusxml2cpp -c VaultManagerInterface -p dbus_interface/vaultmanagerdbus_interface vaultmanagerdbus.xml

