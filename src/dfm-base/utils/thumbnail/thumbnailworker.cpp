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
    thumbHelper.initSizeLimit();
}

QString ThumbnailWorkerPrivate::createThumbnail(const QUrl &url, Global::ThumbnailSize size)
{
    auto info = InfoFactory::create<FileInfo>(url);
    if (!info)
        return "";

    if (!thumbHelper.canGenerateThumbnail(url)) {
        qDebug() << "thumbnail: the file does not support generate thumbnails: " << url;
        return "";
    }

    const auto &absoluteFilePath = info->pathOf(PathInfoType::kAbsoluteFilePath);
    // if the file is in thumb dirs, just return the file itself
    if (thumbHelper.defaultThumbnailDirs().contains(info->pathOf(PathInfoType::kAbsolutePath)))
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

    return thumbHelper.saveThumbnail(url, img, size);
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
    d->isStoped = true;
}

void ThumbnailWorker::onTaskAdded(const QUrl &url, Global::ThumbnailSize size)
{
    if (d->isStoped)
        return;

    QUrl fileUrl = d->originalUrl = url;
    if (UrlRoute::isVirtual(url)) {
        auto info { InfoFactory::create<FileInfo>(url) };
        if (!info || !info->exists())
            return;

        fileUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kAbsoluteFilePath));
        if (!fileUrl.isLocalFile())
            return;
    }

    if (!d->thumbHelper.checkThumbEnable(fileUrl))
        return;

    const auto &img = d->thumbHelper.thumbnailImage(fileUrl, size);
    if (!img.isNull()) {
        Q_EMIT thumbnailCreateFinished(url, img.text(QT_STRINGIFY(Thumb::Path)));
        return;
    }

    createThumbnail(fileUrl, size);
}

void ThumbnailWorker::createThumbnail(const QUrl &url, Global::ThumbnailSize size)
{
    // check whether the file is stable
    // if not, rejoin the event queue and create thumbnail later
    if (!d->checkFileStable(url)) {
        QMetaObject::invokeMethod(this, "onTaskAdded", Qt::QueuedConnection,
                                    Q_ARG(QUrl, d->originalUrl),
                                    Q_ARG(Global::ThumbnailSize, size));
        return;
    }

    // create thumbnail
    const auto &thumbnailPath = d->createThumbnail(url, size);
    if (!thumbnailPath.isEmpty())
        Q_EMIT thumbnailCreateFinished(d->originalUrl, thumbnailPath);
    else
        Q_EMIT thumbnailCreateFailed(d->originalUrl);
}
