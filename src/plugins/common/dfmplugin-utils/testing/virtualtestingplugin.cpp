// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualtestingplugin.h"
#include "events/testingeventrecevier.h"

DPUTILS_USE_NAMESPACE

void VirtualTestingPlugin::initialize()
{
    TestingEventRecevier::instance()->initializeConnections();
}

bool VirtualTestingPlugin::start()
{
    return true;
}
