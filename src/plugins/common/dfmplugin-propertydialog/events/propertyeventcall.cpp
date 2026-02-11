// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertyeventcall.h"
#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

void PropertyEventCall::sendSetPermissionManager(quint64 winID, const QUrl &url, const QFileDevice::Permissions permissions)
{
    dpfSignalDispatcher->publish(GlobalEventType::kSetPermission, winID, url, permissions);
}

void PropertyEventCall::sendFileHide(quint64 winID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kHideFiles, winID, urls);
}
