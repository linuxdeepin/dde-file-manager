/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include <gio/gio.h>
#include <QFileInfo>
#include <QDir>
#include "diskcontrolwidget.h"

#include "dattachedvfsdevice.h"
#include <dgiofile.h>
#include <dgiofileinfo.h>

namespace {
void unmount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    Q_UNUSED(user_data);

    gboolean succeeded;
    GError *error = nullptr;
    succeeded = g_mount_unmount_with_operation_finish (G_MOUNT (object), res, &error);

    if (!succeeded) {
        qWarning()<<"can't umount the device for error code:" << error->code << ", error message:" << error->code;
        DiskControlWidget::NotifyMsg(DiskControlWidget::tr("Cannot unmount the device"), DiskControlWidget::tr("") );
    }
    g_object_unref (G_MOUNT (object));
}

void unmount_mounted(const QString &mount_path)
{
    if (mount_path.isEmpty())
        return;

    qInfo()<<"umount the device:" << mount_path ;
    GFile *file = g_file_new_for_path(QFile::encodeName(mount_path));
    if (file == nullptr)
        return;

    GError *error = nullptr;
    GMount *mount = g_file_find_enclosing_mount (file, nullptr, &error);
    if (mount == nullptr) {
        bool no_permission = false;

        QFileInfo fileInfo(QUrl(mount_path).toLocalFile());

        while (!fileInfo.exists() && fileInfo.fileName() != QDir::rootPath() && !fileInfo.absolutePath().isEmpty()) {
            fileInfo.setFile(fileInfo.absolutePath());
        }

        if (fileInfo.exists()) {
            if (getuid() == fileInfo.ownerId()) {
                if (!fileInfo.permission(QFile::ReadOwner | QFile::ExeOwner))
                    no_permission = true;
            } else if (getgid() == fileInfo.groupId()) {
                if (!fileInfo.permission(QFile::ReadGroup | QFile::ExeGroup))
                    no_permission = true;
            } else if (!fileInfo.permission(QFile::ReadOther | QFile::ExeOther)) {
                no_permission = true;
            }
        }

        if (no_permission) {
            QString user_name = fileInfo.owner();

            if (fileInfo.absoluteFilePath().startsWith("/media/")) {
                user_name = fileInfo.baseName();
            }
            DiskControlWidget::NotifyMsg(DiskControlWidget::tr("The disk is mounted by user \"%1\", you cannot unmount it."), DiskControlWidget::tr("") );
            return;
        }

        DiskControlWidget::NotifyMsg(DiskControlWidget::tr("Cannot find the mounted device"), DiskControlWidget::tr("") );
        return;
    }

    GMountOperation *mount_op = g_mount_operation_new ();
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_FORCE;
    GAsyncReadyCallback callback = unmount_done_cb;
    g_mount_unmount_with_operation(mount, flags, mount_op, nullptr, callback, nullptr);
    g_object_unref (mount_op);
    g_object_unref (file);
}
}

/*!
 * \class DAttachedVfsDevice
 *
 * \brief An attached (mounted) virtual filesystem device from gio
 */
DAttachedVfsDevice::DAttachedVfsDevice(const QString &mountpointPath)
{
    m_dgioMount.reset(DGioMount::createFromPath(mountpointPath));
    m_mountpointPath = mountpointPath;
}

bool DAttachedVfsDevice::isValid()
{
    return !m_dgioMount.isNull();
}

bool DAttachedVfsDevice::detachable()
{
    return m_dgioMount->canUnmount();
}

void DAttachedVfsDevice::detach()
{
    //使用dgioMount->unmount()不能提供失败警告，需要替换为一个有状态返回的卸载方式。 BUG 51596
    //m_dgioMount->unmount();
    unmount_mounted(m_mountpointPath);
}

QString DAttachedVfsDevice::displayName()
{
    return m_dgioMount ? m_dgioMount->name() : QStringLiteral("-");
}

bool DAttachedVfsDevice::deviceUsageValid()
{
    if (m_dgioMount.isNull())
        return false;

    QExplicitlySharedDataPointer<DGioFile> file = m_dgioMount->getRootFile();
    if (file) {
        QExplicitlySharedDataPointer<DGioFileInfo> fi = file->createFileInfo("*", FILE_QUERY_INFO_NONE, 500);
        // 直接取createFileInfo数据有误，createFileSystemInfo数据才准确, createFileInfo仅用来判断文件是否可访问
        // 这里逻辑与计算机页面保持一致
        if (fi && fi->fileType() == DGioFileType::FILE_TYPE_DIRECTORY) {
            return file->createFileSystemInfo();
        }
    }

    return false;
}

QPair<quint64, quint64> DAttachedVfsDevice::deviceUsage()
{
    QExplicitlySharedDataPointer<DGioFile> file = m_dgioMount->getRootFile();
    if (file) {
        QExplicitlySharedDataPointer<DGioFileInfo> fi = file->createFileInfo("*", FILE_QUERY_INFO_NONE, 500);
        // 直接取createFileInfo数据有误，createFileSystemInfo数据才准确, createFileInfo仅用来判断文件是否可访问
        // 这里逻辑与计算机页面保持一致
        if (fi && fi->fileType() == DGioFileType::FILE_TYPE_DIRECTORY) {
            QExplicitlySharedDataPointer<DGioFileInfo> fsInfo = file->createFileSystemInfo();
            if (fsInfo)
                return QPair<quint64, quint64>(fsInfo->fsFreeBytes(), fsInfo->fsTotalBytes());
        }
    }

    return QPair<quint64, quint64>(0, 0);
}

QString DAttachedVfsDevice::iconName()
{
    QStringList iconList = m_dgioMount ? m_dgioMount->themedIconNames() : QStringList();

    if (iconList.empty()) {
        return QStringLiteral("drive-network");
    }

    return iconList.first();
}

QUrl DAttachedVfsDevice::mountpointUrl()
{
    QExplicitlySharedDataPointer<DGioFile> file = m_dgioMount->getRootFile();
    return QUrl::fromLocalFile(file->path());
}

QUrl DAttachedVfsDevice::accessPointUrl()
{
    return mountpointUrl();
}
