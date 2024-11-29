// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"

static TextIndexDBus *textIndexDBus = nullptr;

// DEBUG:
// 1. budild a debug so file and copy to isntall path
// 2. systemctl --user restart deepin-service-group@app.service
// 3. use gdb then attach to to process (/usr/bin/deepin-service-manager -g app)

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
