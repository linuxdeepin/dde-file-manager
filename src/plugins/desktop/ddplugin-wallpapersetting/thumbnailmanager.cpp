/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "thumbnailmanager.h"
#include "wallpaperlist.h"

#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QtConcurrent>
#include <QImageReader>

DDP_WALLPAERSETTING_USE_NAMESPACE

ThumbnailManager::ThumbnailManager(qreal _scale, QObject *parent)
    : QObject(parent)
    , scale(_scale)
{
    cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    // old dir
    cacheDir = cacheDir + QDir::separator() + qApp->applicationVersion() + QDir::separator() + QString::number(scale);
    //cacheDir = cacheDir + QDir::separator() + QString("wallpaperthumbnail") + QDir::separator() + QString::number(scale);

    connect(&futureWatcher, &QFutureWatcher<QPixmap>::finished, this, &ThumbnailManager::onProcessFinished, Qt::QueuedConnection);

    QDir::root().mkpath(cacheDir);
}

ThumbnailManager::~ThumbnailManager()
{
    if (!queuedRequests.isEmpty())
        emit findAborted(queuedRequests);
}

ThumbnailManager *ThumbnailManager::instance(qreal scale)
{
    static QMutex mtx;
    mtx.lock();

    static ThumbnailManager *manager = new ThumbnailManager(scale);
    if (!qFuzzyCompare(manager->scale, scale)) {
        manager->deleteLater();
        manager = new ThumbnailManager(scale);
    }
    mtx.unlock();

    return manager;
}

void ThumbnailManager::find(const QString &key)
{
    QString file = QDir(cacheDir).absoluteFilePath(key);
    const QPixmap pixmap(file);
    if (!pixmap.isNull()) {
        emit thumbnailFounded(key, pixmap);
        return;
    }

    queuedRequests << key;

    // first item, start
    if (queuedRequests.size() == 1)
        processNextReq();
}

void ThumbnailManager::stop()
{
    futureWatcher.cancel();
    queuedRequests.clear();
}

bool ThumbnailManager::replace(const QString &key, const QPixmap &pixmap)
{
    QString file = QDir(cacheDir).absoluteFilePath(key);
    if (QFile::exists(file)) {
        QFile(file).remove();
    }

    return pixmap.save(file);
}

QPixmap ThumbnailManager::thumbnailImage(const QString &key, qreal scale)
{
    ThumbnailManager * tnm = ThumbnailManager::instance(scale);
    // key is percent-encoded filepath. see WallpaperItem::setPath and WallpaperItem::thumbnailKey
    const QString realPath = QUrl(QUrl::fromPercentEncoding(key.toUtf8())).toLocalFile();
    const qreal ratio = scale;

    QImageReader imageReader(realPath);
    imageReader.setDecideFormatFromContent(true);
    QImage image = imageReader.read();
    const int itemWidth = static_cast<int>(WallpaperList::kItemWidth * ratio);
    const int itemHeight = static_cast<int>(WallpaperList::kItemHeight * ratio);
    QPixmap pix = QPixmap::fromImage(image.scaled(QSize(itemWidth, itemHeight), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    const QRect r(0, 0, itemWidth, itemHeight);
    const QSize size(itemWidth, itemHeight);

    if (pix.width() > itemWidth || pix.height() > itemHeight)
        pix = pix.copy(QRect(pix.rect().center() - r.center(), size));

    pix.setDevicePixelRatio(ratio);

    tnm->replace(key, pix);

    return pix;
}

void ThumbnailManager::onProcessFinished()
{
    if (futureWatcher.isCanceled())
        return;

    emit thumbnailFounded(queuedRequests.front(), futureWatcher.result());

    queuedRequests.pop_front();

    if (!queuedRequests.isEmpty())
        processNextReq();
}

void ThumbnailManager::processNextReq()
{
    Q_ASSERT(futureWatcher.isFinished());

    const QString &item = queuedRequests.front();

    QFuture<QPixmap> future = QtConcurrent::run(ThumbnailManager::thumbnailImage, item, scale);
    futureWatcher.setFuture(future);
}
