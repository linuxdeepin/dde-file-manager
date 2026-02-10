// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountcontroldbus.h"

static MountControlDBus *mountControlDBus = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    mountControlDBus = new MountControlDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    mountControlDBus->deleteLater();
    mountControlDBus = nullptr;
    return 0;
}
