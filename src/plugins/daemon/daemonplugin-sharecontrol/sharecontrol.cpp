// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecontrol.h"
#include "sharecontroldbus.h"

namespace daemonplugin_sharecontrol {
DFM_LOG_REISGER_CATEGORY(DAEMONPSHARECONTROL_NAMESPACE)

void ShareControl::initialize()
{
}

bool ShareControl::start()
{
    mng.reset(new ShareControlDBus(this));
    return true;
}

}   // namespace daemonplugin_sharecontrol
