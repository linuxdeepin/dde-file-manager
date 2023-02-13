// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROLDBUS_H
#define MOUNTCONTROLDBUS_H

#include "daemonplugin_mountcontrol_global.h"

#include <QObject>
#include <QDBusContext>

DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE
class MountControlDBusPrivate;
DAEMONPMOUNTCONTROL_END_NAMESPACE

class MountControlDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.MountControl")

public:
    explicit MountControlDBus(QObject *parent = nullptr);
    ~MountControlDBus();

public slots:
    QVariantMap Mount(const QString &path, const QVariantMap &opts);
    QVariantMap Unmount(const QString &path, const QVariantMap &opts);

private:
    QScopedPointer<DAEMONPMOUNTCONTROL_NAMESPACE::MountControlDBusPrivate> d;
};

#endif   // MOUNTCONTROLDBUS_H
