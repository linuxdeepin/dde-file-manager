// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualappendcompressplugin.h"

using namespace dfmplugin_utils;

void VirtualAppendCompressPlugin::initialize()
{
    eventReceiver->initEventConnect();
}

bool VirtualAppendCompressPlugin::start()
{
    return true;
}
