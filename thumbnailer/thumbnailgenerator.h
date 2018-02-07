/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef THUMBNAILGENERATOR_H
#define THUMBNAILGENERATOR_H

#include <QObject>
#include <QPixmap>
#include <QUrl>
#include "shutil/dmimedatabase.h"

#define THUMBNAIL_SIZE_NORMAL 128
#define THUMBNAIL_SIZE_LARGE 256

class ThumbnailGenerator : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailGenerator(QObject *parent = 0);

    enum ThumbnailSize{
        THUMBNAIL_NORMAL = THUMBNAIL_SIZE_NORMAL,
        THUMBNAIL_LARGE = THUMBNAIL_SIZE_LARGE
    };

    QPixmap generateThumbnail(const QUrl& fileUrl , ThumbnailSize size);
    bool canGenerateThumbnail(const QUrl&  fileUrl)const;

    inline static bool isTextPlainFile(const QString &fileName)
    { return DMimeDatabase().mimeTypeForFile(fileName).name() == "text/plain";}
    inline static bool isPDFFile(const QString &fileName)
    { return DMimeDatabase().mimeTypeForFile(fileName).name() == "application/pdf";}
    static bool isVideoFile(const QString &fileName);
    static bool isPictureFile(const QString &fileName);
    static QMap<QString,QString> getAttributeSet(const QUrl&  fileUrl);

signals:

public slots:

private:
    QPixmap getTextplainThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getPDFThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getVideoThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getPictureThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);

};

#endif // THUMBNAILGENERATOR_H
