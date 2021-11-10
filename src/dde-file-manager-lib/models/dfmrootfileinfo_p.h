/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
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

#ifndef DFMROOTFILEINFO_P_H
#define DFMROOTFILEINFO_P_H
#include "dfmrootfileinfo.h"
#include <QStandardPaths>
#include <dblockdevice.h>
#include <dgiofileinfo.h>
#include <dgiomount.h>

class DFMRootFileInfoPrivate
{
public:
    QStandardPaths::StandardLocation stdloc;
    QSharedPointer<DBlockDevice> blk;
    QSharedPointer<DBlockDevice> ctblk;
    QExplicitlySharedDataPointer<DGioMount> gmnt;
    QExplicitlySharedDataPointer<DGioFileInfo> gfsi;
    QString backer_url;
    QByteArrayList mps; /* mountpoints */
    qulonglong size;
    QString label;
    QString fs;
    QString fsVersion;
    QString udispname;
    QString idUUID;
    QString currentUUID;
    QString backupUUID;
    bool isod;
    bool encrypted;

private:
    DFMRootFileInfo *q_ptr;
    Q_DECLARE_PUBLIC(DFMRootFileInfo)
};


#endif // DFMROOTFILEINFO_P_H
