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
