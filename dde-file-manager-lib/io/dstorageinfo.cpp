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
#include <gio/gio.h>

#include "dstorageinfo.h"

DFM_BEGIN_NAMESPACE

static QString preprocessPath(const QString &path, DStorageInfo::PathHints hints)
{
    QFileInfo info(path);

    if (info.isSymLink()) {
        if (!info.exists() || !hints.testFlag(DStorageInfo::FollowSymlink))
            return info.dir().path();
    }

    return path;
}

class DStorageInfoPrivate
{
public:
    GFileInfo *gioInfo = nullptr;
};

DStorageInfo::DStorageInfo()
    : QStorageInfo()
    , d_ptr(new DStorageInfoPrivate())
{

}

DStorageInfo::DStorageInfo(const QString &path, PathHints hints)
    : DStorageInfo()
{
    setPath(preprocessPath(path, hints));
}

DStorageInfo::DStorageInfo(const QDir &dir, PathHints hints)
    : DStorageInfo()
{
    setPath(preprocessPath(dir.path(), hints));
}

DStorageInfo::~DStorageInfo()
{
    Q_D(DStorageInfo);

    if (d->gioInfo) {
        g_object_unref(d->gioInfo);
    }
}

void DStorageInfo::setPath(const QString &path, PathHints hints)
{
    QStorageInfo::setPath(preprocessPath(path, hints));

    Q_D(DStorageInfo);

    if (QStorageInfo::bytesTotal() <= 0) {
        if (d->gioInfo) {
            g_object_unref(d->gioInfo);
        }

        GFile *file = g_file_new_for_path(QFile::encodeName(path).constData());
        GError *error = nullptr;
        d->gioInfo = g_file_query_filesystem_info(file, "filesystem::*", nullptr, &error);

        if (error) {
            qWarning() << QString::fromLocal8Bit(error->message);

            g_error_free(error);
        }

        g_object_unref(file);
    }
}

QByteArray DStorageInfo::fileSystemType() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo) {
        return g_file_info_get_attribute_string(d->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
    }

    return QStorageInfo::fileSystemType();
}

qint64 DStorageInfo::bytesTotal() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo)
        return g_file_info_get_attribute_uint64(d->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);

    return QStorageInfo::bytesTotal();
}

qint64 DStorageInfo::bytesFree() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo) {
        quint64 used = g_file_info_get_attribute_uint64(d->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_USED);

        return bytesTotal() - used;
    }

    return QStorageInfo::bytesFree();
}

qint64 DStorageInfo::bytesAvailable() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo) {
        return g_file_info_get_attribute_uint64(d->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
    }

    return QStorageInfo::bytesFree();
}

bool DStorageInfo::isReadOnly() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo) {
        return g_file_info_get_attribute_boolean(d->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);
    }

    return QStorageInfo::isReadOnly();
}

void DStorageInfo::refresh()
{
    Q_D(DStorageInfo);

    QStorageInfo::refresh();
    setPath(rootPath());
}

bool DStorageInfo::inSameDevice(QString path1, QString path2, PathHints hints)
{
    path1 = QFileInfo(preprocessPath(path1, hints)).absoluteFilePath();
    path2 = QFileInfo(preprocessPath(path2, hints)).absoluteFilePath();

    DStorageInfo info1(path1, hints);

    // for avfs
    if (info1.fileSystemType() == "avfsd") {
        info1.setPath(path1.mid(info1.rootPath().length()));
    }

    DStorageInfo info2(path2, hints);

    // for avfs
    if (info2.fileSystemType() == "avfsd") {
        info2.setPath(path2.mid(info1.rootPath().length()));
    }

    if (info1 != info2)
        return false;

    // for gvfs
    if (info1.fileSystemType() == "gvfsd-fuse") {
        path1 = path1.mid(info1.rootPath().length() + 1);
        path2 = path2.mid(info2.rootPath().length() + 1);

        int seporator_index1 = path1.indexOf(QDir::separator());
        int seporator_index2 = path2.indexOf(QDir::separator());

        if (seporator_index1 != seporator_index2)
            return false;

        if (seporator_index1 == -1)
            return false;

        return path1.left(seporator_index1) == path2.left(seporator_index2);
    }

    return info1 == info2;
}

bool DStorageInfo::inSameDevice(const DUrl &url1, const DUrl &url2, PathHints hints)
{
    if (url1.scheme() != url2.scheme())
        return false;

    if (url1.isLocalFile()) {
        return inSameDevice(url1.toLocalFile(), url2.toLocalFile(), hints);
    }

    return url1.host() == url2.host() && url1.port() == url1.port();
}

DFM_END_NAMESPACE
