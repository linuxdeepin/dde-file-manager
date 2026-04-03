// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalsharedbus.h"

static OpticalShareDBus *opticalShareDBus = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    opticalShareDBus = new OpticalShareDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    opticalShareDBus->deleteLater();
    opticalShareDBus = nullptr;
    return 0;
}
