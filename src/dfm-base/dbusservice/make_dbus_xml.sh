#!/bin/bash

export INTERFACES_PATH=../../plugins/server/dfmsplugin-interfaces

echo "-->make XML of DeviceManagerDBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/devicemanagerdbus.h -o ./devicemanagerdbus.xml
