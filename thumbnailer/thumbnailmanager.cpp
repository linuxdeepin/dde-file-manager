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
#include <QFileInfo>
#include <QFile>
#include <QElapsedTimer>

#include "QPainter"

ThumbnailManager::ThumbnailManager(QObject *parent)
    : QThread(parent)
{
    init();

//    start();
}

ThumbnailManager::~ThumbnailManager()
{
    running = false;
    waitCondition.wakeAll();
    wait();
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

    m_maxTaskCacheNum = 104;
}

QString ThumbnailManager::getThumbnailCachePath() const
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

QString ThumbnailManager::getThumbnailPath(const QString &name, ThumbnailGenerator::ThumbnailSize size) const
{

    if(size == ThumbnailGenerator::THUMBNAIL_NORMAL){
        QDir dir(m_thumbnailNormalPath);
        if(!dir.exists())
            dir.mkpath(m_thumbnailNormalPath);
        return QString("%1/%2.png").arg(m_thumbnailNormalPath,name);
    } else if(size == ThumbnailGenerator::THUMBNAIL_LARGE){
        QDir dir(m_thumbnailLargePath);
        if(!dir.exists())
            dir.mkpath(m_thumbnailLargePath);
        return QString("%1/%2.png").arg(m_thumbnailLargePath,name);
    }
    else
        return m_thumbnailPath;
}

QString ThumbnailManager::getThumbnailFailPath(const QString &name) const
{
    return QString("%1/%2.png").arg(m_thumbnailFailPath,name);
}

QPixmap ThumbnailManager::getThumbnailPixmap(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size)
{
    QString fpath = fileUrl.path();

    // check if the source file is inside the thumbnail cache path
    if(fileUrl.isLocalFile()){
        int pos = fpath.lastIndexOf('/');
        if (m_thumbnailLargePath == fpath.left(pos)) {
            QPixmap icon(fpath);
            return icon;
        }
    }

    const QPixmap &pixmap = m_urlToPixmap.value(fileUrl);

    // check last modify time
    if (!pixmap.isNull() && fileUrl.isLocalFile()) {
        // first of all , check fail thumbnail path
        QString md5 = toMd5(fileUrl.toString());
        QString thumbnailFailPath = getThumbnailFailPath(md5);
        QFileInfo failedFInfo(thumbnailFailPath);

        if (failedFInfo.exists()) {
            QImage failedImg(thumbnailFailPath);

            if (failedImg.text("Thumb::MTime")!=QString::number(failedFInfo.lastModified().toMSecsSinceEpoch()/1000)
                    || failedImg.text("Thumb::Size")!= QString::number(QFile(thumbnailFailPath).size())) {
                m_urlToPixmap.take(fileUrl);
                QFile::remove(thumbnailFailPath);

                return QPixmap();
            } else {
                return getDefaultPixmap(fileUrl);
            }
        }

        // check thumbnail path(normal or large)
        QString thumbnailPath = getThumbnailPath(md5, size);
        QImage reader(thumbnailPath);
        QString dateStr = reader.text("Thumb::MTime");
        QFileInfo info(fpath);

        if (dateStr != QString::number(info.lastModified().toMSecsSinceEpoch()/1000)) {
            m_urlToPixmap.take(fileUrl);
            QFile::remove(thumbnailPath);
        }
    }

    return pixmap;
}

void ThumbnailManager::requestThumbnailPixmap(const QUrl& fileUrl, ThumbnailGenerator::ThumbnailSize size, const int& quality)
{
    if (m_urlToPixmap.contains(fileUrl) && QFile::exists(getThumbnailPath(toMd5(fileUrl.toString()), size)))
        return;

    ThumbnailTask task(fileUrl, size, quality);

//    taskCache.insert(0,task);
//    if(taskCache.count()>200)
//        taskCache.removeLast();
    QWriteLocker locker(&readWriteLockTaskQueue);
    taskQueue << task;
    locker.unlock();
    waitCondition.wakeAll();
//    if(taskQueue.count() > 104)
//        taskQueue.removeFirst();
}

QPixmap ThumbnailManager::getDefaultPixmap(const QUrl &fileUrl)
{
    Q_UNUSED(fileUrl)

    //TODO
    return QPixmap();
}

bool ThumbnailManager::canGenerateThumbnail(const QUrl& fileUrl)
{
    return m_thumbnailGenerator.canGenerateThumbnail(fileUrl);
}

void ThumbnailManager::abortGetThumbnailPixmap(const QUrl &fileUrl, ThumbnailGenerator::ThumbnailSize size, const int &quality)
{
    QWriteLocker locker(&readWriteLockTaskQueue);
    Q_UNUSED(locker)
    taskQueue.removeOne(ThumbnailTask(fileUrl, size, quality));
}

QString ThumbnailManager::toMd5(const QString data)
{
    QByteArray bdata;
    bdata.append(data);
    QString md5 = QCryptographicHash::hash(bdata,QCryptographicHash::Md5).toHex().data();
    return md5;
}

void ThumbnailManager::run()
{
    forever {
        QWriteLocker locker(&readWriteLockTaskQueue);
        if (taskQueue.isEmpty()) {
            waitCondition.wait(&readWriteLockTaskQueue);
        }

        if (!running)
            return;

//        const QString &fileUrl = taskQueue.dequeue();
        const ThumbnailTask &task = taskQueue.dequeue();
        locker.unlock();

        QString fpath = task.fileUrl.path();
        QPixmap pixmap = m_urlToPixmap.value(task.fileUrl);

        if (!pixmap.isNull()) {
            emit pixmapChanged(fpath, pixmap);

            continue;
        };

        const QString &md5 = toMd5(task.fileUrl.toString());
        QString thumbnailPath = QString("%1").arg(getThumbnailPath(md5, task.size));

        if (QFile(thumbnailPath).exists()) {
            pixmap = QPixmap(thumbnailPath);

            m_urlToPixmap[task.fileUrl] = pixmap;
        }
        else if(m_thumbnailGenerator.canGenerateThumbnail(task.fileUrl)){
            const QPixmap &thumbNailedpixmap = m_thumbnailGenerator.generateThumbnail(task.fileUrl, task.size);
            if(thumbNailedpixmap.isNull()){
                // deal with fail thumbnailing pixmap
                QString thumbnailFailedPath = getThumbnailFailPath(md5);
                QImage img(1,1,QImage::Format_ARGB32_Premultiplied);

                QMap<QString,QString> attributeSet = m_thumbnailGenerator.getAttributeSet(task.fileUrl);
                QList<QString> keys = attributeSet.keys();

                foreach (QString key, keys) {
                    img.setText(key,attributeSet.value(key));
                }
                m_urlToPixmap[task.fileUrl] = QPixmap();

                img.save(thumbnailFailedPath,"png");

                //do not emit pixmap changed, otherwise it will turn to a death loops
                continue;
            }
            QImage img  = thumbNailedpixmap.toImage();

            //write extra attribute messages
            QMap<QString,QString> attributeSet = m_thumbnailGenerator.getAttributeSet(task.fileUrl);
            QList<QString> keys = attributeSet.keys();

            foreach (QString key, keys) {
                img.setText(key,attributeSet.value(key));
            }

            img.save(thumbnailPath,"png",task.quality);
            pixmap = QPixmap(thumbnailPath);
            m_urlToPixmap[task.fileUrl] = pixmap;
        }

        emit pixmapChanged(fpath, pixmap);
    }
}
