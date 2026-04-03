// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalshareproxy.h"

#include "global_server_defines.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusArgument>

namespace dfmbase {

using namespace GlobalServerDefines;

OpticalShareProxy &OpticalShareProxy::instance()
{
    static OpticalShareProxy ins;
    return ins;
}

QVariantMap OpticalShareProxy::burnState(const QString &dev)
{
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
    QDBusReply<QVariantMap> reply = dbusInterface()->call("GetBurnStates");
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::burnStates: invalid reply, error:" << reply.error().message();
        return {};
    }

    QVariantMap result;
    const QVariantMap rawMap = reply.value();

    // 遍历返回的 map，将嵌套的 QDBusArgument 解包为 QVariantMap
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

bool OpticalShareProxy::setBurnState(const QString &dev, const QVariantMap &state)
{
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
    QDBusReply<bool> reply = dbusInterface()->call("ClearBurnState", dev);
    if (!reply.isValid()) {
        qCWarning(logDFMBase) << "OpticalShareProxy::clearBurnState: invalid reply for device:" << dev
                              << ", error:" << reply.error().message();
        return false;
    }

    return reply.value();
}

QVariantMap OpticalShareProxy::burnAttribute(const QString &tag)
{
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

OpticalShareProxy::OpticalShareProxy(QObject *parent)
    : QObject(parent)
{
    auto bus = QDBusConnection::systemBus();
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
                                           QDBusConnection::systemBus(),
                                           this));
        interface->setTimeout(3000);
    }

    return interface.data();
}

}   // namespace dfmbase
