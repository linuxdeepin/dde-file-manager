// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultdbusutils.h"
#include "utils/pathmanager.h"
#include "utils/vaulthelper.h"

#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusUnixFileDescriptor>
#include <DDBusSender>
#include <QDBusConnectionInterface>
#include <QDebug>

#include <unistd.h>

inline constexpr int kArgumentsNum { 3 };

using namespace dfmplugin_vault;

VaultDBusUtils *VaultDBusUtils::instance()
{
    static VaultDBusUtils ins;
    return &ins;
}

QVariant VaultDBusUtils::vaultManagerDBusCall(QString function, const QVariant &value)
{
    fmDebug() << "Vault: Calling vault manager DBus method:" << function << "with value:" << value;

    QVariant result;
    QDBusInterface sessionManagerIface(kFileManagerDBusDaemonName,
                                       kFileManagerVaultDBusPath,
                                       kFileManagerVaultDBusInterfaces,
                                       QDBusConnection::sessionBus());

    if (sessionManagerIface.isValid()) {
        fmDebug() << "Vault: DBus interface is valid, proceeding with call";

        if (value.isNull()) {
            QDBusPendingCall call = sessionManagerIface.asyncCall(function);
            call.waitForFinished();
            if (!call.isError()) {
                QDBusReply<quint64> reply = call.reply();
                result = QVariant::fromValue(reply.value());
                fmDebug() << "Vault: DBus call successful, returned value:" << result;
            } else {
                fmWarning() << "Vault: DBus call failed for method" << function << "error:" << call.error().message();
            }
        } else {
            QDBusPendingCall call = sessionManagerIface.asyncCall(function, value);
            call.waitForFinished();
            if (call.isError()) {
                result = call.error().message();
                fmWarning() << "Vault: DBus call failed for method" << function << "error:" << call.error().message();
            } else {
                fmDebug() << "Vault: DBus call successful for method" << function;
            }
        }
    } else {
        fmWarning() << "Vault: DBus interface is not valid for vault manager";
    }

    return result;
}

void VaultDBusUtils::lockEventTriggered(QObject *obj, const char *cslot)
{
    QDBusConnection::sessionBus().connect(
            kFileManagerDBusDaemonName,
            kFileManagerVaultDBusPath,
            kFileManagerVaultDBusInterfaces,
            "LockEventTriggered",
            obj,
            cslot);
}

int VaultDBusUtils::getLeftoverErrorInputTimes()
{
    QDBusInterface VaultManagerdbus(kFileManagerDBusDaemonName,
                                    kFileManagerVaultDBusPath,
                                    kFileManagerVaultDBusInterfaces,
                                    QDBusConnection::sessionBus());

    int leftChance = -1;
    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<int> reply = VaultManagerdbus.call("GetLeftoverErrorInputTimes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError()) {
            fmWarning() << "Vault: dbus method(GetLeftoverErrorInputTimes) called failed! the error is: " << reply.error().message();
        } else {
            leftChance = reply.value();
            fmDebug() << "Vault: Retrieved leftover error input times:" << leftChance;
        }
    } else {
        fmWarning() << "Vault: DBus interface is not valid for getting leftover error input times";
    }

    return leftChance;
}

void VaultDBusUtils::leftoverErrorInputTimesMinusOne()
{
    QDBusInterface VaultManagerdbus(kFileManagerDBusDaemonName,
                                    kFileManagerVaultDBusPath,
                                    kFileManagerVaultDBusInterfaces,
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("LeftoverErrorInputTimesMinusOne", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            fmWarning() << "Vault: dbus method(LeftoverErrorInputTimesMinusOne) called failed! the error is: " << reply.error().message();
    } else {
        fmWarning() << "Vault: DBus interface is not valid for decreasing leftover error input times";
    }
}

void VaultDBusUtils::startTimerOfRestorePasswordInput()
{
    QDBusInterface VaultManagerdbus(kFileManagerDBusDaemonName,
                                    kFileManagerVaultDBusPath,
                                    kFileManagerVaultDBusInterfaces,
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("StartTimerOfRestorePasswordInput", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            fmWarning() << "Vault: dbus method(StartTimerOfRestorePasswordInput) called failed! the error is: " << reply.error().message();
    } else {
        fmWarning() << "Vault: DBus interface is not valid for starting timer of restore password input";
    }
}

int VaultDBusUtils::getNeedWaitMinutes()
{
    QDBusInterface VaultManagerdbus(kFileManagerDBusDaemonName,
                                    kFileManagerVaultDBusPath,
                                    kFileManagerVaultDBusInterfaces,
                                    QDBusConnection::sessionBus());

    int result = 100;
    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<int> reply = VaultManagerdbus.call("GetNeedWaitMinutes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError()) {
            fmWarning() << "Vault: failed to get the number of minutes to wait! the error is: " << reply.error().message();
        } else {
            result = reply.value();
            fmDebug() << "Vault: Retrieved need wait minutes:" << result;
        }
    } else {
        fmWarning() << "Vault: DBus interface is not valid for getting need wait minutes";
    }
    return result;
}

void VaultDBusUtils::restoreNeedWaitMinutes()
{
    QDBusInterface VaultManagerdbus(kFileManagerDBusDaemonName,
                                    kFileManagerVaultDBusPath,
                                    kFileManagerVaultDBusInterfaces,
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("RestoreNeedWaitMinutes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            fmWarning() << "Vault: Error when opening the password input timer! the error is: " << reply.error().message();
    } else {
        fmWarning() << "Vault: DBus interface is not valid for restoring need wait minutes";
    }
}

void VaultDBusUtils::restoreLeftoverErrorInputTimes()
{
    QDBusInterface VaultManagerdbus(kFileManagerDBusDaemonName,
                                    kFileManagerVaultDBusPath,
                                    kFileManagerVaultDBusInterfaces,
                                    QDBusConnection::sessionBus());

    if (VaultManagerdbus.isValid()) {
        QDBusPendingReply<> reply = VaultManagerdbus.call("RestoreLeftoverErrorInputTimes", QVariant::fromValue(int(getuid())));
        reply.waitForFinished();
        if (reply.isError())
            fmWarning() << "Vault: Error in restoring the remaining number of incorrect entries! the error is: " << reply.error().message();
    } else {
        fmWarning() << "Vault: DBus interface is not valid for restoring leftover error input times";
    }
}

bool VaultDBusUtils::isServiceRegister(QDBusConnection::BusType type, const QString &serviceName)
{
    fmDebug() << "Vault: Checking service registration for service:" << serviceName << "on bus type:" << type;

    QDBusConnectionInterface *interface { nullptr };
    switch (type) {
    case QDBusConnection::SystemBus:
        interface = QDBusConnection::systemBus().interface();
        fmDebug() << "Vault: Using system bus for service check";
        break;
    case QDBusConnection::SessionBus:
        interface = QDBusConnection::sessionBus().interface();
        fmDebug() << "Vault: Using session bus for service check";
        break;
    default:
        fmWarning() << "Vault: Unknown bus type:" << type;
        break;
    }
    if (!interface) {
        fmCritical() << "Vault: dbus is not available.";
        return false;
    }

    if (!interface->isServiceRegistered(serviceName)) {
        fmCritical() << "Vault: service is not registered";
        return false;
    }

    fmDebug() << "Vault: Service" << serviceName << "is registered";
    return true;
}

bool VaultDBusUtils::isFullConnectInternet()
{
    fmDebug() << "Vault: Checking full internet connectivity";

    QDBusInterface netWorkInfo(kNetWorkDBusServiceName,
                               kNetWorkDBusPath,
                               kNetWorkDBusInterfaces,
                               QDBusConnection::systemBus());

    Connectivity netState { Connectivity::Unknownconnectivity };

    QVariant reply = netWorkInfo.property("Connectivity");
    if (reply.isValid()) {
        int replyVault = reply.toInt();
        fmInfo() << "Get network value from dbus, the value is " << replyVault;
        netState = static_cast<Connectivity>(replyVault);
    } else {
        fmWarning() << "Dbus call failed, the dbus interfaces is " << kNetWorkDBusInterfaces;
    }

    bool isFullConnected = (netState == Connectivity::Full);
    fmDebug() << "Vault: Internet connectivity check result:" << isFullConnected;

    return isFullConnected;
}

void VaultDBusUtils::handleChangedVaultState(const QVariantMap &map)
{
    fmDebug() << "Vault: Handling changed vault state, map size:" << map.size();

    QVariantMap::const_iterator it = map.constBegin();
    for (; it != map.constEnd(); ++it) {
        if (it.key() == PathManager::vaultUnlockPath() && it.value().toInt() == static_cast<int>(VaultState::kEncrypted)) {
            fmInfo() << "Vault: Updating vault state to encrypted";
            VaultHelper::instance()->updateState(VaultState::kEncrypted);
        }
    }
}

void VaultDBusUtils::handleLockScreenDBus(const QDBusMessage &msg)
{
    fmDebug() << "Vault: Handling lock screen DBus message";

    const QList<QVariant> &arguments = msg.arguments();
    if (kArgumentsNum != arguments.count()) {
        fmCritical() << "Vault: arguments of lock screen dbus error!";
        return;
    }

    const QString &interfaceName = msg.arguments().at(0).toString();
    fmDebug() << "Vault: Lock screen interface name:" << interfaceName;

    if (interfaceName != kAppSessionService) {
        fmDebug() << "Vault: Ignoring non-app session service message";
        return;
    }

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    fmDebug() << "Vault: Changed properties:" << keys;

    Q_FOREACH (const QString &prop, keys) {
        if (prop == "Locked") {   // screen signal property
            fmInfo() << "Vault: Screen locked, updating vault state to unknown";
            VaultHelper::instance()->updateState(VaultState::kUnknow);
        }
    }
}

VaultDBusUtils::VaultDBusUtils()
{
    fmDebug() << "Vault: Initializing VaultDBusUtils";

    QDBusConnection::sessionBus().connect(kFileManagerDBusDaemonName,
                                          kFileManagerVaultDBusPath,
                                          kFileManagerVaultDBusInterfaces,
                                          "ChangedVaultState",
                                          this,
                                          SLOT(handleChangedVaultState(const QVariantMap &)));

    QDBusConnection::sessionBus().connect(kAppSessionService,
                                          kAppSessionPath,
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged",
                                          "sa{sv}as",
                                          this,
                                          SLOT(handleLockScreenDBus(const QDBusMessage &)));

    fmDebug() << "Vault: VaultDBusUtils initialization completed";
}
