// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALSHAREPROXY_H
#define OPTICALSHAREPROXY_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QScopedPointer>
#include <QVariantMap>
#include <QDBusInterface>
#include <QDBusPendingCall>

class QDBusServiceWatcher;

namespace dfmbase {

class OpticalShareProxy : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpticalShareProxy)

public:
    static OpticalShareProxy &instance();

    QVariantMap burnState(const QString &dev);
    QVariantMap burnStates();
    bool setBurnState(const QString &dev, const QVariantMap &state);
    bool clearBurnState(const QString &dev);

    QDBusPendingCall burnStatesAsync();
    QDBusPendingCall clearBurnStateAsync(const QString &dev);

    QVariantMap burnAttribute(const QString &tag);
    bool setBurnAttribute(const QString &tag, const QVariantMap &attribute);
    bool clearBurnAttribute(const QString &tag);

Q_SIGNALS:
    void burnStateChanged(const QString &dev, const QVariantMap &state);
    void burnAttributeChanged(const QString &tag, const QVariantMap &attribute);

private Q_SLOTS:
    void onBurnStateChanged(const QString &dev, const QVariantMap &state);
    void onBurnAttributeChanged(const QString &tag, const QVariantMap &attribute);
    void onServiceRegisteredAsync(const QString &serviceName);

private:
    explicit OpticalShareProxy(QObject *parent = nullptr);

    QDBusInterface *dbusInterface();

    // 服务未注册时跳过同步调用并返回 false；同时确保服务上线监听已建立
    bool guardServiceReady(const char *method);
    // 服务上线后的异步全量刷新，逐设备补发 burnStateChanged
    void refreshBurnStatesAsync();

private:
    QScopedPointer<QDBusInterface> interface;
    QDBusServiceWatcher *serviceWatcher { nullptr };
    bool asyncRefreshRunning { false };
};

}   // namespace dfmbase

#endif   // OPTICALSHAREPROXY_H
