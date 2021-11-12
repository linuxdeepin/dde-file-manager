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

#pragma once
#include <gio/gio.h>
#include <dgiomount.h>

#include <QFileInfo>
#include <dgiofileinfo.h>
#include <dgiofile.h>
#include <QVariantMap>
#include "ut_mock_stub_common_define.h"

DGioMount * createFromPath_gioMount_stub(QString path, QObject *parent = nullptr);

GMount *   g_file_find_enclosing_mount_stub       (GFile *file,GCancellable *cancellable,GError **error);

void       g_mount_unmount_with_operation_stub    (GMount              *mount,
                                               GMountUnmountFlags   flags,
                                               GMountOperation     *mount_operation,
                                               GCancellable        *cancellable,
                                               GAsyncReadyCallback  callback,
                                               gpointer             user_data);


bool canUnmount_stub();

bool exists_qfileinfo_stub();

bool exists_qfileinfo_param_stub(const QString &file);

bool permission_stub(QFile::Permissions permissions);

QString findExecutable_QStandardPaths_return_empty_stub(const QString &executableName, const QStringList &paths = QStringList());

QString name_stub();

QByteArrayList gio_mountPoints_more_stub();

const QList<QExplicitlySharedDataPointer<DGioMount> > getMounts_stub();

const QList<QExplicitlySharedDataPointer<DGioMount> > get_gvfs_Mounts_stub();

QExplicitlySharedDataPointer<DGioFile> get_gvfs_RootFile_stub();

QExplicitlySharedDataPointer<DGioFile> getRootFile_stub();
DGioFileType getFileType_stub();

QExplicitlySharedDataPointer<DGioFileInfo> createFileInfo_stub(QString attr = "*", DGioFileQueryInfoFlags queryInfoFlags = FILE_QUERY_INFO_NONE, unsigned long timeout_msec = ULONG_MAX);
QExplicitlySharedDataPointer<DGioFileInfo> createFileSystemInfo_stub(QString attr = "*");

QExplicitlySharedDataPointer<DGioFileInfo> createFileSystemInfo_return_null_stub(QString attr = "*", DGioFileQueryInfoFlags queryInfoFlags = FILE_QUERY_INFO_NONE, unsigned long timeout_msec = ULONG_MAX);

quint64 fsTotalBytes_2KB_stub();

quint64 fsUsedBytes_1KB_stub();

quint64 fsFreeBytes_1KB_stub();

QStringList themedIconNames_stub();

QStringList themedIconNames_empty_stub();

QString dgio_devpath_stub();

QStringList blockDevices_DDiskManager_stub(QVariantMap options);

bool hasFileSystem_stub();

bool hintIgnore_stub();

bool hintSystem_stub();

QString scheme_burn_stub();

bool isShadowed_stub();

void detach_stub();
