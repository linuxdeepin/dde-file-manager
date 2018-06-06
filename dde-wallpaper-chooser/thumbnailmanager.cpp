/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
#include "constants.h"

#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QStandardPaths>
#include <QApplication>
#include <QUrl>
#include <QtConcurrent>

static QPixmap ThumbnailImage(const QString &path)
{
    QUrl url = QUrl::fromPercentEncoding(path.toUtf8());
    QString realPath = url.toLocalFile();

    ThumbnailManager * tnm = ThumbnailManager::instance();

    const qreal ratio = qApp->devicePixelRatio();

    QPixmap pix = QPixmap::fromImage(QImage(realPath).scaled(QSize(ItemWidth * ratio, ItemHeight * ratio),
                                                             Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    const QRect r(0, 0, ItemWidth * ratio, ItemHeight * ratio);
    const QSize size(ItemWidth * ratio, ItemHeight * ratio);

    if (pix.width() > ItemWidth * ratio || pix.height() > ItemHeight * ratio)
        pix = pix.copy(QRect(pix.rect().center() - r.center(), size));

    pix.setDevicePixelRatio(ratio);

    tnm->replace(path, pix);

    return pix;
}

ThumbnailManager::ThumbnailManager() :
    QObject(NULL)
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cacheDir = cacheDir + QDir::separator() + qApp->applicationVersion() + QDir::separator() + QString::number(qApp->devicePixelRatio());

    connect(&m_futureWatcher, &QFutureWatcher<QPixmap>::finished, this, &ThumbnailManager::onProcessFinished, Qt::QueuedConnection);

    QDir::root().mkpath(m_cacheDir);
}

Q_GLOBAL_STATIC(ThumbnailManager, ThumbnailManagerInstance)

void ThumbnailManager::clear()
{
    QDir dir(m_cacheDir);
    dir.removeRecursively();
    QDir::root().mkpath(m_cacheDir);
}

void ThumbnailManager::find(const QString &key)
{
    QString file = QDir(m_cacheDir).absoluteFilePath(key);
    const QPixmap pixmap(file);
    if (!pixmap.isNull())
    {
        emit thumbnailFounded(key, pixmap);
        return;
    }

    m_queuedRequests << key;

    // first item, start
    if (m_queuedRequests.size() == 1)
        processNextReq();
}

void ThumbnailManager::remove(const QString &key)
{
    QString file = QDir(m_cacheDir).absoluteFilePath(key);
    if (QFile::exists(file)) {
        QFile(file).remove();
    }
}

bool ThumbnailManager::replace(const QString &key, const QPixmap &pixmap)
{
    QString file = QDir(m_cacheDir).absoluteFilePath(key);
    if (QFile::exists(file)) {
        QFile(file).remove();
    }
    pixmap.save(file);

    return true;
}

void ThumbnailManager::stop()
{
    m_futureWatcher.cancel();
    m_queuedRequests.clear();
}

ThumbnailManager *ThumbnailManager::instance()
{
    return ThumbnailManagerInstance;
}

void ThumbnailManager::processNextReq()
{
    Q_ASSERT(m_futureWatcher.isFinished());

    const QString &item = m_queuedRequests.front();

    QFuture<QPixmap> future = QtConcurrent::run(ThumbnailImage, item);
    m_futureWatcher.setFuture(future);
}

void ThumbnailManager::onProcessFinished()
{
    if (m_futureWatcher.isCanceled()) return;

    emit thumbnailFounded(m_queuedRequests.front(), m_futureWatcher.result());

    m_queuedRequests.pop_front();

    if (!m_queuedRequests.isEmpty())
        processNextReq();
}
