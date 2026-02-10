// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTMOUNTHELPER_H
#define ABSTRACTMOUNTHELPER_H

#include "service_mountcontrol_global.h"
#include "polkit/policykithelper.h"

#include <QVariantMap>

class QDBusContext;

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE
inline constexpr char kPolicyKitActionIdDefault[] { "org.deepin.Filemanager.MountController" };
class AbstractMountHelper
{
public:
    explicit AbstractMountHelper(QDBusContext *context)
        : context(context) { }
    virtual ~AbstractMountHelper() { }
    virtual QVariantMap mount(const QString &path, const QVariantMap &opts) = 0;
    virtual QVariantMap unmount(const QString &path, const QVariantMap &opts) = 0;
    virtual bool checkAuthentication(const QString &appName)
    {
        return ServiceCommon::PolicyKitHelper::instance()->checkAuthorization(kPolicyKitActionIdDefault, appName);
    }


protected:
    QDBusContext *context { nullptr };
};
SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // ABSTRACTMOUNTHELPER_H
