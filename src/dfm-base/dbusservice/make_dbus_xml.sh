#!/bin/bash

export INTERFACES_PATH=../../plugins/desktop/ddplugin-dbusregister

echo "-->make XML of DeviceManagerDBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/devicemanagerdbus.h -o ./devicemanagerdbus.xml

echo "-->make XML of OperationsStackManagerDbus"
qdbuscpp2xml -M -S $INTERFACES_PATH/operationsstackmanagerdbus.h -o ./operationsstackmanagerdbus.xml

echo "-->make XML of FileManager1DBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/filemanager1dbus.h -o ./filemanager1dbus.xml

echo "-->make XML of VaultManagerDBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/vaultmanagerdbus.h -o ./vaultmanagerdbus.xml
