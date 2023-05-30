// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QMimeDatabase>

namespace dfmbase {

class DMimeDatabase : public QMimeDatabase
{
    Q_DISABLE_COPY(DMimeDatabase)

public:
    DMimeDatabase();

    QMimeType mimeTypeForFile(const QUrl &url, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const FileInfoPointer &fileInfo, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode, const QString &inod, const bool isGvfs = false) const;
    QMimeType mimeTypeForUrl(const QUrl &url) const;

private:
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode, const QString &inod, const bool isGvfs = false) const;

private:
    QHash<QString, QMimeType> inodMimetypeCache;
};

}

#endif   // DMIMEDATABASE_H
