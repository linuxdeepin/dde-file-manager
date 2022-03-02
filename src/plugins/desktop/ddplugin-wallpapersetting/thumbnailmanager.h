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
#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include "ddplugin_wallpapersetting_global.h"

#include <QObject>
#include <QQueue>
#include <QFutureWatcher>
#include <QPixmap>

DDP_WALLPAERSETTING_BEGIN_NAMESPACE

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(qreal scale, QObject *parent = nullptr);
    ~ThumbnailManager();
    static ThumbnailManager* instance(qreal scale);
    void find(const QString & key);
    void stop();
protected:
    bool replace(const QString & key, const QPixmap & pixmap);
    static QPixmap thumbnailImage(const QString &key, qreal scale);
signals:
    void thumbnailFounded(const QString &key, const QPixmap &pixmap);
    void findAborted(QQueue<QString> queue);

public slots:
private slots:
    void onProcessFinished();
private:
    void processNextReq();
private:
    qreal scale;
    QString cacheDir;
    QFutureWatcher<QPixmap> futureWatcher;
    QQueue<QString> queuedRequests;
};

DDP_WALLPAERSETTING_END_NAMESPACE

#endif // THUMBNAILMANAGER_H
