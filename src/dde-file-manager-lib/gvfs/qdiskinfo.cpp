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

#include "qdiskinfo.h"
#include "deviceinfo/udisklistener.h"

#include "app/define.h"
#include "singleton.h"

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

QDiskInfo::QDiskInfo()
{

}

QString QDiskInfo::id() const
{
    return m_id;
}

void QDiskInfo::setId(const QString &id)
{
    m_id = id;
}

QString QDiskInfo::name() const
{
    return m_name;
}

void QDiskInfo::setName(const QString &name)
{
    m_name = name;
}

QString QDiskInfo::type() const
{
    return m_type;
}

void QDiskInfo::setType(const QString &type)
{
    m_type = type;
}

QString QDiskInfo::unix_device() const
{
    return m_unix_device;
}

void QDiskInfo::setUnix_device(const QString &unix_device)
{
    m_unix_device = unix_device;
}

QString QDiskInfo::uuid() const
{
    return m_uuid;
}

void QDiskInfo::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString QDiskInfo::mounted_root_uri() const
{
    return m_mounted_root_uri;
}

void QDiskInfo::setMounted_root_uri(const QString &mounted_root_uri)
{
    m_mounted_root_uri = mounted_root_uri;
}

QString QDiskInfo::iconName() const
{
    return m_iconName;
}

void QDiskInfo::setIconName(const QString &iconName)
{
    m_iconName = iconName;
}

bool QDiskInfo::can_unmount() const
{
    return m_can_unmount;
}

void QDiskInfo::setCan_unmount(bool can_unmount)
{
    m_can_unmount = can_unmount;
}

bool QDiskInfo::can_eject() const
{
    return m_can_eject;
}

void QDiskInfo::setCan_eject(bool can_eject)
{
    m_can_eject = can_eject;
}

qulonglong QDiskInfo::used() const
{
    return m_used;
}

void QDiskInfo::setUsed(const qulonglong &used)
{
    m_used = used;
}

qulonglong QDiskInfo::total() const
{
    return m_total;
}

void QDiskInfo::setTotal(const qulonglong &total)
{
    m_total = total;
}

qulonglong QDiskInfo::free() const
{
    return m_free;
}

void QDiskInfo::setFree(const qulonglong &free)
{
    m_free = free;
}

//DUrl QDiskInfo::mounted_url() const
//{
//    return m_mounted_url;
//}

//void QDiskInfo::setMounted_url(const DUrl &mounted_url)
//{
//    m_mounted_url = mounted_url;
//}

bool QDiskInfo::isNativeCustom() const
{
    return m_isNativeCustom;
}

void QDiskInfo::setIsNativeCustom(bool isNativeCustom)
{
    m_isNativeCustom = isNativeCustom;
}

bool QDiskInfo::can_mount() const
{
    return m_can_mount;
}

void QDiskInfo::setCan_mount(bool can_mount)
{
    m_can_mount = can_mount;
}

void QDiskInfo::updateGvfsFileSystemInfo(int retryTimes/*=3*/)
{
    if (m_mounted_root_uri.isEmpty()) {
        return;
    }
    std::string file_uri = m_mounted_root_uri.toStdString();
    GFile *file;
    GFileInfo *systemInfo;
    GFileInfo *info;
    GError *error;
    file = g_file_new_for_uri(file_uri.data());
    if (file == nullptr) {
        return;
    }
    error = nullptr;
    // 根据 API 文档，需要什么属性就去获取什么属性，全部拿可能影响性能。
    systemInfo = g_file_query_filesystem_info(file,
                                              G_FILE_ATTRIBUTE_FILESYSTEM_SIZE ","
                                              G_FILE_ATTRIBUTE_FILESYSTEM_FREE ","
                                              G_FILE_ATTRIBUTE_FILESYSTEM_USED ","
                                              G_FILE_ATTRIBUTE_FILESYSTEM_READONLY,
                                              nullptr, &error);
    if (error) {
        g_error_free(error);
        error = nullptr;
    }
    info = g_file_query_info(file, G_FILE_ATTRIBUTE_ID_FILESYSTEM, G_FILE_QUERY_INFO_NONE, nullptr, &error);
    if (info == nullptr && error) {
        if (systemInfo)
            g_object_unref(systemInfo);
        g_object_unref(file);
        qWarning() << "g_file_query_filesystem_info" << error->message << error->code;
        if (error->code == 0 && retryTimes > 0) {
            g_error_free(error);
            return updateGvfsFileSystemInfo(--retryTimes);
        }

        g_error_free(error);
        return;
    }

    m_total = g_file_info_get_attribute_uint64(systemInfo, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
    m_free = g_file_info_get_attribute_uint64(systemInfo, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
    m_used = g_file_info_get_attribute_uint64(systemInfo, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
    m_read_only = g_file_info_get_attribute_boolean(systemInfo, G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);
    m_id_filesystem = QString(g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_ID_FILESYSTEM));

    if (m_used == 0) {
        m_used = m_total - m_free;
    }

    g_object_unref(systemInfo);
    g_object_unref(info);
    g_object_unref(file);

}

bool QDiskInfo::read_only() const
{
    return m_read_only;
}

void QDiskInfo::setRead_only(bool read_only)
{
    m_read_only = read_only;
}

QString QDiskInfo::activation_root_uri() const
{
    return m_activation_root_uri;
}

void QDiskInfo::setActivation_root_uri(const QString &activation_root_uri)
{
    m_activation_root_uri = activation_root_uri;
}

bool QDiskInfo::isValid()
{
    if (id().isEmpty()) {
        return false;
    }
    return true;
}

bool QDiskInfo::is_removable() const
{
    return m_is_removable;
}

void QDiskInfo::setIs_removable(bool is_removable)
{
    m_is_removable = is_removable;
}

bool QDiskInfo::has_volume() const
{
    return m_has_volume;
}

void QDiskInfo::setHas_volume(bool has_volume)
{
    m_has_volume = has_volume;
}

QString QDiskInfo::id_filesystem() const
{
    return m_id_filesystem;
}

void QDiskInfo::setId_filesystem(const QString &id_filesystem)
{
    m_id_filesystem = id_filesystem;
}

QString QDiskInfo::default_location() const
{
    return m_default_location;
}

void QDiskInfo::setDefault_location(const QString &default_location)
{
    m_default_location = default_location;
}

QString QDiskInfo::drive_unix_device() const
{
    return m_drive_unix_device;
}

void QDiskInfo::setDrive_unix_device(const QString &drive_unix_device)
{
    m_drive_unix_device = drive_unix_device;
}

QDiskInfo QDiskInfo::getDiskInfo(const DAbstractFileInfo &fileInfo)
{
    QDiskInfo info;
    qDebug() << fileInfo.filePath();
    UDiskDeviceInfoPointer uDiskDeviceInfoPointer = deviceListener->getDeviceByPath(fileInfo.filePath());
    if (!uDiskDeviceInfoPointer) {
        uDiskDeviceInfoPointer = deviceListener->getDeviceByFilePath(fileInfo.filePath());
    }
    if (uDiskDeviceInfoPointer) {
        info = uDiskDeviceInfoPointer->getDiskInfo();
    }
    return info;
}


QDebug operator<<(QDebug dbg, const QDiskInfo &info)
{
    dbg.nospace() << "QDiskInfo(";
    dbg.nospace() << "id: " << info.id() << ",";
    dbg.nospace() << "name: " << info.name() << ",";
    dbg.nospace() << "type: " << info.type() << ",";
    dbg.nospace() << "drive_unix_device: " << info.drive_unix_device() << ",";
    dbg.nospace() << "unix_device: " << info.unix_device() << ",";
    dbg.nospace() << "uuid: " << info.uuid() << ",";
    dbg.nospace() << "activation_root_uri: " << info.activation_root_uri() << ",";
    dbg.nospace() << "mounted_root_uri: " << info.mounted_root_uri() << ",";
    dbg.nospace() << "id_filesystem: " << info.id_filesystem() << ",";
    dbg.nospace() << "default_location: " << info.default_location() << ",";
    dbg.nospace() << "iconName: " << info.iconName() << ",";
    dbg.nospace() << "is_removable: " << info.is_removable() << ",";
    dbg.nospace() << "can_mount: " << info.can_mount() << ",";
    dbg.nospace() << "can_unmount: " << info.can_unmount() << ",";
    dbg.nospace() << "can_eject: " << info.can_eject() << ",";
    dbg.nospace() << "used: " << info.used() << ",";
    dbg.nospace() << "total: " << info.total() << ",";
//    dbg.nospace() << "mounted_url: " << info.mounted_url() << ",";
    dbg.nospace() << ")";
    return dbg;
}
