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

#ifndef QDISKINFO_H
#define QDISKINFO_H

#include <QString>
#include <QDebug>
#include "durl.h"

class DAbstractFileInfo;
class QDiskInfo
{
public:
    QDiskInfo();

    QString id() const;
    void setId(const QString &id);

    QString name() const;
    void setName(const QString &name);

    QString type() const;
    void setType(const QString &type);

    QString unix_device() const;
    void setUnix_device(const QString &unix_device);

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString mounted_root_uri() const;
    void setMounted_root_uri(const QString &mounted_root_uri);

    QString iconName() const;
    void setIconName(const QString &iconName);

    bool can_unmount() const;
    void setCan_unmount(bool can_unmount);

    bool can_eject() const;
    void setCan_eject(bool can_eject);

    qulonglong used() const;
    void setUsed(const qulonglong &used);

    qulonglong total() const;
    void setTotal(const qulonglong &total);

    qulonglong free() const;
    void setFree(const qulonglong &free);

//    DUrl mounted_url() const;
//    void setMounted_url(const DUrl &mounted_url);

    bool isNativeCustom() const;
    void setIsNativeCustom(bool isNativeCustom);

    bool can_mount() const;
    void setCan_mount(bool can_mount);


    void updateGvfsFileSystemInfo(int retryTimes=3);

    bool read_only() const;
    void setRead_only(bool read_only);

    QString activation_root_uri() const;
    void setActivation_root_uri(const QString &activation_root_uri);

    bool isValid();

    bool is_removable() const;
    void setIs_removable(bool is_removable);

    bool has_volume() const;
    void setHas_volume(bool has_volume);

    QString id_filesystem() const;
    void setId_filesystem(const QString &id_filesystem);

    QString default_location() const;
    void setDefault_location(const QString &default_location);

    QString drive_unix_device() const;
    void setDrive_unix_device(const QString &drive_unix_device);

    static QDiskInfo getDiskInfo(const DAbstractFileInfo &fileInfo);

private:
    QString m_id;
    QString m_name;
    QString m_type;
    QString m_drive_unix_device;
    QString m_unix_device;
    QString m_uuid;
    QString m_activation_root_uri;
    QString m_mounted_root_uri;
    QString m_iconName;
    QString m_id_filesystem;
    QString m_default_location;
    bool m_is_removable = false;
    bool m_can_mount = false;
    bool m_can_unmount = false;
    bool m_can_eject = false;
    bool m_read_only = false;
    bool m_has_volume = false;
    qulonglong m_used = 0;
    qulonglong m_total = 0;
    qulonglong m_free = 0;

//    DUrl m_mounted_url;
    /*if true show in computerview for especially used*/
    bool m_isNativeCustom = false;

};

typedef QList<QDiskInfo> QDiskInfoList;

Q_DECLARE_METATYPE(QDiskInfo)
Q_DECLARE_METATYPE(QDiskInfoList)

QDebug operator<<(QDebug dbg, const QDiskInfo &info);

#endif // QDISKINFO_H
