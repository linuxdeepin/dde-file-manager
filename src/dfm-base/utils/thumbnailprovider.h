// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/schemefactory.h>

#include <QObject>

#include <functional>

class QMimeType;

namespace dfmbase {

class ThumbnailProviderPrivate;

class ThumbnailProvider : public QObject
{
    Q_OBJECT
public:
    enum Size : uint16_t {
        kSmall = 64,
        kNormal = 128,
        kLarge = 256,
    };

    static ThumbnailProvider *instance()
    {
        static ThumbnailProvider thumb;
        return &thumb;
    }

    bool hasThumbnail(const QUrl &url) const;
    bool hasThumbnail(const QMimeType &mimeType) const;
    int hasThumbnailFast(const QString &mimeType) const;
    bool thumbnailEnable(const QUrl &url) const;

    QPixmap thumbnailPixmap(const QUrl &fileUrl, Size size) const;

    QString createThumbnail(FileInfoPointer fileInfo, Size size);

    QString errorString() const;
    qint64 sizeLimit(const QMimeType &mimeType) const;

private:
    void createAudioThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image);
    bool createImageVDjvuThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image, const QString &thumbnailName, QString &thumbnail);
    void createImageThumbnail(const QUrl &url, const QMimeType &mime, const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image);
    void createTextThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image);
    void createPdfThumbnail(const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image);
    bool createDefaultThumbnail(const QMimeType &mime, const QString &filePath, ThumbnailProvider::Size size, QScopedPointer<QImage> &image, QString &thumbnail);
    bool createThumnailByMovieLib(const QString &filePath, QScopedPointer<QImage> &image);
    void initThumnailTool();
    bool createThumnailByDtkTools(const QMimeType &mime, ThumbnailProvider::Size size, const QString &filePath, QScopedPointer<QImage> &image);
    bool createThumnailByTools(const QMimeType &mime, ThumbnailProvider::Size size, const QString &filePath, QScopedPointer<QImage> &image);
    bool isIconCachePath(const QString &dirPath) const;

protected:
    explicit ThumbnailProvider(QObject *parent = nullptr);
    ~ThumbnailProvider() override;

private:
    QScopedPointer<ThumbnailProviderPrivate> d;
};

}

Q_DECLARE_METATYPE(dfmbase::ThumbnailProvider::Size)

#endif   // THUMBNAILPROVIDER_H
