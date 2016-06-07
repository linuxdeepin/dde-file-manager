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

QIcon ThumbnailManager::thumbnailIcon(const QString &fpath)
{
    QIcon icon = m_thumbnailsIcon.value(fpath);

    if (!icon.isNull())
        return icon;

    QFileInfo info(fpath);

    if (info.size() > 1024 * 256) {
        QString md5 = FileUtils::md5(fpath);
        QString thumbnailPath = QString("%1.png").arg(getThumbnailPath(md5));

        if (QFile(thumbnailPath).exists()) {
            icon = QIcon(thumbnailPath);

            m_thumbnailsIcon[fpath] = icon;
        } else {
            addThumbnailTask(fpath, thumbnailPath);
        }
    } else {
        icon = QIcon(fpath);

        m_thumbnailsIcon[fpath] = icon;
    }

    return icon;
}

void ThumbnailManager::addThumbnailTask(const QString &fpath, const QString &thumbnailPath)
{
    QtConcurrent::run(m_threadPool, this, &ThumbnailManager::actionThumbnailTask, fpath, thumbnailPath);
}

void ThumbnailManager::actionThumbnailTask(const QString &fpath, const QString &thumbnailPath)
{
    if (m_thumbnailsIcon.contains(fpath)){
        return;
    }

    m_thumbnailsIcon[fpath] = QIcon();

    QFileInfo info(fpath);

    if (info.exists()){
        QPixmap pixmap(fpath);
        QPixmap thumbnail = pixmap.scaled(256, 256, Qt::KeepAspectRatio);
        thumbnail.save(thumbnailPath);
        m_thumbnailsIcon[fpath] = QIcon(thumbnail);
    }
}
