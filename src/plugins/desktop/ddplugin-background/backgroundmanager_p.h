// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDMANAGER_P_H
#define BACKGROUNDMANAGER_P_H

#include "backgroundmanager.h"
#include "backgroundservice.h"

#include <QObject>

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundBridge : public QObject
{
    Q_OBJECT
public:
    struct Requestion
    {
        QString screen;
        QString path;
        QSize size;
        QPixmap pixmap;
    };

public:
    explicit BackgroundBridge(class BackgroundManagerPrivate *ptr);
    ~BackgroundBridge();

public:
    inline bool isRunning() const
    {
        return future.isRunning();
    }
    inline bool isForce() const
    {
        return force;
    }
    inline void setRepeat()
    {
        repeat = true;
    }
    void request(bool refresh);
    void forceRequest();
    void terminate(bool wait);
    Q_INVOKABLE void onFinished(void *pData);
    static QPixmap getPixmap(const QString &path, const QSize &targetSize, const QPixmap &defaultPixmap = QPixmap());

private:
    void queryCacheAndClassify(Requestion &req, QList<Requestion> &cachedRequests, QList<Requestion> &uncachedRequests);
    void processRequests(const QList<Requestion> &cachedRequests, const QList<Requestion> &uncachedRequests, bool forceMode = false);
    static void runUpdate(BackgroundBridge *self, QList<Requestion> reqs);

private:
    class BackgroundManagerPrivate *d = nullptr;
    volatile bool getting = false;
    volatile bool force = false;
    QFuture<void> future;
    bool repeat = false;
};

class BackgroundManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManagerPrivate(BackgroundManager *qq);
    ~BackgroundManagerPrivate();
    bool isEnableBackground();

    inline QRect relativeGeometry(const QRect &geometry)
    {
        return QRect(QPoint(0, 0), geometry.size());
    }

public:
    BackgroundManager *const q = nullptr;
    BackgroundService *service = nullptr;
    BackgroundBridge *bridge = nullptr;
    QMap<QString, BackgroundWidgetPointer> backgroundWidgets;
    QMap<QString, QString> backgroundPaths;
    bool enableBackground = true;
};

DDP_BACKGROUND_END_NAMESPACE

#endif   // BACKGROUNDMANAGER_P_H
