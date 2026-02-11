// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
    if (!info) {
        qCWarning(logDFMBase) << "thumbnail: failed to create FileInfo for URL:" << url;
        return "";
    }

    if (!thumbHelper.canGenerateThumbnail(url)) {
        qCDebug(logDFMBase) << "thumbnail: file does not support thumbnail generation:" << url;
        return "";
    }

    const auto &absoluteFilePath = info->pathOf(PathInfoType::kAbsoluteFilePath);
    // if the file is in thumb dirs, just return the file itself
    if (thumbHelper.defaultThumbnailDirs().contains(info->pathOf(PathInfoType::kAbsolutePath))) {
        qCDebug(logDFMBase) << "thumbnail: file is already in thumbnail directory, returning original path:" << absoluteFilePath;
        return absoluteFilePath;
    }

    QImage img;
    const auto &mime = mimeDb.mimeTypeForUrl(url);
    const auto &mimeName = mime.name();

    if (creators.contains(mimeName)) {   // accurate match
        qCDebug(logDFMBase) << "thumbnail: using exact mime type creator for:" << mimeName;
        img = creators.value(mimeName)(absoluteFilePath, size);
    } else {
        // Try parent MIME types for inheritance support (e.g., WPS override)
        QStringList parentTypes = mime.parentMimeTypes();
        for (const QString &parentType : parentTypes) {
            if (creators.contains(parentType)) {
                qCDebug(logDFMBase) << "thumbnail: using parent mime type creator for:" << mimeName << "matched parent:" << parentType;
                img = creators.value(parentType)(absoluteFilePath, size);
                break;
            }
        }

        // If no parent match, try pattern matching
        if (img.isNull()) {
            for (auto &mimeRegx : creators.keys()) {
                QRegularExpression regx(mimeRegx);
                if (mimeName.contains(regx)) {
                    qCDebug(logDFMBase) << "thumbnail: using pattern creator for mime type:" << mimeName << "with pattern:" << mimeRegx;
                    img = creators.value(mimeRegx)(absoluteFilePath, size);
                    break;
                }
            }
        }
    }

    // default image generator if cannot create by customized function
    if (img.isNull()) {
        qCDebug(logDFMBase) << "thumbnail: using default creator for:" << url;
        img = ThumbnailCreators::defaultThumbnailCreator(absoluteFilePath, size);
    }

    if (img.isNull()) {
        qCWarning(logDFMBase) << "thumbnail: failed to generate thumbnail for file:" << url;
        return "";
    }

    if (img.height() > size || img.width() > size) {
        qCDebug(logDFMBase) << "thumbnail: scaling image from" << img.size() << "to fit size:" << size;
        img = img.scaled({ size, size }, Qt::KeepAspectRatio);
    }

    const QString thumbnailPath = thumbHelper.saveThumbnail(url, img, size);
    if (!thumbnailPath.isEmpty()) {
        qCInfo(logDFMBase) << "thumbnail: successfully created thumbnail for:" << url << "saved to:" << thumbnailPath;
    }

    return thumbnailPath;
}

bool ThumbnailWorkerPrivate::checkFileStable(const QUrl &url)
{
    const auto &info = InfoFactory::create<FileInfo>(url);
    if (!info) {
        qCWarning(logDFMBase) << "thumbnail: failed to create FileInfo for stability check:" << url;
        return true;
    }

    // Check if modification time is stable
    qint64 mtime = info->timeOf(TimeInfoType::kMetadataChangeTimeSecond).toLongLong();
    qint64 curTime = QDateTime::currentDateTime().toSecsSinceEpoch();
    qint64 diffTime = curTime - mtime;

    if (diffTime < 2 && diffTime >= 0) {
        qCDebug(logDFMBase) << "thumbnail: file is not stable, modification time difference:" << diffTime << "seconds for:" << url;
        return false;
    }

    return true;
}

void ThumbnailWorkerPrivate::startDelayWork()
{
    if (!delayTimer) {
        delayTimer = new QTimer(q);
        delayTimer->setInterval(2 * 1000);
        delayTimer->setSingleShot(true);
        q->connect(
                delayTimer, &QTimer::timeout, q, [this] { q->onTaskAdded(delayTaskMap); }, Qt::QueuedConnection);
        qCDebug(logDFMBase) << "thumbnail: delay timer initialized with 2 second interval";
    }

    delayTimer->start();
    qCDebug(logDFMBase) << "thumbnail: delay timer started for" << delayTaskMap.size() << "tasks";
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
        qCWarning(logDFMBase) << "thumbnail: failed to register creator, mime type already registered:" << mimeType;
        return false;
    }

    d->creators.insert(mimeType, creator);
    qCInfo(logDFMBase) << "thumbnail: successfully registered creator for mime type:" << mimeType;
    return true;
}

void ThumbnailWorker::stop()
{
    d->isStoped = true;
    qCInfo(logDFMBase) << "thumbnail: ThumbnailWorker stopped";
}

void ThumbnailWorker::onTaskAdded(const ThumbnailTaskMap &taskMap)
{
    if (d->isStoped) {
        qCDebug(logDFMBase) << "thumbnail: worker is stopped, ignoring" << taskMap.size() << "tasks";
        return;
    }

    qCInfo(logDFMBase) << "thumbnail: processing" << taskMap.size() << "thumbnail tasks";

    QMapIterator<QUrl, Global::ThumbnailSize> iter(taskMap);
    while (iter.hasNext()) {
        iter.next();
        QUrl fileUrl = d->originalUrl = iter.key();
        if (!d->thumbHelper.checkThumbEnable(fileUrl)) {
            continue;
        }

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
            d->urlCheckCountMap[d->originalUrl] = 1;
            qCDebug(logDFMBase) << "thumbnail: file not stable, adding to delay queue:" << d->originalUrl;
        } else {
            d->delayTaskMap.remove(d->originalUrl);
            // Give up generation after 10 attempts
            auto count = d->urlCheckCountMap.value(d->originalUrl, 0);
            if (++count > 10) {
                qCWarning(logDFMBase) << "thumbnail: giving up after 10 stability check attempts for:" << d->originalUrl;
                d->urlCheckCountMap.remove(d->originalUrl);   // 清理映射表
                return;
            }

            d->urlCheckCountMap[d->originalUrl] = count;
            qCDebug(logDFMBase) << "thumbnail: file still not stable, retry count:" << count << "for:" << d->originalUrl;
        }

        d->delayTaskMap.insert(d->originalUrl, size);
        d->startDelayWork();
        return;
    } else if (d->urlCheckCountMap.contains(d->originalUrl)) {
        // 文件已稳定，清理重试计数
        d->urlCheckCountMap.remove(d->originalUrl);
        qCDebug(logDFMBase) << "thumbnail: file is now stable, cleared check count for:" << d->originalUrl;
    }

    // create thumbnail
    const auto &thumbnailPath = d->createThumbnail(url, size);
    if (!thumbnailPath.isEmpty()) {
        qCInfo(logDFMBase) << "thumbnail: thumbnail creation completed for:" << d->originalUrl;
        Q_EMIT thumbnailCreateFinished(d->originalUrl, thumbnailPath);
    } else {
        qCWarning(logDFMBase) << "thumbnail: thumbnail creation failed for:" << d->originalUrl;
        Q_EMIT thumbnailCreateFailed(d->originalUrl);
    }
}
