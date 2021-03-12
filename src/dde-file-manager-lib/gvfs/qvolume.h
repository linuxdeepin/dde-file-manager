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

#ifndef QVOLUME_H
#define QVOLUME_H

#include <QString>
#include <QMap>
#include <QDebug>

#include "qdrive.h"


class QVolume
{
public:
    QVolume();

    QString name() const;
    void setName(const QString &name);

    QString uuid() const;
    void setUuid(const QString &uuid);

    QStringList icons() const;
    void setIcons(const QStringList &icons);

    QStringList symbolic_icons() const;
    void setSymbolic_icons(const QStringList &symbolic_icons);

    bool can_mount() const;
    void setCan_mount(bool can_mount);

    bool can_eject() const;
    void setCan_eject(bool can_eject);

    bool should_automount() const;
    void setShould_automount(bool should_automount);

    QString unix_device() const;
    void setUnix_device(const QString &unix_device);

    QString lable() const;
    void setLable(const QString &lable);

    QString nfs_mount() const;
    void setNfs_mount(const QString &nfs_mount);

    bool isMounted() const;
    void setIsMounted(bool isMounted);

    QString mounted_root_uri() const;
    void setMounted_root_uri(const QString &mounted_root_uri);

    bool isValid();

    QString activation_root_uri() const;
    void setActivation_root_uri(const QString &activation_root_uri);

    bool is_removable() const;
    void setIs_removable(bool is_removable);

    QDrive drive() const;
    void setDrive(const QDrive &drive);

    QString drive_unix_device() const;
    void setDrive_unix_device(const QString &drive_unix_device);

private:
    QString m_name;
    QString m_drive_unix_device;
    QString m_unix_device;
    QString m_lable;
    QString m_uuid;
    QString m_nfs_mount;
    QStringList m_icons;
    QStringList m_symbolic_icons;
    QString m_activation_root_uri;
    QString m_mounted_root_uri;
    bool m_can_mount = false;
    bool m_can_eject = false;
    bool m_should_automount = false;
    bool m_isMounted = false;
    bool m_is_removable = false;
    QDrive m_drive;
};

QDebug operator<<(QDebug dbg, const QVolume& volume);

#endif // QVOLUME_H
