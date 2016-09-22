#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QThread>
#include <QQueue>
#include <QMap>
#include <QPixmap>
#include "thumbnailgenerator.h"
#include <QCache>
#include <QUrl>

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
};

class ThumbnailManager : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = 0);
    void init();
    void initConnections();

    QString getThumbnailCachePath() const;
    QString getThumbnailPath(const QString& name, ThumbnailGenerator::ThumbnailSize size) const;
    QString getThumbnailFailPath(const QString& name) const;

    QPixmap getThumbnailPixmap(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size);
    void requestThumbnailPixmap(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size, const int& quality);
    QPixmap getDefaultPixmap(const QUrl& fileUrl);
    bool canGenerateThumbnail(const QUrl& fileUrl);

    QString toMd5(const QString data);

signals:
    void pixmapChanged(const QString &filePath, const QPixmap &pixmap);
public slots:
    void onFileChanged(const QString &path);

protected:
    void run () Q_DECL_OVERRIDE;

private:
    QString m_cachePath;
    QString m_thumbnailNormalPath;
    QString m_thumbnailLargePath;
    QString m_thumbnailFailPath;
    QString m_thumbnailPath;

    QQueue<ThumbnailTask> taskQueue;
    QMap<QString, QString> m_pathToMd5;
    QMap<QString, QPixmap> m_md5ToPixmap;

    QFileSystemWatcher *m_watcher = Q_NULLPTR;
    ThumbnailGenerator m_thumbnailGenerator;
};

#endif // THUMBNAILMANAGER_H
