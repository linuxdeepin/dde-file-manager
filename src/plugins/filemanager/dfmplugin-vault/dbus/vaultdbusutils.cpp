// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultdbusutils.h"

#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusUnixFileDescriptor>
#include <DDBusSender>
#include <QDBusConnectionInterface>
#include <QDebug>

#include <unistd.h>

using namespace dfmplugin_vault;

QVariant VaultDBusUtils::vaultManagerDBusCall(QString function, const QVariant &vaule)
{
    QVariant value;
    QDBusInterface sessionManagerIface("org.deepin.filemanager.server",
                                       "/org/deepin/filemanager/server/VaultManager",
                                       "org.deepin.filemanager.server.VaultManager",
                                       QDBusConnection::sessionBus());

    if (sessionManagerIface.isValid()) {

        if (vaule.isNull()) {
            QDBusPendingCall call = sessionManagerIface.asyncCall(function);
            call.waitForFinished();
            qInfo() << call.error().message();
            if (!call.isError()) {
                QDBusReply<quint64> reply = call.reply();
                value = QVariant::fromValue(reply.value());
            }
        } else {
            QDBusPendingCall call = sessionManagerIface.asyncCall(function, vaule);
            call.waitForFinished();
            if (call.isError()) {
                value = call.error().message();
            }
        }
    }
    return value;
}

VaultPolicyState VaultDBusUtils::getVaultPolicy()
{
    if (!isServiceRegister(QDBusConnection::SystemBus, kDeamonServiceName))
        return VaultPolicyState::kEnable;

    QDBusInterface deepinSystemInfo(kDeamonServiceName,
                                    "/com/deepin/filemanager/daemon/AccessControlManager",
                                    "com.deepin.filemanager.daemon.AccessControlManager",
                                    QDBusConnection::systemBus());

    VaultPolicyState vaulthidestate { VaultPolicyState::kUnkonw };

    //调用
    auto response = deepinSystemInfo.call("QueryVaultAccessPolicyVisible");
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        QVariantList value = response.arguments();
        if (!value.isEmpty()) {
            QVariant varVaule = value.first();
            vaulthidestate = static_cast<VaultPolicyState>(varVaule.toInt());
        }
    } else {
        qInfo() << "Vault error: value method called failed!";
    }

    return vaulthidestate;
}

bool VaultDBusUtils::setVaultPolicyState(int policyState)
{
    if (!isServiceRegister(QDBusConnection::SystemBus, kDeamonServiceName))
        return false;

    QDBusInterface deepinSystemInfo(kDeamonServiceName,
                                    "/com/deepin/filemanager/daemon/AccessControlManager",
                                    "com.deepin.filemanager.daemon.AccessControlManager",
                                    QDBusConnection::systemBus());

    auto response = deepinSystemInfo.call("FileManagerReply", QVariant::fromValue(policyState));
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        QVariantList value = response.arguments();
        if (!value.isEmpty()) {
            QVariant varVaule = value.first();
            if (!varVaule.toString().isEmpty()) {
                return true;
            }
        } else {
            return false;
        }

    } else {
        qDebug() << "value method called failed!";
        return false;
    }

    return false;
}

void VaultDBusUtils::lockEventTriggered(QObject *obj, const char *cslot)
{
    QDBusConnection::sessionBus().connect(
            "org.deepin.filemanager.server",
            "/org/deepin/filemanager/server/VaultManager",
            "org.deepin.filemanager.server.VaultManager",
            "LockEventTriggered",
            obj,
            cslot);
}

int VaultDBusUtils::getLeftoverErrorInputTimes()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.server",
                                    "/org/deepin/filemanager/server/VaultManager",
                                    "org.deepin.filemanager.server.VaultManager",
                                    QDBusConnection::sessionBus());

    int leftChance = -1;
    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<int> reply = VaultManagerdbus.call("GetLeftoverErrorInputTimes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError()) {
            qInfo() << "Warning: Obtaining the remaining number of password input errors!" << reply.error().message();
        } else {
            leftChance = reply.value();
        }
    }

    return leftChance;
}

void VaultDBusUtils::leftoverErrorInputTimesMinusOne()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.server",
                                    "/org/deepin/filemanager/server/VaultManager",
                                    "org.deepin.filemanager.server.VaultManager",
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("LeftoverErrorInputTimesMinusOne", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: The remaining password input times minus 1 is wrong!" << reply.error().message();
    }
}

void VaultDBusUtils::startTimerOfRestorePasswordInput()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.server",
                                    "/org/deepin/filemanager/server/VaultManager",
                                    "org.deepin.filemanager.server.VaultManager",
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("StartTimerOfRestorePasswordInput", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Error when opening the password input timer!" << reply.error().message();
    }
}

int VaultDBusUtils::getNeedWaitMinutes()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.server",
                                    "/org/deepin/filemanager/server/VaultManager",
                                    "org.deepin.filemanager.server.VaultManager",
                                    QDBusConnection::sessionBus());

    int result = 100;
    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<int> reply = VaultManagerdbus.call("GetNeedWaitMinutes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError()) {
            qInfo() << "Warning: Failed to get the number of minutes to wait!" << reply.error().message();
        } else {
            result = reply.value();
        }
    }
    return result;
}

void VaultDBusUtils::restoreNeedWaitMinutes()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.server",
                                    "/org/deepin/filemanager/server/VaultManager",
                                    "org.deepin.filemanager.server.VaultManager",
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("RestoreNeedWaitMinutes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Error when opening the password input timer!" << reply.error().message();
    }
}

void VaultDBusUtils::restoreLeftoverErrorInputTimes()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.server",
                                    "/org/deepin/filemanager/server/VaultManager",
                                    "org.deepin.filemanager.server.VaultManager",
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("RestoreLeftoverErrorInputTimes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Error in restoring the remaining number of incorrect entries!" << reply.error().message();
    }
}

bool VaultDBusUtils::isServiceRegister(QDBusConnection::BusType type, const QString &serviceName)
{
    QDBusConnectionInterface *interface { nullptr };
    switch (type) {
    case QDBusConnection::SystemBus:
        interface = QDBusConnection::systemBus().interface();
        break;
    case QDBusConnection::SessionBus:
        interface = QDBusConnection::sessionBus().interface();
        break;
    default:
        break;
    }
    if (!interface) {
        qWarning() << "Vault error: dbus is not available.";
        return false;
    }

    if (!interface->isServiceRegistered(serviceName)) {
        qWarning() << "Vault error: service is not registered";
        return false;
    }

    return true;
}
