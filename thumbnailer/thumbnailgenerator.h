#ifndef THUMBNAILGENERATOR_H
#define THUMBNAILGENERATOR_H

#include <QObject>
#include <QPixmap>

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

    QPixmap generateThumbnail(const QString fileUrl , ThumbnailSize size);
    bool canGenerateThumbnail(const QString fileUrl)const;

    bool isTextPlainFile(const QString &fileName) const;
    bool isPDFFile(const QString &fileName) const;
    bool isVideoFile(const QString &fileName) const;
    bool isPictureFile(const QString &fileName) const;

signals:

public slots:

private:
    QPixmap getTextplainThumbnail(const QString &fileUrl, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getPDFThumbnail(const QString &fileUrl, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getVideoThumbnail(const QString &fileUrl, const ThumbnailSize &size = THUMBNAIL_NORMAL);
    QPixmap getPictureThumbnail(const QString &fileUrl, const ThumbnailSize &size = THUMBNAIL_NORMAL);

};

#endif // THUMBNAILGENERATOR_H
