// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingserver.h"

static service_anything::AnythingPlugin *anythingServer = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    anythingServer = new service_anything::AnythingPlugin();
    anythingServer->initialize();
    anythingServer->start();
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    anythingServer->stop();
    anythingServer->deleteLater();
    anythingServer = nullptr;
    return 0;
}
