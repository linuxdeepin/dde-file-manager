#! /bin/bash

qdbusxml2cpp -i ../accesscontroldbus.h -c AccessControlManagerAdaptor -l AccessControlDBus -a dbusadaptor/accesscontrolmanager_adaptor accesscontroldbus.xml
# qdbusxml2cpp -c AccessControlInterface -p dbusinterface/accesscontrol_interface accesscontroldbus.xml

