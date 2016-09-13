#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QThread>
#include <QQueue>
#include <QMap>
#include <QIcon>
#include "thumbnailgenerator.h"
#include <QCache>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
QT_END_NAMESPACE

class ThumbnailManager : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = 0);
    void init();
    void initConnections();

    QString getThumbnailCachePath();
    QString getThumbnailPath(const QString& name, ThumbnailGenerator::ThumbnailSize size);

    QIcon getThumbnailIcon(const QUrl& fileUrl);
    void requestThumbnailIcon(const QUrl& fileUrl);

    bool canGenerateThumbnail(const QUrl& fileUrl);

    QString toMd5(const QString data);

signals:
    void iconChanged(const QString &filePath, const QIcon &icon);
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

    QQueue<QString> taskQueue;
    QMap<QString, QString> m_pathToMd5;
    QMap<QString, QIcon> m_md5ToIcon;

    QFileSystemWatcher *m_watcher = Q_NULLPTR;
    ThumbnailGenerator m_thumbnailGenerator;
};

#endif // THUMBNAILMANAGER_H
