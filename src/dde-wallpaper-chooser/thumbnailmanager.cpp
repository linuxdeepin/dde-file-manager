/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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
#include <QImageReader>

static QPixmap ThumbnailImage(const QString &path, qreal scale)
{
    QUrl url = QUrl::fromPercentEncoding(path.toUtf8());
    QString realPath = url.toLocalFile();

    ThumbnailManager * tnm = ThumbnailManager::instance(scale);

    const qreal ratio = scale;

    QImageReader imageReader(realPath);
    imageReader.setDecideFormatFromContent(true);
    QImage image = imageReader.read();

    QPixmap pix = QPixmap::fromImage(image.scaled(QSize(static_cast<int>(ItemWidth * ratio), static_cast<int>(ItemHeight * ratio)),
                                                             Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    const QRect r(0, 0, static_cast<int>(ItemWidth * ratio), static_cast<int>(ItemHeight * ratio));
    const QSize size(static_cast<int>(ItemWidth * ratio), static_cast<int>(ItemHeight * ratio));

    if (pix.width() > ItemWidth * ratio || pix.height() > ItemHeight * ratio)
        pix = pix.copy(QRect(pix.rect().center() - r.center(), size));

    pix.setDevicePixelRatio(ratio);

    tnm->replace(path, pix);

    return pix;
}

ThumbnailManager::ThumbnailManager(qreal scale)
    : QObject(nullptr)
    , m_scale(scale)
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cacheDir = cacheDir + QDir::separator() + qApp->applicationVersion() + QDir::separator() + QString::number(scale);

    connect(&m_futureWatcher, &QFutureWatcher<QPixmap>::finished, this, &ThumbnailManager::onProcessFinished, Qt::QueuedConnection);

    QDir::root().mkpath(m_cacheDir);
}

ThumbnailManager::~ThumbnailManager()
{
    if (!m_queuedRequests.isEmpty())
        emit findAborted(m_queuedRequests);
}

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

    return pixmap.save(file);
}

void ThumbnailManager::stop()
{
    m_futureWatcher.cancel();
    m_queuedRequests.clear();
}

qreal ThumbnailManager::scale() const
{
    return m_scale;
}

ThumbnailManager *ThumbnailManager::instance(qreal scale)
{
    static ThumbnailManager *manager = new ThumbnailManager(scale);

    if (!qFuzzyCompare(manager->scale(), scale)) {
        manager->deleteLater();
        manager = new ThumbnailManager(scale);
    }

    return manager;
}

void ThumbnailManager::processNextReq()
{
    Q_ASSERT(m_futureWatcher.isFinished());

    const QString &item = m_queuedRequests.front();

    QFuture<QPixmap> future = QtConcurrent::run(ThumbnailImage, item, m_scale);
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
