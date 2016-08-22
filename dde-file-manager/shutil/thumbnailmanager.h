#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QThread>
#include <QQueue>
#include <QMap>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
QT_END_NAMESPACE

class ThumbnailManager : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = 0);

    static QString getThumbnailCachePath();
    static QString getThumbnailPath(const QString& name);

    QIcon getThumbnailIcon(const QString &fpath);

    void requestThumbnailIcon(const QString &fpath);

signals:
    void iconChanged(const QString &filePath, const QIcon &icon);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QQueue<QString> taskQueue;
    QMap<QString, QString> m_pathToMd5;
    QMap<QString, QIcon> m_md5ToIcon;

    QFileSystemWatcher *watcher = Q_NULLPTR;
};

#endif // THUMBNAILMANAGER_H
