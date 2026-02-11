// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYEVENTCALL_H
#define PROPERTYEVENTCALL_H

#include "dfmplugin_propertydialog_global.h"

#include <QFileDevice>

namespace dfmplugin_propertydialog {
class PropertyEventCall
{
public:
    PropertyEventCall() = delete;

public:
    static void sendSetPermissionManager(quint64 winID, const QUrl &url, const QFileDevice::Permissions permissions);

    static void sendFileHide(quint64 winID, const QList<QUrl> &urls);
};
}

Q_DECLARE_METATYPE(QFileDevice::Permissions)
#endif   // PROPERTYEVENTCALL_H
