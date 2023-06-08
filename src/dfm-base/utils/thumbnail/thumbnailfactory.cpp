// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailfactory.h"
#include "thumbnailcreators.h"
#include "private/thumbnailfactory_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>

using namespace dfmbase;
DFMGLOBAL_USE_NAMESPACE

ThumbnailFactoryPrivate::ThumbnailFactoryPrivate(ThumbnailFactory *qq)
    : q(qq)
{
}

QString ThumbnailFactoryPrivate::createThumbnail(const QUrl &url, ThumbnialSize size)
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
        Q_EMIT q->produceFailed(url);
        return "";
    }

    if (img.height() > size || img.width() > size)
        img = img.scaled({ size, size }, Qt::KeepAspectRatio);

    QString thumbnailPath = ThumbnailHelper::instance()->saveThumbnail(url, img, size);
    if (!thumbnailPath.isEmpty())
        Q_EMIT q->produceFinished(url, thumbnailPath);
    else
        Q_EMIT q->produceFailed(url);

    return thumbnailPath;
}

bool ThumbnailFactoryPrivate::checkFileStable(const QUrl &url)
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

ThumbnailFactory::ThumbnailFactory(QObject *parent)
    : QThread(parent),
      d(new ThumbnailFactoryPrivate(this))
{
    registerThumbnailCreator(Mime::kTypeImageVDjvu, ThumbnailCreators::djvuThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeImageVDMultipage, ThumbnailCreators::djvuThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeTextPlain, ThumbnailCreators::textThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppPdf, ThumbnailCreators::pdfThumbnailCreator);
    registerThumbnailCreator(Mime::kTypeAppVRRMedia, ThumbnailCreators::videoThumbnailCreatorFfmpeg);
    registerThumbnailCreator("image/*", ThumbnailCreators::imageThumbnailCreator);
    registerThumbnailCreator("audio/*", ThumbnailCreators::audioThumbnailCreator);
    registerThumbnailCreator("video/*", ThumbnailCreators::videoThumbnailCreator);

    initConnections();
}

ThumbnailFactory::~ThumbnailFactory()
{
    if (d->running) {
        d->running = false;
        d->waitCon.wakeAll();
    }

    quit();
    wait();
}

void ThumbnailFactory::initConnections()
{
    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, &ThumbnailFactory::onDevUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &ThumbnailFactory::onDevUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, &ThumbnailFactory::onDevUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevRemoved, this, &ThumbnailFactory::onDevUnmounted);
}

void ThumbnailFactory::run()
{
    d->running = true;
    forever {
        if (!d->running)
            return;

        if (d->produceTasks.isEmpty()) {
            QMutexLocker lk(&d->mutex);
            d->waitCon.wait(&d->mutex);

            if (!d->running)
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
        d->createThumbnail(task.srcUrl, task.size);
    }
}

QImage ThumbnailFactory::createThumbnail(const QUrl &url, ThumbnialSize size)
{
    const QImage &img = ThumbnailHelper::instance()->thumbnailImage(url, size);
    if (!img.isNull())
        return img;

    if (!contains(url)) {
        QMutexLocker lk(&d->mutex);
        d->produceTasks.enqueue({ url, size });
    }

    isRunning() ? d->waitCon.wakeAll() : start();
    return {};
}

void ThumbnailFactory::removeTasksWithUrl(const QUrl &url)
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

bool ThumbnailFactory::registerThumbnailCreator(const QString &mimeType, ThumbnailCreator creator)
{
    Q_ASSERT(creator);

    if (d->creators.contains(mimeType))
        return false;

    d->creators.insert(mimeType, creator);
    return true;
}

bool ThumbnailFactory::contains(const QUrl &url)
{
    QMutexLocker lk(&d->mutex);
    return std::any_of(d->produceTasks.begin(), d->produceTasks.end(), [&url](const ProduceTask &task) {
        return UniversalUtils::urlEquals(url, task.srcUrl);
    });
}

void ThumbnailFactory::onDevUnmounted(const QString &id, const QString &oldMpt)
{
    Q_UNUSED(id)

    const auto &&mpt = QUrl::fromLocalFile(oldMpt);
    removeTasksWithUrl(mpt);
}
