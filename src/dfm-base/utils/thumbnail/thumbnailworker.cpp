// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailworker.h"
#include "private/thumbnailworker_p.h"
#include "thumbnailcreators.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>

#include <QtConcurrent>
#include <QPainter>
#include <QDebug>

using namespace dfmbase;

ThumbnailWorkerPrivate::ThumbnailWorkerPrivate(ThumbnailWorker *qq)
    : q(qq)
{
}

QString ThumbnailWorkerPrivate::createThumbnail(const QUrl &url, Global::ThumbnailSize size)
{
    auto info = InfoFactory::create<FileInfo>(url);
    if (!info)
        return "";

    if (!ThumbnailHelper::instance()->canGenerateThumbnail(url)) {
        qDebug() << "thumbnail: the file does not support generate thumbnails: " << url;
        return "";
    }

    const auto &absoluteFilePath = info->pathOf(PathInfoType::kAbsoluteFilePath);
    // if the file is in thumb dirs, just return the file itself
    if (ThumbnailHelper::defaultThumbnailDirs().contains(info->pathOf(PathInfoType::kAbsolutePath)))
        return absoluteFilePath;

    QImage img;
    const auto &mime = mimeDb.mimeTypeForUrl(url);
    const auto &mimeName = mime.name();

    if (creators.contains(mimeName)) {   // accularate match
        img = creators.value(mimeName)(absoluteFilePath, size);
    } else {   // pattern match
        for (auto &mimeRegx : creators.keys()) {
            QRegularExpression regx(mimeRegx);
            if (mimeName.contains(regx)) {
                img = creators.value(mimeRegx)(absoluteFilePath, size);
                break;
            }
        }
    }

    // default image generator if cannot create by customized function
    if (img.isNull())
        img = ThumbnailCreators::defaultThumbnailCreator(absoluteFilePath, size);

    if (img.isNull()) {
        qDebug() << "thumbnail: cannot generate thumbnail for file: " << url;
        return "";
    }

    if (img.height() > size || img.width() > size)
        img = img.scaled({ size, size }, Qt::KeepAspectRatio);

    return ThumbnailHelper::instance()->saveThumbnail(url, img, size);
}

bool ThumbnailWorkerPrivate::checkFileStable(const QUrl &url)
{
    const auto &info = InfoFactory::create<FileInfo>(url);
    if (!info)
        return true;

    // 修改时间稳定
    qint64 mtime = info->timeOf(TimeInfoType::kLastModifiedSecond).toLongLong();
    qint64 curTime = QDateTime::currentDateTime().toTime_t();
    if (curTime - mtime < 2)
        return false;

    return true;
}

void ThumbnailWorkerPrivate::insertUrl(const QUrl &key, const QUrl &value)
{
    QMutexLocker lk(&mutex);
    localAndVirtualHash.insert(key, value);
}

QUrl ThumbnailWorkerPrivate::takeUrl(const QUrl &key)
{
    QMutexLocker lk(&mutex);
    if (!localAndVirtualHash.contains(key))
        return key;

    return localAndVirtualHash.take(key);
}

QIcon ThumbnailWorkerPrivate::createIcon(const QString &iconName)
{
    QIcon icon(iconName);
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(Global::kLarge, Global::kLarge);
        QPainter pa(&pixmap);
        pa.setPen(Qt::gray);
        pa.drawPixmap(0, 0, pixmap);

        icon.addPixmap(pixmap);
    }

    return icon;
}

ThumbnailWorker::ThumbnailWorker(QObject *parent)
    : QObject(parent),
      d(new ThumbnailWorkerPrivate(this))
{
}

ThumbnailWorker::~ThumbnailWorker()
{
    stop();
}

bool ThumbnailWorker::registerCreator(const QString &mimeType, ThumbnailWorker::ThumbnailCreator creator)
{
    Q_ASSERT(creator);

    if (d->creators.contains(mimeType)) {
        qWarning() << "register failed, the mime type has already been registered." << mimeType;
        return false;
    }

    d->creators.insert(mimeType, creator);
    return true;
}

void ThumbnailWorker::stop()
{
    if (d->isRunning) {
        d->isRunning = false;
        d->waitCon.wakeAll();
        d->future.waitForFinished();
    }
}

void ThumbnailWorker::onTaskAdded(const QUrl &url, Global::ThumbnailSize size)
{
    QUrl fileUrl = url;
    bool isTransform = false;
    if (UrlRoute::isVirtual(fileUrl)) {
        auto info { InfoFactory::create<FileInfo>(url) };
        fileUrl = info->urlOf(UrlInfoType::kRedirectedFileUrl);
        if (!fileUrl.isLocalFile())
            return;
        isTransform = true;
    }

    if (!ThumbnailHelper::instance()->checkThumbEnable(fileUrl))
        return;

    const auto &img = ThumbnailHelper::instance()->thumbnailImage(fileUrl, size);
    if (!img.isNull()) {
        Q_EMIT thumbnailCreateFinished(url, d->createIcon(img.text(QT_STRINGIFY(Thumb::Path))));
        return;
    }

    if (contains(fileUrl)) return;
    {
        QMutexLocker lk(&d->mutex);
        d->produceTasks.enqueue({ fileUrl, size });
    }

    if (isTransform)
        d->insertUrl(fileUrl, url);
    if (d->isRunning)
        d->waitCon.wakeAll();
    else
        d->future = QtConcurrent::run(this, &ThumbnailWorker::doWork);
}

void ThumbnailWorker::onTaskRemoved(const QUrl &url)
{
    if (!url.isLocalFile())
        return;

    QMutexLocker lk(&d->mutex);
    auto it = d->produceTasks.begin();
    while (it != d->produceTasks.end()) {
        const auto &path = it->srcUrl.path();
        if (path.startsWith(url.path()))
            it = d->produceTasks.erase(it);
        else
            ++it;
    }
}

void ThumbnailWorker::doWork()
{
    d->isRunning = true;
    forever {
        if (!d->isRunning)
            return;

        if (d->produceTasks.isEmpty()) {
            QMutexLocker lk(&d->mutex);
            d->waitCon.wait(&d->mutex);

            if (!d->isRunning)
                return;
        }

        d->mutex.lock();
        const auto &task = d->produceTasks.dequeue();
        if (!d->checkFileStable(task.srcUrl)) {
            d->produceTasks.enqueue(task);
            d->mutex.unlock();
            continue;
        }
        d->mutex.unlock();

        // create thumbnail
        const auto &thumbnailPath = d->createThumbnail(task.srcUrl, task.size);
        if (!thumbnailPath.isEmpty())
            Q_EMIT thumbnailCreateFinished(d->takeUrl(task.srcUrl), d->createIcon(thumbnailPath));
        else
            Q_EMIT thumbnailCreateFailed(d->takeUrl(task.srcUrl));
    }
}

bool ThumbnailWorker::contains(const QUrl &url)
{
    QMutexLocker lk(&d->mutex);
    return std::any_of(d->produceTasks.begin(), d->produceTasks.end(), [&url](const ProduceTask &task) {
        return UniversalUtils::urlEquals(url, task.srcUrl);
    });
}
