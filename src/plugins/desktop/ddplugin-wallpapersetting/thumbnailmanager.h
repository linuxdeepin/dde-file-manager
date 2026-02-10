// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include "ddplugin_wallpapersetting_global.h"

#include <QObject>
#include <QQueue>
#include <QFutureWatcher>
#include <QPixmap>

namespace ddplugin_wallpapersetting {

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

}

#endif // THUMBNAILMANAGER_H
