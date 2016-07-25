#include "thumbnailmanager.h"
#include "standardpath.h"
#include "fileutils.h"

#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <QImageReader>
#include <QCryptographicHash>
ThumbnailManager::ThumbnailManager(QObject *parent)
    : QThread(parent)
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

QIcon ThumbnailManager::getThumbnailIcon(const QString &fpath)
{
    const QString &md5 = m_pathToMd5.value(fpath);

    if (md5.isEmpty())
        return QIcon();

    return m_md5ToIcon.value(md5);
}

void ThumbnailManager::requestThumbnailIcon(const QString &fpath)
{
    if (taskQueue.contains(fpath))
        return;

    taskQueue << fpath;

    if (!isRunning())
        start();
}

void ThumbnailManager::run()
{
    while (!taskQueue.isEmpty()) {
        const QString &fpath = taskQueue.dequeue();

        QFile file(fpath);

        /// ensure image size < 100MB
        if (file.size() > 1024 * 1024 * 100)
            continue;

        QString md5;

        if (file.open(QIODevice::ReadOnly)) {
            md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        } else {
            md5 = fpath;
        }

        m_pathToMd5[fpath] = md5;

        QIcon icon = m_md5ToIcon.value(md5);

        if (!icon.isNull()) {
            emit taskFinished(fpath, icon);

            continue;
        }

        QString thumbnailPath = QString("%1.jpg").arg(getThumbnailPath(md5));

        if (QFile(thumbnailPath).exists()) {
            icon = QIcon(thumbnailPath);

            m_md5ToIcon[md5] = icon;
        } else {
            QImageReader reader(fpath);

            if (reader.canRead()) {
                QSize size = reader.size();

                bool canScale = size.width() > 256 || size.height() > 256;

                if (canScale) {
                    size.scale(QSize(qMin(256, size.width()), qMin(256, size.height())), Qt::KeepAspectRatio);
                    reader.setScaledSize(size);
                }

                const QPixmap &pixmap = QPixmap::fromImageReader(&reader);

                icon = QIcon(pixmap);

                m_md5ToIcon[md5] = icon;

                if (canScale) {
                    pixmap.save(thumbnailPath, "jpg", 20);
                } else {
                    QFile::link(fpath, thumbnailPath);
                }
            }
        }

        emit taskFinished(fpath, icon);
    }
}
