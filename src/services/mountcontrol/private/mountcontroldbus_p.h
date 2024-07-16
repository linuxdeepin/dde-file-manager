// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROLDBUS_P_H
#define MOUNTCONTROLDBUS_P_H

#include "service_mountcontrol_global.h"

#include "mounthelpers/abstractmounthelper.h"

#include <QString>
#include <QVariantMap>

class MountControlAdapter;
class MountControlDBus;

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE

class MountControlDBusPrivate
{
    friend class ::MountControlDBus;

public:
    explicit MountControlDBusPrivate(MountControlDBus *qq);
    ~MountControlDBusPrivate();

private:
    // for remove the gerrit warning
    MountControlDBusPrivate(const MountControlDBusPrivate &other) { }
    MountControlDBusPrivate &operator=(const MountControlDBusPrivate &other) { return *this; }

    QVariantMap mountDlnfs(const QString &path, const QVariantMap &opts);
    QVariantMap unmountDlnfs(const QString &path, const QVariantMap &opts);

    bool checkDlnfsExist(const QString &path);

private:
    QMap<QString, AbstractMountHelper *> mountHelpers;
    MountControlDBus *q { nullptr };
    MountControlAdapter *adapter { nullptr };
};

SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // MOUNTCONTROLDBUS_P_H
