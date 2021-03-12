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

#include "qdrive.h"

QDrive::QDrive()
{

}

QString QDrive::name() const
{
    return m_name;
}

void QDrive::setName(const QString &name)
{
    m_name = name;
}

QStringList QDrive::icons() const
{
    return m_icons;
}

void QDrive::setIcons(const QStringList &icons)
{
    m_icons = icons;
}

QStringList QDrive::symbolic_icons() const
{
    return m_symbolic_icons;
}

void QDrive::setSymbolic_icons(const QStringList &symbolic_icons)
{
    m_symbolic_icons = symbolic_icons;
}

bool QDrive::has_volumes() const
{
    return m_has_volumes;
}

void QDrive::setHas_volumes(bool has_volumes)
{
    m_has_volumes = has_volumes;
}

bool QDrive::can_eject() const
{
    return m_can_eject;
}

void QDrive::setCan_eject(bool can_eject)
{
    m_can_eject = can_eject;
}

bool QDrive::can_start() const
{
    return m_can_start;
}

void QDrive::setCan_start(bool can_start)
{
    m_can_start = can_start;
}

bool QDrive::can_start_degraded() const
{
    return m_can_start_degraded;
}

void QDrive::setCan_start_degraded(bool can_start_degraded)
{
    m_can_start_degraded = can_start_degraded;
}

bool QDrive::can_poll_for_media() const
{
    return m_can_poll_for_media;
}

void QDrive::setCan_poll_for_media(bool can_poll_for_media)
{
    m_can_poll_for_media = can_poll_for_media;
}

bool QDrive::can_stop() const
{
    return m_can_stop;
}

void QDrive::setCan_stop(bool can_stop)
{
    m_can_stop = can_stop;
}

bool QDrive::has_media() const
{
    return m_has_media;
}

void QDrive::setHas_media(bool has_media)
{
    m_has_media = has_media;
}

bool QDrive::is_media_check_automatic() const
{
    return m_is_media_check_automatic;
}

void QDrive::setIs_media_check_automatic(bool is_media_check_automatic)
{
    m_is_media_check_automatic = is_media_check_automatic;
}

bool QDrive::is_removable() const
{
    return m_is_removable;
}

void QDrive::setIs_removable(bool is_removable)
{
    m_is_removable = is_removable;
}

bool QDrive::is_media_removable() const
{
    return m_is_media_removable;
}

void QDrive::setIs_media_removable(bool is_media_removable)
{
    m_is_media_removable = is_media_removable;
}

GDriveStartStopType QDrive::start_stop_type() const
{
    return m_start_stop_type;
}

void QDrive::setStart_stop_type(const GDriveStartStopType &start_stop_type)
{
    m_start_stop_type = start_stop_type;
}

QString QDrive::unix_device() const
{
    return m_unix_device;
}

void QDrive::setUnix_device(const QString &unix_device)
{
    m_unix_device = unix_device;
}

bool QDrive::isValid()
{
    return !name().isEmpty();
}

QDebug operator<<(QDebug dbg, const QDrive &drive)
{
    dbg << "QDrive: {"
        << "name:" << drive.name() << ","
        << "unix_device:" << drive.unix_device() << ","
        << "icons:" << drive.icons() << ","
        << "is_removable" << drive.is_removable() << ","
        << "start_stop_type" << drive.start_stop_type() << ","
        << "can_stop:" << drive.can_stop()
        << "}";
    return dbg;
}
