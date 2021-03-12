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

#ifndef QDRIVE_H
#define QDRIVE_H

#include <QString>
#include <QDebug>

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

class QDrive
{
public:
    QDrive();

    QString name() const;
    void setName(const QString &name);

    QStringList icons() const;
    void setIcons(const QStringList &icons);

    QStringList symbolic_icons() const;
    void setSymbolic_icons(const QStringList &symbolic_icons);

    bool has_volumes() const;
    void setHas_volumes(bool has_volumes);

    bool can_eject() const;
    void setCan_eject(bool can_eject);

    bool can_start() const;
    void setCan_start(bool can_start);

    bool can_start_degraded() const;
    void setCan_start_degraded(bool can_start_degraded);

    bool can_poll_for_media() const;
    void setCan_poll_for_media(bool can_poll_for_media);

    bool can_stop() const;
    void setCan_stop(bool can_stop);

    bool has_media() const;
    void setHas_media(bool has_media);

    bool is_media_check_automatic() const;
    void setIs_media_check_automatic(bool is_media_check_automatic);

    bool is_removable() const;
    void setIs_removable(bool is_removable);

    bool is_media_removable() const;
    void setIs_media_removable(bool is_media_removable);

    GDriveStartStopType start_stop_type() const;
    void setStart_stop_type(const GDriveStartStopType &start_stop_type);

    QString unix_device() const;
    void setUnix_device(const QString &unix_device);

    bool isValid();

private:
    QString m_name;
    QString m_unix_device;
    QStringList m_icons;
    QStringList m_symbolic_icons;
    bool m_has_volumes = false;
    bool m_can_eject = false;
    bool m_can_start = false;
    bool m_can_start_degraded = false;
    bool m_can_poll_for_media = false;
    bool m_can_stop = false;
    bool m_has_media = false;
    bool m_is_media_check_automatic = false;
    bool m_is_removable = false;
    bool m_is_media_removable = false;
    GDriveStartStopType m_start_stop_type;
};

Q_DECLARE_METATYPE(QDrive)

QDebug operator<<(QDebug dbg, const QDrive& drive);

#endif // QDRIVE_H
