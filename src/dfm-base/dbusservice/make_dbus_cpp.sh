#!/bin/bash

export INTERFACES_PATH=../../plugins/desktop/ddplugin-devices

echo "-->make adaptor and interface of DeviceManagerDBus"
qdbusxml2cpp -i $INTERFACES_PATH/devicemanagerdbus.h -c DeviceManagerAdaptor -l DeviceManagerDBus -a $INTERFACES_PATH/dbus_adaptor/devicemanagerdbus_adaptor devicemanagerdbus.xml
qdbusxml2cpp -c DeviceManagerInterface -p dbus_interface/devicemanagerdbus_interface devicemanagerdbus.xml

