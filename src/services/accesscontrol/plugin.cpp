// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accesscontroldbus.h"

static AccessControlDBus *accessControlServer = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    accessControlServer = new AccessControlDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    accessControlServer->deleteLater();
    accessControlServer = nullptr;
    return 0;
}
