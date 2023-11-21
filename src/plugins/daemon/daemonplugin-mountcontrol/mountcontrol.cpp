// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountcontrol.h"
#include "mountcontroldbus.h"

namespace daemonplugin_mountcontrol {
DFM_LOG_REISGER_CATEGORY(DAEMONPMOUNTCONTROL_NAMESPACE)

void MountControl::initialize()
{
}

bool MountControl::start()
{
    mng.reset(new MountControlDBus(this));
    return true;
}

}   // namespace daemonplugin_mountcontrol
