#!/bin/bash

echo "-->make adaptor and interface of TagDBus"
qdbusxml2cpp -i ../tagdbus.h -c TagDBusAdaptor -l TagDBus -a tagdbus_adaptor org.deepin.filemanager.tag.xml
mv tagdbus_adaptor.* dbus_adaptor/

qdbusxml2cpp -c TagDBusInterface -p tagdbus_interface org.deepin.filemanager.tag.xml
mv tagdbus_interface.* dbus_interface/

