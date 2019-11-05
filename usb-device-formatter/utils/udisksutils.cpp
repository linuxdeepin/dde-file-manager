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

#include "udisksutils.h"

#include <QCoreApplication>
#include <QLocale>
#include <QStorageInfo>

#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <ddiskmanager.h>

UDisksBlock::UDisksBlock(const QString &devnode)
{
    QString udiskspath(devnode);
    udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    blk.reset(DDiskManager::createBlockDevice(udiskspath));
}

UDisksBlock::~UDisksBlock()
{
}

bool UDisksBlock::isReadOnly() const
{
    return blk->readOnly();
}

/*
 * To whomever modifying this class in the future:
 * Consult the DFMFileInfo class in dde-file-manager first! Make sure
 * reasonable parity is maintained between these two classes.
 */
QString UDisksBlock::displayName() const
{
    if (blk->mountPoints().contains(QByteArray("/\0", 2))) {
        return QCoreApplication::tr("System Disk");
    }
    if (blk->idLabel().length() == 0) {
        if (blk->isEncrypted() && blk->cleartextDevice().length() <= 1) {
            return QCoreApplication::tr("%1 Encrypted").arg(QLocale::system().formattedDataSize(sizeTotal()));
        }
        return QCoreApplication::tr("%1 Volume").arg(QLocale::system().formattedDataSize(sizeTotal()));
    } else {
        return blk->idLabel();
    }
}

QString UDisksBlock::iconName() const
{
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
    if (drv->media() == "thumb" || drv->removable() || drv->mediaRemovable() || drv->ejectable()) {
        return QString("drive-removable-media") + (blk->isEncrypted() ? "-encrypted" : "");
    }
    return QString("drive-harddisk") + (blk->isEncrypted() ? "-encrypted" : "");
}

QString UDisksBlock::fsType() const
{
    return blk->idType();
}

qint64 UDisksBlock::sizeTotal() const
{
    return static_cast<qint64>(blk->size());
}

qint64 UDisksBlock::sizeUsed() const
{
    QScopedPointer<DBlockDevice> rblk(DDiskManager::createBlockDevice(blk->path()));
    if (rblk->isEncrypted() && rblk->cleartextDevice().length() > 1) {
        rblk.reset(DDiskManager::createBlockDevice(blk->cleartextDevice()));
    }
    if (!rblk->hasFileSystem()) {
        return -1;
    }

    QString mp;
    if (rblk->mountPoints().empty()) {
        mp = rblk->mount({{"auth.no_user_interaction", true}});
        if (rblk->lastError().isValid()) {
            mp.clear();
        }
    } else {
        mp = rblk->mountPoints().front();
    }

    if (!mp.length()) {
        return -1;
    }

    QStorageInfo si(mp);
    return si.bytesTotal() - si.bytesFree();
}

DBlockDevice *UDisksBlock::operator ->()
{
    return blk.data();
}
