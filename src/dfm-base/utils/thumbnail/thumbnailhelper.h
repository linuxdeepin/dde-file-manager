// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILHELPER_H
#define THUMBNAILHELPER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-base/mimetype/dmimedatabase.h>

#include <QUrl>
#include <QMimeType>

namespace dfmbase {

class ThumbnailHelper
{
public:
    static ThumbnailHelper *instance();

    bool canGenerateThumbnail(const QUrl &url);
    bool checkThumbEnable(const QUrl &url);
    QString saveThumbnail(const QUrl &url, const QImage &img, DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    QImage thumbnailImage(const QUrl &fileUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize size) const;

    void setSizeLimit(const QMimeType &mime, qint64 size);
    qint64 sizeLimit(const QMimeType &mime);

    static const QStringList &defaultThumbnailDirs();
    static QString sizeToFilePath(DFMGLOBAL_NAMESPACE::ThumbnailSize size);
    static QByteArray dataToMd5Hex(const QByteArray &data);

private:
    explicit ThumbnailHelper();
    bool checkMimeTypeSupport(const QMimeType &mime);
    void makePath(const QString &path);

private:
    DMimeDatabase mimeDatabase;
    QHash<QMimeType, qint64> sizeLimitHash;
};
}   // namespace dfmbase

#endif   // THUMBNAILHELPER_H
