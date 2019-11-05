/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#include "fsutils.h"

#include <ddiskmanager.h>

int FsUtils::maxLabelLength(const QString &fs)
{
    if (fs == "vfat") {
        // man 8 mkfs.fat
        return 11;
    }
    if (fs.startsWith("ext")) {
        // man 8 mke2fs
        return 16;
    }
    if (fs == "btrfs") {
        // https://btrfs.wiki.kernel.org/index.php/Manpage/btrfs-filesystem
        return 255;
    }
    if (fs == "f2fs") {
        // https://www.kernel.org/doc/Documentation/filesystems/f2fs.txt
        // https://git.kernel.org/pub/scm/linux/kernel/git/jaegeuk/f2fs-tools.git/tree/mkfs/f2fs_format_main.c
        return 512;
    }
    if (fs == "jfs") {
        // jfsutils/mkfs/mkfs.c:730
        return 16;
    }
    if (fs == "exfat") {
        // man 8 mkexfatfs
        return 15;
    }
    if (fs == "nilfs2") {
        // man 8 mkfs.nilfs2
        return 80;
    }
    if (fs == "ntfs") {
        // https://docs.microsoft.com/en-us/dotnet/api/system.io.driveinfo.volumelabel?view=netframework-4.8
        return 32;
    }
    if (fs == "reiserfs") {
        // man 8 mkreiserfs
        return 16;
    }
    if (fs == "reiser4") {
        // https://github.com/edward6/reiser4progs/blob/master/include/reiser4/types.h fs_hint_t
        return 16;
    }
    if (fs == "xfs") {
        // man 8 mkfs.xfs
        return 12;
    }
    return -1;
}

QStringList FsUtils::supportedFilesystems()
{
    QStringList ret;
    DDiskManager diskmgr;
    for (auto &fs : diskmgr.supportedFilesystems()) {
        if (diskmgr.canFormat(fs)) {
            ret.push_back(fs);
        }
    }
    return ret;
}
