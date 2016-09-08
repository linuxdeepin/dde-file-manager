#include "thumbnailmanager.h"
#include "standardpath.h"
#include "fileutils.h"

#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <QImageReader>
#include <QCryptographicHash>
#include <QFileSystemWatcher>
#include "app/global.h"
#include "widgets/singleton.h"
#include "controllers/fileservices.h"
#include "./mimetypedisplaymanager.h"

ThumbnailManager::ThumbnailManager(QObject *parent)
    : QThread(parent)
    , watcher(new QFileSystemWatcher(this))
{
    connect(watcher, &QFileSystemWatcher::fileChanged, this, [this] (const QString &filePath) {
        const QString &md5 = m_pathToMd5.take(filePath);

        if (!md5.isEmpty()) {
            emit iconChanged(filePath, QIcon());
        }
    });
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
    int pos = fpath.lastIndexOf('/');

    if (pos > 0 && getThumbnailPath("") == fpath.left(pos + 1)) {
        if (!m_pathToMd5.contains(fpath)) {
            const QString &md5 = fpath.mid(pos + 1);

            m_pathToMd5[fpath] = md5;

            if (!m_md5ToIcon.contains(md5)) {
                const QIcon &icon = QIcon(fpath);

                m_md5ToIcon[md5] = icon;

                return icon;
            }

            return m_md5ToIcon.value(md5);
        }
    }

    return m_md5ToIcon.value(m_pathToMd5.value(fpath));
}

void ThumbnailManager::requestThumbnailIcon(const QString &fpath)
{
    if (m_pathToMd5.contains(fpath))
        return;

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
        QImageReader reader(&file);
        AbstractFileInfoPointer fileInfo = fileService->createFileInfo(DUrl::fromUserInput(fpath));

        /// ensure image size < 30MB
        if (file.size() > 1024 * 1024 * 30&&reader.canRead()) {
            m_pathToMd5[fpath] = QString();

            continue;
        }

        watcher->addPath(fpath);

        const QString &md5 = FileUtils::md5(&file, fpath);;

        m_pathToMd5[fpath] = md5;

        QIcon icon = m_md5ToIcon.value(md5);

        if (!icon.isNull()) {
            emit iconChanged(fpath, icon);

            continue;
        };

        QString thumbnailPath = QString("%1").arg(getThumbnailPath(md5));

        if (QFile(thumbnailPath).exists()) {
            icon = QIcon(thumbnailPath);

            m_md5ToIcon[md5] = icon;
        } else {
            if (reader.canRead()) {
                QSize size = reader.size();

                bool canScale = size.width() > 256 || size.height() > 256;

                if (canScale) {
                    size.scale(QSize(qMin(256, size.width()), qMin(256, size.height())), Qt::KeepAspectRatio);
                    reader.setScaledSize(size);
                }

                const QByteArray &format = FileUtils::imageFormatName(reader.imageFormat());
                const QPixmap &pixmap = QPixmap::fromImageReader(&reader);

                icon = QIcon(pixmap);

                m_md5ToIcon[md5] = icon;

                if (canScale) {
                    pixmap.save(thumbnailPath, format, 20);
                } else {
                    QFile::link(fpath, thumbnailPath);
                }
            }
            else if(fileInfo->mimeTypeName() == "text/plain"){
                const QPixmap &pixmap = FilePreviewIconProvider::getPlainTextPreviewIcon(fpath);
                icon = QIcon(pixmap);
                m_md5ToIcon[md5] = icon;
                pixmap.save(thumbnailPath,"png",20);
            }
            else if(fileInfo->mimeTypeName() == "application/pdf"){
                const QPixmap &pixmap = FilePreviewIconProvider::getPDFPreviewIcon(fpath);
                icon = QIcon(pixmap);
                m_md5ToIcon[md5] = icon;
                pixmap.save(thumbnailPath,"png",20);
            }
            else if(mimeTypeDisplayManager->defaultIcon(fileInfo->mimeTypeName()) == "video"){
                const QPixmap &pixmap = FilePreviewIconProvider::getVideoPreviewIcon(fpath);
                icon = QIcon(pixmap);
                m_md5ToIcon[md5] = icon;
                pixmap.save(thumbnailPath,"png",20);
            }
        }

        emit iconChanged(fpath, icon);
    }
}
