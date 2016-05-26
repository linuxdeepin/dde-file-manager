#include "thumbnailmanager.h"
#include "standardpath.h"
#include "fileutils.h"
#include <QDir>
#include <QtConcurrent/QtConcurrentRun>

ThumbnailManager::ThumbnailManager(QObject *parent) : QObject(parent)
{
    m_threadPool = new QThreadPool(this);
}

ThumbnailManager::~ThumbnailManager()
{

}

QString ThumbnailManager::getThumbnailCachePath()
{
    QDir dir(StandardPath::getCachePath());
    dir.mkpath("thumbnails");
    return QString("%1/%2").arg(StandardPath::getCachePath(), "thumbnails");
}

QString ThumbnailManager::getThumbnailPath(const QString &name)
{
    return QString("%1/%2").arg(getThumbnailCachePath(), name);
}

QString ThumbnailManager::thumbnail(const QString &fpath)
{
    return m_thumbnails.value(fpath, "");
}

void ThumbnailManager::addThumbnailTask(const QString &fpath)
{
    QtConcurrent::run(m_threadPool, this, &ThumbnailManager::actionThumbnailTask, fpath);
}

void ThumbnailManager::actionThumbnailTask(const QString &fpath)
{
    if (m_thumbnails.contains(fpath)){
        return;
    }
    QFileInfo info(fpath);
    if (info.exists()){
        QString md5 = FileUtils::md5(fpath);
        QString thumbnailPath = QString("%1.png").arg(getThumbnailPath(md5));
        if (m_taskSet.contains(md5)){
            return;
        }
        if (!QFile(thumbnailPath).exists()){
            qDebug() << thumbnailPath;
            QPixmap pixmap(fpath);
            QPixmap thumbnail = pixmap.scaled(256, 256, Qt::KeepAspectRatio);
            thumbnail.save(thumbnailPath);
            m_taskSet.insert(thumbnailPath);
        }
        m_thumbnails.insert(fpath, thumbnailPath);
    }
}
