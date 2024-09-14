// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROLDBUS_H
#define MOUNTCONTROLDBUS_H

#include "service_mountcontrol_global.h"

#include <QObject>
#include <QDBusContext>

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE
class MountControlDBusPrivate;
SERVICEMOUNTCONTROL_END_NAMESPACE

class MountControlDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.MountControl")

public:
    explicit MountControlDBus(const char *name, QObject *parent = nullptr);
    ~MountControlDBus();

public slots:
    Q_SCRIPTABLE QVariantMap Mount(const QString &path, const QVariantMap &opts);
    Q_SCRIPTABLE QVariantMap Unmount(const QString &path, const QVariantMap &opts);
    Q_SCRIPTABLE QStringList SupportedFileSystems();

private:
    QScopedPointer<SERVICEMOUNTCONTROL_NAMESPACE::MountControlDBusPrivate> d;
};

#endif   // MOUNTCONTROLDBUS_H
