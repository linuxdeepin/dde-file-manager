#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QThread>
#include <QQueue>
#include <QMap>
#include <QPixmap>
#include "thumbnailgenerator.h"
#include <QCache>
#include <QUrl>
#include <QTimer>
#include <QWaitCondition>
#include <QReadWriteLock>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
QT_END_NAMESPACE

struct ThumbnailTask{
    QUrl fileUrl;
    ThumbnailGenerator::ThumbnailSize size;
    int quality = -1;
    ThumbnailTask(const QUrl& url, const ThumbnailGenerator::ThumbnailSize& size, const int& quality):
        fileUrl(url){
        this->size = size;
        this->quality = quality;
    }

    bool operator ==(const ThumbnailTask &other) {
        return other.fileUrl == fileUrl && other.size == size && other.quality == quality;
    }
};

class ThumbnailManager : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = 0);
    void init();

    QString getThumbnailCachePath() const;
    QString getThumbnailPath(const QString& name, ThumbnailGenerator::ThumbnailSize size) const;
    QString getThumbnailFailPath(const QString& name) const;

    QPixmap getThumbnailPixmap(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size);
    void requestThumbnailPixmap(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size, const int& quality);
    QPixmap getDefaultPixmap(const QUrl& fileUrl);
    bool canGenerateThumbnail(const QUrl& fileUrl);

    void abortGetThumbnailPixmap(const QUrl &fileUrl, ThumbnailGenerator::ThumbnailSize size, const int& quality);

    QString toMd5(const QString data);

signals:
    void pixmapChanged(const QString &filePath, const QPixmap &pixmap);

protected:
    void run () Q_DECL_OVERRIDE;

private:
    QString m_cachePath;
    QString m_thumbnailNormalPath;
    QString m_thumbnailLargePath;
    QString m_thumbnailFailPath;
    QString m_thumbnailPath;
    int m_maxTaskCacheNum;

    QQueue<ThumbnailTask> taskQueue;
    QQueue<ThumbnailTask> taskCache;
    QMap<QUrl, QPixmap> m_urlToPixmap;

    ThumbnailGenerator m_thumbnailGenerator;

    QWaitCondition waitCondition;
    QReadWriteLock readWriteLockTaskQueue;
};

#endif // THUMBNAILMANAGER_H
