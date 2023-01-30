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
#ifndef MOUNTCONTROLDBUS_P_H
#define MOUNTCONTROLDBUS_P_H

#include "daemonplugin_mountcontrol_global.h"

#include "mounthelpers/abstractmounthelper.h"

#include <QString>
#include <QVariantMap>

class MountControlAdapter;
class MountControlDBus;

DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE

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

DAEMONPMOUNTCONTROL_END_NAMESPACE

#endif   // MOUNTCONTROLDBUS_P_H
