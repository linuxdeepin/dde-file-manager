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

    QString thumbnail(const QString& fpath);

signals:

public slots:
    void addThumbnailTask(const QString& fpath);
    void actionThumbnailTask(const QString& fpath);
private:
    QMap<QString, QString> m_thumbnails;
    QSet<QString> m_taskSet;
    QThreadPool* m_threadPool;
};

#endif // THUMBNAILMANAGER_H
