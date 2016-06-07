#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QThreadPool>

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = 0);
    ~ThumbnailManager();

    static QString getThumbnailCachePath();
    static QString getThumbnailPath(const QString& name);

    QIcon thumbnailIcon(const QString &fpath);

public slots:
    void addThumbnailTask(const QString& fpath, const QString &thumbnailPath);
    void actionThumbnailTask(const QString& fpath, const QString &thumbnailPath);

private:
    QMap<QString, QIcon> m_thumbnailsIcon;
    QThreadPool* m_threadPool;
};

#endif // THUMBNAILMANAGER_H
