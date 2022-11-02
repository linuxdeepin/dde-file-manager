/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "storageinfo.h"

#include <QDebug>

#include <sys/stat.h>

DPFILEOPERATIONS_USE_NAMESPACE

static QString preprocessPath(const QString &path, StorageInfo::PathHints hints)
{
    QFileInfo info(path);

    if (info.isSymLink()) {
        if (!info.exists() || !hints.testFlag(StorageInfo::FollowSymlink))
            return info.dir().path();
    }

    return path;
}

StorageInfo::StorageInfo()
    : QStorageInfo(), dataInfo(new StorageDataInfo())
{
}

StorageInfo::StorageInfo(const QString &path, PathHints hints)
    : StorageInfo()
{
    setPath(path, hints);
}

StorageInfo::StorageInfo(const QDir &dir, PathHints hints)
    : StorageInfo()
{
    setPath(dir.path(), hints);
}

StorageInfo::StorageInfo(const StorageInfo &other)
    : QStorageInfo(other), dataInfo(other.dataInfo)
{
}

StorageInfo::~StorageInfo()
{
}

StorageInfo &StorageInfo::operator=(const StorageInfo &other)
{
    dataInfo = other.dataInfo;

    return *this;
}
/*!
 * \brief StorageInfo::setPath 设置storage的路径
 * \param path 文件路径
 * \param hints
 */
void StorageInfo::setPath(const QString &path, PathHints hints)
{
    QStorageInfo::setPath(preprocessPath(path, hints));

    dataInfo.detach();

    if (QStorageInfo::bytesTotal() <= 0) {
        GFile *file = g_file_new_for_path(QFile::encodeName(path).constData());
        GError *error = nullptr;
        dataInfo->gioInfo = g_file_query_filesystem_info(file, "filesystem::*", nullptr, &error);

        if (error) {
            g_error_free(error);
            error = nullptr;
        }

        GMount *mount = g_file_find_enclosing_mount(file, nullptr, &error);

        if (error) {
            g_error_free(error);
        } else if (mount) {
            GFile *root_file = g_mount_get_root(mount);
            char *root_path = g_file_get_path(root_file);

            dataInfo->rootPath = QFile::decodeName(root_path);
            dataInfo->device = QStorageInfo::device();

            if (dataInfo->device == QByteArrayLiteral("gvfsd-fuse")) {
                char *uri = g_file_get_uri(root_file);

                dataInfo->device = QByteArray(uri);
                g_free(uri);
            }

            g_free(root_path);
            g_object_unref(root_file);
            g_object_unref(mount);
        }

        g_object_unref(file);
    }
}
/*!
 * \brief StorageInfo::rootPath get root path
 * \return
 */
QString StorageInfo::rootPath() const
{
    if (dataInfo->gioInfo && !dataInfo->rootPath.isEmpty()) {
        return dataInfo->rootPath;
    }

    return QStorageInfo::rootPath();
}
/*!
 * \brief StorageInfo::device device,such as: /dev/sda
 * \return
 */
QByteArray StorageInfo::device() const
{

    if (dataInfo->device.isEmpty())
        return QStorageInfo::device();

    return dataInfo->device;
}
/*!
 * \brief StorageInfo::fileSystemType file system type,such as:ext4,ntfs,vfat
 * \return
 */
QByteArray StorageInfo::fileSystemType() const
{
    if (dataInfo->gioInfo) {
        return g_file_info_get_attribute_string(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
    }

    return QStorageInfo::fileSystemType();
}
/*!
 * \brief StorageInfo::bytesTotal the device total size
 * \return
 */
qint64 StorageInfo::bytesTotal() const
{
    if (dataInfo->gioInfo) {
        if (g_file_info_has_attribute(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE)) {
            return static_cast<qint64>(g_file_info_get_attribute_uint64(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE));
        } else {
            qInfo() << "file do not support G_FILE_ATTRIBUTE_FILESYSTEM_SIZE, returns max of qint64";
            return std::numeric_limits<qint64>::max();
        }
    }

    return QStorageInfo::bytesTotal();
}
/*!
 * \brief StorageInfo::bytesFree device's free size
 * \return
 */
qint64 StorageInfo::bytesFree() const
{
    if (dataInfo->gioInfo) {
        if (g_file_info_has_attribute(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_USED)) {
            quint64 used = g_file_info_get_attribute_uint64(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
            return bytesTotal() - static_cast<qint64>(used);
        } else {
            qInfo() << "file do not support G_FILE_ATTRIBUTE_FILESYSTEM_USED, returns max of qint64";
            return std::numeric_limits<qint64>::max();
        }
    }

    return QStorageInfo::bytesFree();
}
/*!
 * \brief StorageInfo::bytesAvailable device's Available size, != free size
 * \return
 */
qint64 StorageInfo::bytesAvailable() const
{
    if (dataInfo->gioInfo) {
        if (g_file_info_has_attribute(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_FREE)) {
            return static_cast<qint64>(g_file_info_get_attribute_uint64(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_FREE));
        } else {
            qInfo() << "file do not support G_FILE_ATTRIBUTE_FILESYSTEM_FREE, returns max of qint64";
            return std::numeric_limits<qint64>::max();
        }
    }

    return QStorageInfo::bytesAvailable();
}
/*!
 * \brief StorageInfo::isReadOnly device is ready only
 * \return
 */
bool StorageInfo::isReadOnly() const
{
    if (dataInfo->gioInfo) {
        return g_file_info_get_attribute_boolean(dataInfo->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);
    }

    return QStorageInfo::isReadOnly();
}
/*!
 * \brief StorageInfo::isLocalDevice device is local device,such as : /dev/sda
 * \return
 */
bool StorageInfo::isLocalDevice() const
{
    return device().startsWith("/dev/");
}

bool StorageInfo::isValid() const
{
    return QStorageInfo::isValid();
}

void StorageInfo::refresh()
{
    QStorageInfo::refresh();
    setPath(rootPath());
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug debug, const StorageInfo &info)
{
    QDebugStateSaver saver(debug);
    Q_UNUSED(saver)
    debug.nospace();
    debug.noquote();
    debug << "StorageInfo(";
    if (info.isValid()) {
        debug << '"' << info.rootPath() << '"';
        if (!info.fileSystemType().isEmpty())
            debug << ", type=" << info.fileSystemType();
        if (!info.name().isEmpty())
            debug << ", name=\"" << info.name() << '"';
        if (!info.device().isEmpty())
            debug << ", device=\"" << info.device() << '"';
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
        if (!info.subvolume().isEmpty())
            debug << ", subvolume=\"" << info.subvolume() << '"';
#endif
        if (info.isReadOnly())
            debug << " [read only]";
        debug << (info.isReady() ? " [ready]" : " [not ready]");
        if (info.bytesTotal() > 0) {
            debug << ", bytesTotal=" << info.bytesTotal() << ", bytesFree=" << info.bytesFree()
                  << ", bytesAvailable=" << info.bytesAvailable();
        }
    } else {
        debug << "invalid";
    }
    debug << ')';
    return debug;
}
QT_END_NAMESPACE
