/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "vaultdbusutils.h"

#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusUnixFileDescriptor>
#include <DDBusSender>
#include <QDebug>

DPVAULT_USE_NAMESPACE

QVariant VaultDBusUtils::vaultManagerDBusCall(QString function, const QVariant &vaule)
{
    QVariant value;
    QDBusInterface sessionManagerIface("com.deepin.filemanager.service",
                                       "/com/deepin/filemanager/service/VaultManager",
                                       "com.deepin.filemanager.service.VaultManager",
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
