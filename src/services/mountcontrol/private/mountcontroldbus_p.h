// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROLDBUS_P_H
#define MOUNTCONTROLDBUS_P_H

#include "service_mountcontrol_global.h"

#include "mountcontroladaptor.h"
#include "mounthelpers/abstractmounthelper.h"

#include <QString>
#include <QVariantMap>

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

private:
    QMap<QString, AbstractMountHelper *> mountHelpers;
    MountControlDBus *q { nullptr };
    MountControlAdaptor *adapter { nullptr };
    QStringList supportedFS;
};

SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // MOUNTCONTROLDBUS_P_H
