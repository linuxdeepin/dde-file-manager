// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tpmcontroldbus.h"

static TPMControlDBus *tpmControlDBus = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    tpmControlDBus = new TPMControlDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    if (tpmControlDBus) {
        tpmControlDBus->deleteLater();
        tpmControlDBus = nullptr;
    }
    return 0;
}
