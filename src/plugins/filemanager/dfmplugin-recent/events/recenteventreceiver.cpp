/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "recenteventreceiver.h"
#include "utils/recentmanager.h"

#include <dfm-framework/dpf.h>

DPRECENT_USE_NAMESPACE

RecentEventReceiver *RecentEventReceiver::instance()
{
    static RecentEventReceiver ins;
    return &ins;
}

void RecentEventReceiver::initConnect()
{
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_CheckInputAdddressStr", instance(), &RecentEventReceiver::handleAddressInputStr);
}

void RecentEventReceiver::handleAddressInputStr(QString *str)
{
    if (str->startsWith(RecentManager::scheme())) {
        str->clear();
        str->append(RecentManager::scheme() + ":/");
    }
}

RecentEventReceiver::RecentEventReceiver(QObject *parent)
    : QObject(parent)
{
}
