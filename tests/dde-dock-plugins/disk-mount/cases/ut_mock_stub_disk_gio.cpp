/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "ut_mock_stub_disk_gio.h"

#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <QCoreApplication>

char giomount_buffer[1024];
DGioMount * createFromPath_gioMount_stub(QString path, QObject *parent)
{
    //Gio::Mount mount_dummy;
    return (DGioMount*)giomount_buffer;
}


bool canUnmount_stub()
{
    return true;
}

GMount *   g_file_find_enclosing_mount_stub       (GFile                      *file,
                                                           GCancellable               *cancellable,
                                                           GError                    **error)
{
    return (GMount*)giomount_buffer;
}

void  g_mount_unmount_with_operation_stub    (GMount              *mount,
                                               GMountUnmountFlags   flags,
                                               GMountOperation     *mount_operation,
                                               GCancellable        *cancellable,
                                               GAsyncReadyCallback  callback,
                                               gpointer             user_data)
{
    //callback((GObject *)giomount_buffer,(GAsyncResult*)giomount_buffer,nullptr); // crash max-lv
}

bool exists_qfileinfo_stub()
{
    return true;
}

bool exists_qfileinfo_param_stub(const QString &file)
{
    return true;
}

bool permission_stub(QFile::Permissions permissions)
{
    return true;
}

QString name_stub()
{
    return gio_dummy_device_name;
}

const QList<QExplicitlySharedDataPointer<DGioMount> > getMounts_stub()
{
    QList<QExplicitlySharedDataPointer<DGioMount> > listGioMount;

    DGioMount *gioMount = DGioMount::createFromPath(dgio_devpath_stub());
    if (gioMount) {
        QExplicitlySharedDataPointer<DGioMount> mntPtr(gioMount);
        listGioMount.append(mntPtr);
    }
    return listGioMount;
}


QString g_vfs_mount_point = "smb///10.1.1.100";

const QList<QExplicitlySharedDataPointer<DGioMount> > get_gvfs_Mounts_stub()
{
    QList<QExplicitlySharedDataPointer<DGioMount> > listGioMount;

    DGioMount *gioMount = DGioMount::createFromPath(BLK_DEVICE_MOUNT_POINT);
    if (gioMount) {
        QExplicitlySharedDataPointer<DGioMount> mntPtr(gioMount);
        listGioMount.append(mntPtr);
    }
    return listGioMount;
}

QExplicitlySharedDataPointer<DGioFile> get_gvfs_RootFile_stub()
{
    QExplicitlySharedDataPointer<DGioFile> rootfile = QExplicitlySharedDataPointer<DGioFile>(DGioFile::createFromPath(BLK_DEVICE_MOUNT_POINT));
    return rootfile;
}

QExplicitlySharedDataPointer<DGioFile> getRootFile_stub()
{
    QExplicitlySharedDataPointer<DGioFile> rootfile = QExplicitlySharedDataPointer<DGioFile>(DGioFile::createFromPath(BLK_DEVICE_MOUNT_POINT));
    return rootfile;
}

QExplicitlySharedDataPointer<DGioFileInfo> createFileInfo_stub(QString attr, DGioFileQueryInfoFlags queryInfoFlags, unsigned long timeout_msec)
{
    Q_UNUSED(attr);
    Q_UNUSED(queryInfoFlags);
    Q_UNUSED(timeout_msec);

    QExplicitlySharedDataPointer<DGioFileInfo> fileSystemInfo = QExplicitlySharedDataPointer<DGioFileInfo>( new DGioFileInfo(nullptr));
    return fileSystemInfo;
}

QExplicitlySharedDataPointer<DGioFileInfo> createFileSystemInfo_stub(QString attr)
{
    Q_UNUSED(attr);

    QExplicitlySharedDataPointer<DGioFileInfo> fileSystemInfo = QExplicitlySharedDataPointer<DGioFileInfo>( new DGioFileInfo(nullptr));
    return fileSystemInfo;
}

QExplicitlySharedDataPointer<DGioFileInfo> createFileSystemInfo_return_null_stub(QString attr, DGioFileQueryInfoFlags queryInfoFlags, unsigned long timeout_msec )
{
    return QExplicitlySharedDataPointer<DGioFileInfo>(nullptr);
}

quint64 fsTotalBytes_2KB_stub()
{
    return 2*1024;
}

quint64 fsUsedBytes_1KB_stub()
{
    return 1024;
}

quint64 fsFreeBytes_1KB_stub()
{
    return 1024;
}

QStringList themedIconNames_stub()
{
    QStringList stringlist;
    stringlist.push_back(gio_dummy_device_icon_name);
    return stringlist;
}

QStringList themedIconNames_empty_stub()
{
    QStringList stringlist;
    return stringlist;
}

QByteArrayList gio_mountPoints_more_stub()
{
    QByteArrayList arraylist;
    arraylist.append(BLK_DEVICE_MOUNT_POINT.toUtf8());
    arraylist.append(BLK_DEVICE_MOUNT_POINT.toUtf8());
    return arraylist;
}

QString dgio_devpath_stub()
{
    return BLK_DEVICE_PATH;// QCoreApplication::applicationDirPath();
}

QString findExecutable_QStandardPaths_return_empty_stub(const QString &executableName, const QStringList &paths)
{
    return "";
}

QStringList blockDevices_DDiskManager_stub(QVariantMap options)
{
    QStringList devlist;
    devlist << dgio_devpath_stub();
    return devlist;
}

bool hasFileSystem_stub()
{
    return true;
}

bool hintIgnore_stub()
{
    return false;
}

bool hintSystem_stub()
{
    return false;
}

QString scheme_burn_stub()
{
    return "burn";
}

bool isShadowed_stub()
{
    return false;
}

void detach_stub()
{}

DGioFileType getFileType_stub()
{
    return DGioFileType::FILE_TYPE_DIRECTORY;
}
