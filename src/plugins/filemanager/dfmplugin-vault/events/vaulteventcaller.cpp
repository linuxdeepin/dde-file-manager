/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "vaulteventcaller.h"

#include "dfm-base/dfm_event_defines.h"
#include "services/common/propertydialog/property_defines.h"

#include "services/filemanager/bookmark/bookmark_defines.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
static DPF_NAMESPACE::EventDispatcherManager *dispatcher()
{
    return &dpfInstance.eventDispatcher();
}

void VaultEventCaller::sendItemActived(quint64 windowId, const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kChangeCurrentUrl, windowId, url);
}

void VaultEventCaller::sendOpenWindow(const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kOpenNewWindow, url);
}

void VaultEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void VaultEventCaller::sendVaultProperty(const QUrl &url)
{
    dispatcher()->publish(Property::EventType::kEvokePropertyDialog, QList<QUrl>() << url);
}

void VaultEventCaller::sendBookMarkDisabled(const QString scheme)
{
    dispatcher()->publish(DSB_FM_NAMESPACE::BookMark::EventType::kBookMarkDisabled, scheme);
}
