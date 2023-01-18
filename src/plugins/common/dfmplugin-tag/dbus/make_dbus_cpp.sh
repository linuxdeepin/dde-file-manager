#!/bin/bash

echo "-->make adaptor and interface of TagDBus"
qdbusxml2cpp -i ../tagdbus.h -c TagDBusAdaptor -l TagDBus -a dbus_adaptor/tagdbus_adaptor org.deepin.filemanager.tag.xml
qdbusxml2cpp -c TagDBusInterface -p dbus_interface/tagdbus_interface org.deepin.filemanager.tag.xml

