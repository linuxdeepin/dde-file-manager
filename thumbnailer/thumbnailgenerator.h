#ifndef THUMBNAILGENERATOR_H
#define THUMBNAILGENERATOR_H

#include <QObject>
#include <QPixmap>
#include <QUrl>

#define THUMBNAIL_SIZE_NORMAL 128
#define THUMBNAIL_SIZE_LARGE 256
#define DROPSHADOW_RADIUS 15
#define IMAGE_BORDER 8

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

    bool isTextPlainFile(const QString &fileName) const;
    bool isPDFFile(const QString &fileName) const;
    bool isVideoFile(const QString &fileName) const;
    bool isPictureFile(const QString &fileName) const;
    QMap<QString,QString> getAttributeSet(const QUrl&  fileUrl);

signals:

public slots:

private:
    QPixmap getTextplainThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getPDFThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getVideoThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getPictureThumbnail(const QString &fpath, const ThumbnailSize &size = THUMBNAIL_NORMAL);

};

#endif // THUMBNAILGENERATOR_H
