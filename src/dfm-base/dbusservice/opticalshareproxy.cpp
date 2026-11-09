// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalshareproxy.h"

#include "global_server_defines.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusServiceWatcher>

namespace dfmbase {

using namespace GlobalServerDefines;

namespace {
// 服务在位时同步调用为一问一答，耗时可忽略；超时仅作慢应答场景的兜底，
// 取较小值避免主线程被拖住
constexpr int kSyncCallTimeoutMs { 100 };
// 异步刷新不在主线程等待，允许较宽超时以覆盖冷启动后首次应答偏慢的情况
constexpr int kAsyncCallTimeoutMs { 1000 };

bool serviceRegistered()
{
    // NameHasOwner 仅查询总线缓存，不触发目标服务的 systemd 激活，
    // 因此可以在主线程安全调用；登录早期会话总线可能尚未就绪，
    // interface() 可能返回空指针，按防御范式判空后视为未注册
    QDBusConnectionInterface *iface = QDBusConnection::sessionBus().interface();
    return iface && iface->isServiceRegistered(OpticalShareDBusInfo::kService);
}

QVariantMap unpackStatesMap(const QVariantMap &rawMap)
{
    QVariantMap result;
    for (auto iter = rawMap.cbegin(); iter != rawMap.cend(); ++iter) {
        const QVariant &value = iter.value();
        if (value.canConvert<QDBusArgument>()) {
            QDBusArgument arg = value.value<QDBusArgument>();
            QVariantMap nestedMap;
            arg >> nestedMap;
            result.insert(iter.key(), nestedMap);
        } else if (value.canConvert<QVariantMap>()) {
            result.insert(iter.key(), value.toMap());
        } else {
            result.insert(iter.key(), value);
        }
    }
    return result;
}
}   // namespace

OpticalShareProxy &OpticalShareProxy::instance()
{
    static OpticalShareProxy ins;
    return ins;
}

QVariantMap OpticalShareProxy::burnState(const QString &dev)
{
    if (!guardServiceReady("burnState"))
        return {};

    QDBusReply<QVariantMap> reply = dbusInterface()->call("GetBurnState", dev);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::burnState: invalid reply for device:" << dev
                              << ", error:" << reply.error().message();
        return {};
    }

    return reply.value();
}

QVariantMap OpticalShareProxy::burnStates()
{
    if (!guardServiceReady("burnStates"))
        return {};

    QDBusReply<QVariantMap> reply = dbusInterface()->call("GetBurnStates");
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::burnStates: invalid reply, error:" << reply.error().message();
        return {};
    }

    return unpackStatesMap(reply.value());
}

bool OpticalShareProxy::setBurnState(const QString &dev, const QVariantMap &state)
{
    if (!guardServiceReady("setBurnState"))
        return false;

    QDBusReply<bool> reply = dbusInterface()->call("SetBurnState", dev, state);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::setBurnState: invalid reply for device:" << dev
                              << ", error:" << reply.error().message();
        return false;
    }

    return reply.value();
}

bool OpticalShareProxy::clearBurnState(const QString &dev)
{
    if (!guardServiceReady("clearBurnState"))
        return false;

    QDBusReply<bool> reply = dbusInterface()->call("ClearBurnState", dev);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::clearBurnState: invalid reply for device:" << dev
                              << ", error:" << reply.error().message();
        return false;
    }

    return reply.value();
}


QDBusPendingCall OpticalShareProxy::burnStatesAsync()
{
    if (!guardServiceReady("burnStatesAsync"))
        // 无效 pending call 立即结束，调用方（如 Burn::start 的 watcher）按失败应答处理
        return QDBusPendingCall(QDBusPendingReply<QVariantMap>());

    return dbusInterface()->asyncCall("GetBurnStates");
}

QDBusPendingCall OpticalShareProxy::clearBurnStateAsync(const QString &dev)
{
    if (!guardServiceReady("clearBurnStateAsync"))
        return QDBusPendingCall(QDBusPendingReply<bool>());

    return dbusInterface()->asyncCall("ClearBurnState", dev);
}

QVariantMap OpticalShareProxy::burnAttribute(const QString &tag)
{
    if (!guardServiceReady("burnAttribute"))
        return {};

    QDBusReply<QVariantMap> reply = dbusInterface()->call("GetBurnAttribute", tag);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::burnAttribute: invalid reply for tag:" << tag
                              << ", error:" << reply.error().message();
        return {};
    }

    return reply.value();
}

bool OpticalShareProxy::setBurnAttribute(const QString &tag, const QVariantMap &attribute)
{
    if (!guardServiceReady("setBurnAttribute"))
        return false;

    QDBusReply<bool> reply = dbusInterface()->call("SetBurnAttribute", tag, attribute);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::setBurnAttribute: invalid reply for tag:" << tag
                              << ", error:" << reply.error().message();
        return false;
    }

    return reply.value();
}

bool OpticalShareProxy::clearBurnAttribute(const QString &tag)
{
    if (!guardServiceReady("clearBurnAttribute"))
        return false;

    QDBusReply<bool> reply = dbusInterface()->call("ClearBurnAttribute", tag);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::clearBurnAttribute: invalid reply for tag:" << tag
                              << ", error:" << reply.error().message();
        return false;
    }

    return reply.value();
}

void OpticalShareProxy::onBurnStateChanged(const QString &dev, const QVariantMap &state)
{
    emit burnStateChanged(dev, state);
}

void OpticalShareProxy::onBurnAttributeChanged(const QString &tag, const QVariantMap &attribute)
{
    emit burnAttributeChanged(tag, attribute);
}

void OpticalShareProxy::onServiceRegisteredAsync(const QString &serviceName)
{
    qCWarning(logDFMBase) << "OpticalShareProxy: service appeared:" << serviceName
                          << ", refresh burn states asynchronously";
    refreshBurnStatesAsync();
}

OpticalShareProxy::OpticalShareProxy(QObject *parent)
    : QObject(parent)
{
    auto bus = QDBusConnection::sessionBus();
    bus.connect(OpticalShareDBusInfo::kService,
                OpticalShareDBusInfo::kPath,
                OpticalShareDBusInfo::kInterface,
                "BurnStateChanged",
                this,
                SLOT(onBurnStateChanged(QString,QVariantMap)));
    bus.connect(OpticalShareDBusInfo::kService,
                OpticalShareDBusInfo::kPath,
                OpticalShareDBusInfo::kInterface,
                "BurnAttributeChanged",
                this,
                SLOT(onBurnAttributeChanged(QString,QVariantMap)));
}

QDBusInterface *OpticalShareProxy::dbusInterface()
{
    if (!interface || !interface->isValid()) {
        interface.reset(new QDBusInterface(OpticalShareDBusInfo::kService,
                                           OpticalShareDBusInfo::kPath,
                                           OpticalShareDBusInfo::kInterface,
                                           QDBusConnection::sessionBus(),
                                           this));
        interface->setTimeout(kSyncCallTimeoutMs);
    }

    return interface.data();
}

bool OpticalShareProxy::guardServiceReady(const char *method)
{
    if (serviceRegistered())
        return true;

    qCWarning(logDFMBase) << "OpticalShareProxy:" << method
                          << ": service not registered, skip sync call:" << OpticalShareDBusInfo::kService;

    // 建立服务上线监听（幂等）：注册后由 watcher 触发一次异步全量刷新，
    // 调用方在服务缺席期间拿到空结果，与原先“调用失败返回空”的语义一致
    if (!serviceWatcher) {
        serviceWatcher = new QDBusServiceWatcher(OpticalShareDBusInfo::kService,
                                                 QDBusConnection::sessionBus(),
                                                 QDBusServiceWatcher::WatchForRegistration,
                                                 this);
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered,
                this, &OpticalShareProxy::onServiceRegisteredAsync);
    }
    return false;
}

void OpticalShareProxy::refreshBurnStatesAsync()
{
    if (asyncRefreshRunning)
        return;
    asyncRefreshRunning = true;

    QDBusMessage msg = QDBusMessage::createMethodCall(OpticalShareDBusInfo::kService,
                                                      OpticalShareDBusInfo::kPath,
                                                      OpticalShareDBusInfo::kInterface,
                                                      "GetBurnStates");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg, kAsyncCallTimeoutMs);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        asyncRefreshRunning = false;

        QDBusPendingReply<QVariantMap> reply = *w;
        if (reply.isError()) {
            qCWarning(logDFMBase) << "OpticalShareProxy: async refresh failed:" << reply.error().message();
            return;
        }

        const QVariantMap states = unpackStatesMap(reply.value());
        for (auto iter = states.cbegin(); iter != states.cend(); ++iter)
            emit burnStateChanged(iter.key(), iter.value().toMap());
    });
}

}   // namespace dfmbase
