// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"

static TextIndexDBus *textIndexDBus = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    textIndexDBus = new TextIndexDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    textIndexDBus->deleteLater();
    textIndexDBus = nullptr;
    return 0;
} 