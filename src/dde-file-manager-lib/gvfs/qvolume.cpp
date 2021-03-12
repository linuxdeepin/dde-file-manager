/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "qvolume.h"

QVolume::QVolume()
{

}

QString QVolume::name() const
{
    return m_name;
}

void QVolume::setName(const QString &name)
{
    m_name = name;
}

QString QVolume::uuid() const
{
    return m_uuid;
}

void QVolume::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QStringList QVolume::icons() const
{
    return m_icons;
}

void QVolume::setIcons(const QStringList &icons)
{
    m_icons = icons;
}

QStringList QVolume::symbolic_icons() const
{
    return m_symbolic_icons;
}

void QVolume::setSymbolic_icons(const QStringList &symbolic_icons)
{
    m_symbolic_icons = symbolic_icons;
}

bool QVolume::can_mount() const
{
    return m_can_mount;
}

void QVolume::setCan_mount(bool can_mount)
{
    m_can_mount = can_mount;
}

bool QVolume::can_eject() const
{
    return m_can_eject;
}

void QVolume::setCan_eject(bool can_eject)
{
    m_can_eject = can_eject;
}

bool QVolume::should_automount() const
{
    return m_should_automount;
}

void QVolume::setShould_automount(bool should_automount)
{
    m_should_automount = should_automount;
}

QString QVolume::unix_device() const
{
    return m_unix_device;
}

void QVolume::setUnix_device(const QString &unix_device)
{
    m_unix_device = unix_device;
}

QString QVolume::lable() const
{
    return m_lable;
}

void QVolume::setLable(const QString &lable)
{
    m_lable = lable;
}

QString QVolume::nfs_mount() const
{
    return m_nfs_mount;
}

void QVolume::setNfs_mount(const QString &nfs_mount)
{
    m_nfs_mount = nfs_mount;
}

bool QVolume::isMounted() const
{
    return m_isMounted;
}

void QVolume::setIsMounted(bool isMounted)
{
    m_isMounted = isMounted;
}

QString QVolume::mounted_root_uri() const
{
    return m_mounted_root_uri;
}

void QVolume::setMounted_root_uri(const QString &mounted_root_uri)
{
    m_mounted_root_uri = mounted_root_uri;
}

bool QVolume::isValid()
{
    return !m_unix_device.isEmpty();
}

QString QVolume::activation_root_uri() const
{
    return m_activation_root_uri;
}

void QVolume::setActivation_root_uri(const QString &activation_root_uri)
{
    m_activation_root_uri = activation_root_uri;
}

bool QVolume::is_removable() const
{
    return m_is_removable;
}

void QVolume::setIs_removable(bool is_removable)
{
    m_is_removable = is_removable;
}

QDrive QVolume::drive() const
{
    return m_drive;
}

void QVolume::setDrive(const QDrive &drive)
{
    m_drive = drive;
}

QString QVolume::drive_unix_device() const
{
    return m_drive_unix_device;
}

void QVolume::setDrive_unix_device(const QString &drive_unix_device)
{
    m_drive_unix_device = drive_unix_device;
}

QDebug operator<<(QDebug dbg, const QVolume &volume)
{
    dbg << "QVolume: {"
        << "name:" << volume.name() << ","
        << "unix_device:" << volume.unix_device() << ","
        << "icons:" << volume.icons() << ","
        << "is_Mounted:" << volume.isMounted() << ","
        << "is_removable:" << volume.is_removable() << ","
        << "mounted_root_uri:" << volume.mounted_root_uri()
        << "}";
    return dbg;
}
