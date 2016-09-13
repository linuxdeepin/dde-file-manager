#include "thumbnailmanager.h"

#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <QImageReader>
#include <QCryptographicHash>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QImageWriter>
#include <QDateTime>
#include <QUrl>

#include "app/global.h"
#include "widgets/singleton.h"
#include "controllers/fileservices.h"

ThumbnailManager::ThumbnailManager(QObject *parent)
    : QThread(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    init();
    initConnections();
}

void ThumbnailManager::init()
{
    m_cachePath = getThumbnailCachePath();
    QDir thumbnails(m_cachePath+"/thumbnails");

    if(!thumbnails.exists())
        thumbnails.mkdir(thumbnails.path());

    QDir normal(thumbnails.path() + "/normal");
    if(!normal.exists())
        normal.mkdir(normal.path());

    QDir large(thumbnails.path() + "/large");
    if(!large.exists())
        large.mkdir(large.path());

    QDir fail(thumbnails.path() + "/fail");
    if(!fail.exists())
        fail.mkdir(fail.path());

    m_thumbnailPath = thumbnails.path();
    m_thumbnailNormalPath = normal.path();
    m_thumbnailLargePath = large.path();
    m_thumbnailFailPath = fail.path();
}

void ThumbnailManager::initConnections()
{
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &ThumbnailManager::onFileChanged);
}

QString ThumbnailManager::getThumbnailCachePath()
{
    QStringList systemEnvs = QProcess::systemEnvironment();
    foreach (QString it, systemEnvs) {
        QString env = it.split("=").at(0);
        if(env == "XDG_CACHE_HOME")
            return it.split("=").at(1);
    }

    QDir::home().mkpath(".cache");
    QString cachePath = QString("%1/%2").arg(QDir::homePath(), ".cache");
    return cachePath;
}

QString ThumbnailManager::getThumbnailPath(const QString &name, ThumbnailGenerator::ThumbnailSize size)
{
    if(size == ThumbnailGenerator::THUMBNAIL_NORMAL)
        return QString("%1/%2.png").arg(m_thumbnailNormalPath,name);
    else if(size == ThumbnailGenerator::THUMBNAIL_LARGE)
        return QString("%1/%2.png").arg(m_thumbnailLargePath,name);
    else
        return m_thumbnailPath;
}

QIcon ThumbnailManager::getThumbnailIcon(const QUrl& fileUrl)
{
    QString fpath = fileUrl.path();

    // check if the source file is inside the thumbnail cache path
    if(fileUrl.isLocalFile()){
        int pos = fpath.lastIndexOf('/');
        if (m_thumbnailLargePath == fpath.left(pos)) {
            QIcon icon(fpath);
            return icon;
        }
    }

    // check last modify time
    if(m_md5ToIcon.contains(m_pathToMd5.value(fileUrl.toString()))&&fileUrl.isLocalFile()){

        QString thumbnailPath = QString("%1").arg(getThumbnailPath(m_pathToMd5.value(fileUrl.toString()), ThumbnailGenerator::THUMBNAIL_LARGE));
        QImage reader(thumbnailPath);
        QString dateStr = reader.text("Thumb::MTime");
        QFileInfo info(fpath);
        if(dateStr != QString::number(info.lastModified().toMSecsSinceEpoch()/1000)){
            m_md5ToIcon.take(m_pathToMd5.value(fileUrl.toString()));
            m_pathToMd5.take(fileUrl.toString());
            QFile::remove(thumbnailPath);
            return QIcon();
        }

        return m_md5ToIcon.value(m_pathToMd5.value(fileUrl.toString()));
    }

    return m_md5ToIcon.value(m_pathToMd5.value(fileUrl.toString()));
}

void ThumbnailManager::requestThumbnailIcon(const QUrl& fileUrl)
{
    if (m_pathToMd5.contains(fileUrl.toString()))
        return;

    if (taskQueue.contains(fileUrl.toString()))
        return;

    taskQueue << fileUrl.toString();

    if (!isRunning())
        start();
}

bool ThumbnailManager::canGenerateThumbnail(const QUrl& fileUrl)
{
    return m_thumbnailGenerator.canGenerateThumbnail(fileUrl);
}

QString ThumbnailManager::toMd5(const QString data)
{
    QByteArray bdata;
    bdata.append(data);
    QString md5 = QCryptographicHash::hash(bdata,QCryptographicHash::Md5).toHex().data();
    return md5;
}

void ThumbnailManager::onFileChanged(const QString &path)
{
    const QString &md5 = m_pathToMd5.take(path);

    if (!md5.isEmpty())
        emit iconChanged(path, QIcon());
}

void ThumbnailManager::run()
{
    while (!taskQueue.isEmpty()) {
        const QString &fileUrl = taskQueue.dequeue();

        QUrl url(fileUrl);
        QString fpath = url.path();

        if(url.isLocalFile())
            m_watcher->addPath(fpath);

        const QString &md5 = toMd5(fileUrl);

        m_pathToMd5[fileUrl] = md5;

        QIcon icon = m_md5ToIcon.value(md5);

        if (!icon.isNull()) {
            emit iconChanged(fpath, icon);

            continue;
        };

        QString thumbnailPath = QString("%1").arg(getThumbnailPath(md5, ThumbnailGenerator::THUMBNAIL_LARGE));

        if (QFile(thumbnailPath).exists()) {
            icon = QIcon(thumbnailPath);

            m_md5ToIcon[md5] = icon;
        }
        else if(m_thumbnailGenerator.canGenerateThumbnail(url)){
            const QPixmap &pixmap = m_thumbnailGenerator.generateThumbnail(url, ThumbnailGenerator::THUMBNAIL_LARGE);
            icon = QIcon(pixmap);
            m_md5ToIcon[md5] = icon;
            QImage img  = pixmap.toImage();

            //write extra attribute messages
            QMap<QString,QString> attributeSet = m_thumbnailGenerator.getAttributeSet(fileUrl);
            QList<QString> keys = attributeSet.keys();

            foreach (QString key, keys) {
                img.setText(key,attributeSet.value(key));
            }

            img.save(thumbnailPath,"png",20);
        }
        emit iconChanged(fpath, icon);
    }
}
