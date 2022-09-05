// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
