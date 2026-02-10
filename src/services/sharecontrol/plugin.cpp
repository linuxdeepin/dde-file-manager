// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecontroldbus.h"

static ShareControlDBus *shareControlDBus = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    shareControlDBus = new ShareControlDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    shareControlDBus->deleteLater();
    shareControlDBus = nullptr;
    return 0;
}
