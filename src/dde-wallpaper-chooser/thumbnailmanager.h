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
#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include <QQueue>
#include <QFutureWatcher>
#include <QPixmap>

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(qreal scale);
    ~ThumbnailManager();

    void clear();
    void find(const QString & key);
    void remove(const QString & key);
    bool replace(const QString & key, const QPixmap & pixmap);

    void stop();
    qreal scale() const;

    static ThumbnailManager * instance(qreal scale);

signals:
    void thumbnailFounded(const QString &key, const QPixmap &pixmap);
    void findAborted(QQueue<QString> queue);

private:
    void processNextReq();

private slots:
    void onProcessFinished();

private:
    QQueue<QString> m_queuedRequests;
    QString m_cacheDir;
    QFutureWatcher<QPixmap> m_futureWatcher;
    qreal m_scale;
};

#endif // THUMBNAILMANAGER_H
