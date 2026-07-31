// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicemountsubscriber.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_log_defines.h>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

DeviceMountSubscriber *DeviceMountSubscriber::instance()
{
    static DeviceMountSubscriber instance;
    return &instance;
}

DeviceMountSubscriber::DeviceMountSubscriber(QObject *parent)
    : QObject(parent)
{
    connect(&cleanupTimer, &QTimer::timeout, this, &DeviceMountSubscriber::cleanupExpiredSubscriptions);
    cleanupTimer.start(60000);   // check once per minute
    fmDebug() << "DeviceMountSubscriber initialized";
}

int DeviceMountSubscriber::subscribe(const QUrl &deviceUrl, std::function<void(const QUrl &)> callback)
{
    if (!deviceUrl.isValid()) {
        fmWarning() << "DeviceMountSubscriber: Cannot subscribe to invalid URL";
        return -1;
    }

    int id = nextSubscriptionId++;

    Subscription sub;
    sub.id = id;
    sub.deviceUrl = deviceUrl;
    sub.callback = callback;
    sub.timestamp = QDateTime::currentDateTime();

    subscriptions.insert(id, sub);

    fmDebug() << "DeviceMountSubscriber: Subscribed to" << deviceUrl << "with ID" << id
              << "- active subscriptions:" << subscriptions.size();

    return id;
}

void DeviceMountSubscriber::unsubscribe(int subscriptionId)
{
    if (subscriptions.contains(subscriptionId)) {
        QUrl url = subscriptions[subscriptionId].deviceUrl;
        fmDebug() << "DeviceMountSubscriber: Unsubscribed ID" << subscriptionId
                  << "for device" << url
                  << "- remaining subscriptions:" << (subscriptions.size() - 1);
        subscriptions.remove(subscriptionId);
    } else {
        fmWarning() << "DeviceMountSubscriber: Attempted to unsubscribe non-existent ID" << subscriptionId;
    }
}

void DeviceMountSubscriber::notifyMountFinished(const QUrl &deviceUrl, const QUrl &mountedUrl)
{
    if (!deviceUrl.isValid() || !mountedUrl.isValid()) {
        fmWarning() << "DeviceMountSubscriber: Invalid URL in notifyMountFinished"
                   << "deviceUrl:" << deviceUrl << "mountedUrl:" << mountedUrl;
        return;
    }

    fmDebug() << "DeviceMountSubscriber: Device mounted" << deviceUrl << "at" << mountedUrl
              << "- checking" << subscriptions.size() << "subscriptions";

    QList<int> matchedIds;

    for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it) {
        if (UniversalUtils::urlEquals(it.value().deviceUrl, deviceUrl)) {
            fmDebug() << "DeviceMountSubscriber: Found matching subscription ID" << it.key()
                     << "for device" << deviceUrl;

            try {
                fmDebug() << "DeviceMountSubscriber: Executing callback for ID" << it.key();
                it.value().callback(mountedUrl);
            } catch (const std::exception &e) {
                fmWarning() << "DeviceMountSubscriber: Exception in callback:" << e.what();
            } catch (...) {
                fmWarning() << "DeviceMountSubscriber: Unknown exception in callback";
            }

            matchedIds.append(it.key());
        }
    }

    for (int id : matchedIds) {
        fmDebug() << "DeviceMountSubscriber: Removing completed subscription ID" << id;
        subscriptions.remove(id);
    }

    if (matchedIds.isEmpty()) {
        fmDebug() << "DeviceMountSubscriber: No matching subscriptions found for device:" << deviceUrl;
    } else {
        fmDebug() << "DeviceMountSubscriber: Processed" << matchedIds.size()
                 << "subscriptions for device:" << deviceUrl
                 << "- remaining subscriptions:" << subscriptions.size();
    }
}

void DeviceMountSubscriber::cleanupExpiredSubscriptions()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<int> expiredIds;

    for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it) {
        int ageInSeconds = it.value().timestamp.secsTo(now);
        if (ageInSeconds > 600) {   // 10 minutes
            expiredIds.append(it.key());
            fmDebug() << "DeviceMountSubscriber: Subscription ID" << it.key()
                     << "expired after" << ageInSeconds << "seconds";
        }
    }

    for (int id : expiredIds) {
        fmDebug() << "DeviceMountSubscriber: Removing expired subscription ID" << id
                 << "for device" << subscriptions[id].deviceUrl;
        subscriptions.remove(id);
    }

    if (!expiredIds.isEmpty()) {
        fmDebug() << "DeviceMountSubscriber: Cleaned up" << expiredIds.size()
                 << "expired subscriptions - remaining:" << subscriptions.size();
    }
}
