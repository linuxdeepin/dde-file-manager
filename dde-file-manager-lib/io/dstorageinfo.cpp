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

#include <QRegularExpression>

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

class DStorageInfoPrivate : public QSharedData
{
public:
    ~DStorageInfoPrivate() {
        if (gioInfo) {
            g_object_unref(gioInfo);
        }
    }

    GFileInfo *gioInfo = nullptr;
    QString rootPath;
    QByteArray device;
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

DStorageInfo::DStorageInfo(const DStorageInfo &other)
    : QStorageInfo(other)
    , d_ptr(other.d_ptr)
{

}

DStorageInfo::~DStorageInfo()
{

}

DStorageInfo &DStorageInfo::operator=(const DStorageInfo &other)
{
    d_ptr = other.d_ptr;

    return *this;
}

void DStorageInfo::setPath(const QString &path, PathHints hints)
{
    QStorageInfo::setPath(preprocessPath(path, hints));

    Q_D(DStorageInfo);

    d_ptr.detach();

    if (QStorageInfo::bytesTotal() <= 0) {
        GFile *file = g_file_new_for_path(QFile::encodeName(path).constData());
        GError *error = nullptr;
        d->gioInfo = g_file_query_filesystem_info(file, "filesystem::*", nullptr, &error);

        if (error) {
            qWarning() << QString::fromLocal8Bit(error->message);

            g_error_free(error);
            error = nullptr;
        }

        GMount *mount = g_file_find_enclosing_mount(file, nullptr, &error);

        if (error) {
            qWarning() << QString::fromLocal8Bit(error->message);

            g_error_free(error);
        } else {
            GFile *root_file = g_mount_get_root(mount);
            char *root_path = g_file_get_path(root_file);

            d->rootPath = QFile::decodeName(root_path);
            d->device = QStorageInfo::device();

            if (d->device == QByteArrayLiteral("gvfsd-fuse")) {
                char *uri = g_file_get_uri(root_file);

                d->device = QByteArray(uri);
                g_free(uri);
            }

            g_free(root_path);
            g_object_unref(root_file);
            g_object_unref(mount);
        }

        g_object_unref(file);
    }
}

QString DStorageInfo::rootPath() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo && !d->rootPath.isEmpty()) {
        return d->rootPath;
    }

    return QStorageInfo::rootPath();
}

QByteArray DStorageInfo::device() const
{
    Q_D(const DStorageInfo);

    if (d->device.isEmpty())
        return QStorageInfo::device();

    return d->device;
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

    return QStorageInfo::bytesAvailable();
}

bool DStorageInfo::isReadOnly() const
{
    Q_D(const DStorageInfo);

    if (d->gioInfo) {
        return g_file_info_get_attribute_boolean(d->gioInfo, G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);
    }

    return QStorageInfo::isReadOnly();
}

bool DStorageInfo::isLocalDevice() const
{
    return device().startsWith("/dev/");
}

bool DStorageInfo::isLowSpeedDevice() const
{
    const QString &device = this->device();

    return device.startsWith("mtp://")
            || device.startsWith("gphoto://")
            || device.startsWith("gphoto2://")
            || device.startsWith("smb-share://");
}

bool DStorageInfo::isValid() const
{
    Q_D(const DStorageInfo);

    return QStorageInfo::isValid();
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

bool DStorageInfo::isLocalDevice(const QString &path)
{
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/.+$",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);

    if (regExp.match(path, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch())
        return false;

    return DStorageInfo(path).device().startsWith("/dev/");
}

bool DStorageInfo::isLowSpeedDevice(const QString &path)
{
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/(?'scheme'\\w+):.+/",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);

    const QRegularExpressionMatch &match = regExp.match(path, 0, QRegularExpression::NormalMatch,
                                                        QRegularExpression::DontCheckSubjectStringMatchOption);

    if (match.hasMatch()) {
        const QString &scheme = match.captured("scheme");

        return (scheme == "mtp" || scheme == "gphoto" || scheme == "gphoto2" || scheme == "smb-share");
    }

    return DStorageInfo(path).isLowSpeedDevice();
}

DFM_END_NAMESPACE

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug debug, const DFM_NAMESPACE::DStorageInfo &info)
{
    QDebugStateSaver saver(debug);
    Q_UNUSED(saver)
    debug.nospace();
    debug.noquote();
    debug << "DStorageInfo(";
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
    debug<< ')';
    return debug;
}
QT_END_NAMESPACE
