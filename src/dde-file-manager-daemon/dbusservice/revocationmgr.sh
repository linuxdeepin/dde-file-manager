#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

echo "=====revocationmanager====="
#qdbuscpp2xml -M -S ../revocation/revocationmanager.h -o revocation.xml
qdbusxml2cpp -i ../revocation/revocationmanager.h -c RevocationMgrAdaptor -l RevocationManager -a ../dbusservice/dbusadaptor/revocationmgr_adaptor revocation.xml
qdbusxml2cpp -c RevocationMgrInterface -p ../dbusservice/dbusinterface/revocationmgr_interface revocation.xml

