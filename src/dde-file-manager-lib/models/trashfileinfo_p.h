// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEINFO_P_H
#define TRASHFILEINFO_P_H

#include "private/dabstractfileinfo_p.h"
#include "trashfileinfo.h"
#

class TrashFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    TrashFileInfoPrivate(const DUrl &url, TrashFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true)
    {
        columnCompact = false;
    }

    QString desktopIconName;
    QString displayName;
    QString originalFilePath;
    QString displayDeletionDate;
    QDateTime deletionDate;
    QStringList tagNameList;

    void updateInfo();
    void inheritParentTrashInfo();
};


#endif // TRASHFILEINFO_P_H
