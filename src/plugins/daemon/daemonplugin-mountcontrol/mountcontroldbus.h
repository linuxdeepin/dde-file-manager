/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
    QString Mount(const QString &path, const QVariantMap &opts);
    bool Unmount(const QString &path);

private:
    QScopedPointer<DAEMONPMOUNTCONTROL_NAMESPACE::MountControlDBusPrivate> d;
};

#endif   // MOUNTCONTROLDBUS_H
