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
#include "propertyeventcall.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

void PropertyEventCall::sendSetPermissionManager(quint64 winID, const QUrl &url, const QFileDevice::Permissions permissions)
{
    dpfSignalDispatcher->publish(GlobalEventType::kSetPermission, winID, url, permissions);
}

void PropertyEventCall::sendFileHide(quint64 winID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kHideFiles, winID, urls);
}
