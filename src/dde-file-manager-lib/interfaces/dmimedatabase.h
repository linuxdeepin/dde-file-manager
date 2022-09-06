// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include <QMimeDatabase>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DMimeDatabase : public QMimeDatabase
{
    Q_DISABLE_COPY(DMimeDatabase)

public:
    DMimeDatabase();

    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode, const QString& inod,const bool isgvfs = false) const;
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode, const QString& inod,const bool isgvfs = false) const;
    QMimeType mimeTypeForUrl(const QUrl &url) const;

private:
    QHash<QString, QMimeType> inodmimetypecache;
};

DFM_END_NAMESPACE

#endif // DMIMEDATABASE_H
