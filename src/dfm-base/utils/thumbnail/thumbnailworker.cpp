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
        qCDebug(logDFMBase) << "thumbnail: the file does not support generate thumbnails: " << url;
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
        qCWarning(logDFMBase) << "thumbnail: cannot generate thumbnail for file: " << url;
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
    qint64 mtime = info->timeOf(TimeInfoType::kMetadataChangeTimeSecond).toLongLong();
    qint64 curTime = QDateTime::currentDateTime().toSecsSinceEpoch();
    qint64 diffTime = curTime - mtime;

    if (diffTime < 2 && diffTime >= 0)
        return false;

    return true;
}

void ThumbnailWorkerPrivate::startDelayWork()
{
    if (!delayTimer) {
        delayTimer = new QTimer(q);
        delayTimer->setInterval(2 * 1000);
        delayTimer->setSingleShot(true);
        q->connect(delayTimer, &QTimer::timeout, q, [this] { q->onTaskAdded(delayTaskMap); }, Qt::QueuedConnection);
    }

    delayTimer->start();
}

QUrl ThumbnailWorkerPrivate::setCheckCount(const QUrl &url, int count)
{
    QUrl tmpUrl(url);
    QUrlQuery query(url.query());
    query.removeQueryItem("checkCount");
    query.addQueryItem("checkCount", QString::number(count));
    tmpUrl.setQuery(query);

    return tmpUrl;
}

int ThumbnailWorkerPrivate::checkCount(const QUrl &url)
{
    if (!url.hasQuery())
        return 0;

    QUrlQuery query(url.query());
    return query.queryItemValue("checkCount").toInt();
}

QUrl ThumbnailWorkerPrivate::clearCheckCount(const QUrl &url)
{
    if (!url.hasQuery())
        return url;

    QUrl tmpUrl(url);
    QUrlQuery query(url.query());
    query.removeQueryItem("checkCount");
    tmpUrl.setQuery(query);

    return tmpUrl;
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
        qCWarning(logDFMBase) << "register failed, the mime type has already been registered." << mimeType;
        return false;
    }

    d->creators.insert(mimeType, creator);
    return true;
}

void ThumbnailWorker::stop()
{
    d->isStoped = true;
}

void ThumbnailWorker::onTaskAdded(const ThumbnailTaskMap &taskMap)
{
    if (d->isStoped)
        return;

    QMapIterator<QUrl, Global::ThumbnailSize> iter(taskMap);
    while (iter.hasNext()) {
        iter.next();
        QUrl fileUrl = d->originalUrl = iter.key();
        if (!d->thumbHelper.checkThumbEnable(fileUrl))
            continue;

        const auto &img = d->thumbHelper.thumbnailImage(fileUrl, iter.value());
        if (!img.isNull()) {
            Q_EMIT thumbnailCreateFinished(iter.key(), img.text(QT_STRINGIFY(Thumb::Path)));
            continue;
        }

        createThumbnail(fileUrl, iter.value());
    }
}

void ThumbnailWorker::createThumbnail(const QUrl &url, Global::ThumbnailSize size)
{
    // check whether the file is stable
    // if not, rejoin the event queue and create thumbnail later
    if (!d->checkFileStable(url)) {
        if (!d->delayTaskMap.contains(d->originalUrl)) {
            d->originalUrl = d->setCheckCount(d->originalUrl, 1);
        } else {
            d->delayTaskMap.remove(d->originalUrl);
            // 超过10次，放弃生成
            auto count = d->checkCount(d->originalUrl);
            if (++count > 10)
                return;

            d->originalUrl = d->setCheckCount(d->originalUrl, count);
        }

        d->delayTaskMap.insert(d->originalUrl, size);
        d->startDelayWork();
        return;
    } else if (d->originalUrl.hasQuery()) {
        d->originalUrl = d->clearCheckCount(d->originalUrl);
    }

    // create thumbnail
    const auto &thumbnailPath = d->createThumbnail(url, size);
    if (!thumbnailPath.isEmpty())
        Q_EMIT thumbnailCreateFinished(d->originalUrl, thumbnailPath);
    else
        Q_EMIT thumbnailCreateFailed(d->originalUrl);
}
