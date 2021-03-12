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

#include "qmount.h"

QMount::QMount()
{

}

QString QMount::name() const
{
    return m_name;
}

QString QMount::uuid() const
{
    return m_uuid;
}

QString QMount::default_location() const
{
    return m_default_location;
}

void QMount::setDefault_location(const QString &default_location)
{
    m_default_location = default_location;
}

QStringList QMount::icons() const
{
    return m_icons;
}

QStringList QMount::symbolic_icons() const
{
    return m_symbolic_icons;
}

void QMount::setSymbolic_icons(const QStringList &symbolic_icons)
{
    m_symbolic_icons = symbolic_icons;
}

bool QMount::can_unmount() const
{
    return m_can_unmount;
}

void QMount::setCan_unmount(bool can_unmount)
{
    m_can_unmount = can_unmount;
}

bool QMount::can_eject() const
{
    return m_can_eject;
}

void QMount::setCan_eject(bool can_eject)
{
    m_can_eject = can_eject;
}

bool QMount::is_shadowed() const
{
    return m_is_shadowed;
}

void QMount::setIs_shadowed(bool is_shadowed)
{
    m_is_shadowed = is_shadowed;
}

void QMount::setName(const QString &name)
{
    m_name = name;
}

void QMount::setIcons(const QStringList &icons)
{
    m_icons = icons;
}

QString QMount::mounted_root_uri() const
{
    return m_mounted_root_uri;
}

void QMount::setMounted_root_uri(const QString &mounted_root_uri)
{
    m_mounted_root_uri = mounted_root_uri;
}

QDebug operator<<(QDebug dbg, const QMount &mount)
{
    dbg << "QMount: {"
        << "name:" << mount.name() << ","
        << "mounted root uri:" << mount.mounted_root_uri() << ","
        << "can_unmount:" << mount.can_unmount() << ","
        << "icon:" << mount.icons() << ","
        << "is_shadowed:" << mount.is_shadowed()
        << "}";
    return dbg;
}
