/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFMBLOCKDEVICE_P_H
#define DFMBLOCKDEVICE_P_H

#include "dfmblockdevice.h"

QT_BEGIN_NAMESPACE
class QDBusObjectPath;
QT_END_NAMESPACE

class OrgFreedesktopUDisks2BlockInterface;

DFM_BEGIN_NAMESPACE

class DFMBlockDevicePrivate
{
public:
    explicit DFMBlockDevicePrivate(DFMBlockDevice *qq);

    OrgFreedesktopUDisks2BlockInterface *dbus;
    bool watchChanges = false;
    DFMBlockDevice *q_ptr;

    void _q_onInterfacesAdded(const QDBusObjectPath &object_path, const QMap<QString, QVariantMap> &interfaces_and_properties);
    void _q_onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void _q_onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties);

    Q_DECLARE_PUBLIC(DFMBlockDevice)
};

DFM_END_NAMESPACE

#endif // DFMBLOCKDEVICE_P_H
