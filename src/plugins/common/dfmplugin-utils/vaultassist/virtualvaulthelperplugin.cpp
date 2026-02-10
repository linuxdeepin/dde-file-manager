// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualvaulthelperplugin.h"

DPUTILS_USE_NAMESPACE

void VirtualVaultHelperPlugin::initialize()
{
    eventReceiver->initEventConnect();
}

bool VirtualVaultHelperPlugin::start()
{
    return true;
}
