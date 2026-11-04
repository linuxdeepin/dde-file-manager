// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONMOUNTHELPER_H
#define COMMONMOUNTHELPER_H

#include "abstractmounthelper.h"

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE

class CommonMountHelper : public AbstractMountHelper
{
public:
    explicit CommonMountHelper(QDBusContext *context)
        : AbstractMountHelper(context) { }

    QVariantMap mount(const QString &path, const QVariantMap &opts);
    QVariantMap unmount(const QString &path, const QVariantMap &opts);
    bool checkAuthentication(const QString &appName, const uint callerID, const bool doMount, const QString &path) override;
};

SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // COMMONMOUNTHELPER_H
