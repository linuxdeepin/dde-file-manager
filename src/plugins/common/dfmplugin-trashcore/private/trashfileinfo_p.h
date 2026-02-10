// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEINFO_P_H
#define TRASHFILEINFO_P_H

#include "dfmplugin_trashcore_global.h"
#include "trashfileinfo.h"

#include <dfm-io/dfileinfo.h>
USING_IO_NAMESPACE
namespace dfmplugin_trashcore {

class TrashFileInfoPrivate
{
public:
    explicit TrashFileInfoPrivate(TrashFileInfo *qq)
        : q(qq)
    {
    }

    virtual ~TrashFileInfoPrivate();

    QUrl initTarget();
    QString fileName() const;
    QString copyName() const;
    QString mimeTypeName();
    QDateTime lastRead() const;
    QDateTime lastModified() const;
    QDateTime deletionTime() const;
    QString symLinkTarget() const;

    QSharedPointer<DFileInfo> dFileInfo { nullptr };
    QSharedPointer<DFileInfo> dAncestorsFileInfo { nullptr };
    QUrl targetUrl;
    QUrl originalUrl;
    TrashFileInfo *const q;
};

}

#endif   // TRASHFILEINFO_P_H
