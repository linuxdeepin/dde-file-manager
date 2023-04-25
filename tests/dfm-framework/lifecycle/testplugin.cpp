// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testplugin.h"

void TestPlugin::initialize()
{
    initialized = true;
}

bool TestPlugin::start()
{
    return startResult;
}

void TestPlugin::stop()
{
    stopped = true;
}
