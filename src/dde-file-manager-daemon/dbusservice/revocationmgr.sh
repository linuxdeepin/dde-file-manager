#!/bin/bash


echo "=====revocationmanager====="
#qdbuscpp2xml -M -S ../revocation/revocationmanager.h -o revocation.xml
qdbusxml2cpp -i ../revocation/revocationmanager.h -c RevocationMgrAdaptor -l RevocationManager -a ../dbusservice/dbusadaptor/revocationmgr_adaptor revocation.xml
qdbusxml2cpp -c RevocationMgrInterface -p ../dbusservice/dbusinterface/revocationmgr_interface revocation.xml

