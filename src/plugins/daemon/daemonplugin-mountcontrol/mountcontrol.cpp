// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountcontrol.h"
#include "mountcontroldbus.h"

DAEMONPMOUNTCONTROL_USE_NAMESPACE

void MountControl::initialize()
{
}

bool MountControl::start()
{
    mng.reset(new MountControlDBus(this));
    return true;
}
