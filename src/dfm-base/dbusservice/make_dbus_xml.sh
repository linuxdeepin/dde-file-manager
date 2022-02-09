#!/bin/bash

export INTERFACES_PATH=../../plugins/desktop/ddplugin-devices

echo "-->make XML of DeviceManagerDBus"
qdbuscpp2xml -M -S $INTERFACES_PATH/devicemanagerdbus.h -o ./devicemanagerdbus.xml
