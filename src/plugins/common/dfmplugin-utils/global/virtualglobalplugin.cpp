// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualglobalplugin.h"

using namespace dfmplugin_utils;

void VirtualGlobalPlugin::initialize()
{
    eventReceiver->initEventConnect();
}

bool VirtualGlobalPlugin::start()
{
    return true;
}
