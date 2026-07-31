// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEMOUNTSUBSCRIBER_H
#define DEVICEMOUNTSUBSCRIBER_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QTimer>
#include <QDateTime>
#include <functional>

DPSIDEBAR_BEGIN_NAMESPACE

/**
 * @brief Device mount event subscription manager.
 *
 * Implements a publish-subscribe pattern to handle callbacks after a device
 * mount completes. Used by the sidebar to auto-expand a partition once its
 * underlying device is mounted.
 */
class DeviceMountSubscriber : public QObject
{
    Q_OBJECT
public:
    static DeviceMountSubscriber *instance();

    int subscribe(const QUrl &deviceUrl, std::function<void(const QUrl &)> callback);
    void unsubscribe(int subscriptionId);
    void notifyMountFinished(const QUrl &deviceUrl, const QUrl &mountedUrl);

private:
    explicit DeviceMountSubscriber(QObject *parent = nullptr);

    struct Subscription {
        int id;
        QUrl deviceUrl;
        std::function<void(const QUrl &)> callback;
        QDateTime timestamp;
    };

    QMap<int, Subscription> subscriptions;
    int nextSubscriptionId = 0;
    QTimer cleanupTimer;

    void cleanupExpiredSubscriptions();
};

DPSIDEBAR_END_NAMESPACE

#endif   // DEVICEMOUNTSUBSCRIBER_H
