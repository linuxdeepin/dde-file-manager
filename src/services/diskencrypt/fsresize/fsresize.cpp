// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fsresize.h"

#include <sys/stat.h>

using namespace daemonplugin_file_encrypt;

bool fs_resize::shrinkFileSystem_ext(const QString &device)
{
    // TODO(xust): not find the API of resize2fs, use BIN program temp
    QString cmd = QString("e2fsck -f -y %1").arg(device);
    int ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "e2fsck failed!"
                   << ret;
        return false;
    }

    cmd = QString("resize2fs -M %1").arg(device);
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "resize2fs failed"
                   << ret;
        return false;
    }

    return true;
}

bool fs_resize::expandFileSystem_ext(const QString &device)
{
    // TODO(xust): not find the API of resize2fs, use BIN program temp
    QString cmd = QString("e2fsck -f -y %1").arg(device);
    int ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "e2fsck failed!"
                   << ret;
        return false;
    }

    cmd = QString("resize2fs %1").arg(device);
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "resize2fs failed"
                   << ret;
        return false;
    }

    return true;
}

bool fs_resize::recoverySuperblock_ext(const QString &device,
                                       const QString &cryptHeaderPath)
{
    struct stat st;
    int ret = stat(cryptHeaderPath.toStdString().c_str(),
                   &st);
    if (ret != 0) {
        qWarning() << "cannot stat header file"
                   << device
                   << cryptHeaderPath;
        return false;
    }

    QString cmd = QString("e2image -aro %1 %2 %2")
                          .arg(st.st_size)
                          .arg(device);
    ret = ::system(cmd.toStdString().c_str());
    if (ret != 0) {
        qWarning() << "cannot recovery superblock of"
                   << device;
        return false;
    }
    return true;
}
