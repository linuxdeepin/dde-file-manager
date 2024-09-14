#! /bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

qdbusxml2cpp -i ../accesscontroldbus.h -c AccessControlManagerAdaptor -l AccessControlDBus -a dbusadaptor/accesscontrolmanager_adaptor accesscontroldbus.xml
# qdbusxml2cpp -c AccessControlInterface -p dbusinterface/accesscontrol_interface accesscontroldbus.xml

