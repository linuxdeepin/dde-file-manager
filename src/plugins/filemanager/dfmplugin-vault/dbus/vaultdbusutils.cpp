// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultdbusutils.h"

#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusUnixFileDescriptor>
#include <DDBusSender>
#include <QDebug>

#include <unistd.h>

using namespace dfmplugin_vault;

QVariant VaultDBusUtils::vaultManagerDBusCall(QString function, const QVariant &vaule)
{
    QVariant value;
    QDBusInterface sessionManagerIface("org.deepin.filemanager.service",
                                       "/org/deepin/filemanager/service/VaultManager",
                                       "org.deepin.filemanager.service.VaultManager",
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

int VaultDBusUtils::getVaultPolicy()
{
    QDBusInterface deepinSystemInfo("com.deepin.filemanager.daemon",
                                    "/com/deepin/filemanager/daemon/AccessControlManager",
                                    "com.deepin.filemanager.daemon.AccessControlManager",
                                    QDBusConnection::systemBus());

    int vaulthidestate = -1;

    //调用
    auto response = deepinSystemInfo.call("QueryVaultAccessPolicyVisible");
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        QVariantList value = response.arguments();
        if (!value.isEmpty()) {
            QVariant varVaule = value.first();
            vaulthidestate = varVaule.toInt();
        } else {
            vaulthidestate = -1;
        }

    } else {
        qDebug() << "value method called failed!";
        vaulthidestate = -1;
    }

    return vaulthidestate;
}

bool VaultDBusUtils::setVaultPolicyState(int policyState)
{
    QDBusInterface deepinSystemInfo("com.deepin.filemanager.daemon",
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
            "org.deepin.filemanager.service",
            "/org/deepin/filemanager/service/VaultManager",
            "org.deepin.filemanager.service.VaultManager",
            "lockEventTriggered",
            obj,
            cslot);
}

int VaultDBusUtils::getLeftoverErrorInputTimes()
{
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.service",
                                    "/org/deepin/filemanager/service/VaultManager",
                                    "org.deepin.filemanager.service.VaultManager",
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
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.service",
                                    "/org/deepin/filemanager/service/VaultManager",
                                    "org.deepin.filemanager.service.VaultManager",
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
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.service",
                                    "/org/deepin/filemanager/service/VaultManager",
                                    "org.deepin.filemanager.service.VaultManager",
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
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.service",
                                    "/org/deepin/filemanager/service/VaultManager",
                                    "org.deepin.filemanager.service.VaultManager",
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
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.service",
                                    "/org/deepin/filemanager/service/VaultManager",
                                    "org.deepin.filemanager.service.VaultManager",
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
    QDBusInterface VaultManagerdbus("org.deepin.filemanager.service",
                                    "/org/deepin/filemanager/service/VaultManager",
                                    "org.deepin.filemanager.service.VaultManager",
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("RestoreLeftoverErrorInputTimes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            qInfo() << "Warning: Error in restoring the remaining number of incorrect entries!" << reply.error().message();
    }
}
