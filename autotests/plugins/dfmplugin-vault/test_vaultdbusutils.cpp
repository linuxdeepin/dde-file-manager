// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultDBusUtils tests. DBus interfaces are forced invalid via stubs so all
// manager-call methods take the deterministic no-dbus branch. No real daemon
// is ever contacted.

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariantMap>
#include <QDBusMessage>
#include <QtDBus/QDBusArgument>

#include "stubext.h"

#include "dbus/vaultdbusutils.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"

#include <QtDBus/QDBusAbstractInterface>

DPVAULT_USE_NAMESPACE

class VaultDBusUtilsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(&QDBusAbstractInterface::isValid,
                       [](QDBusAbstractInterface *) -> bool { return false; });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(VaultDBusUtilsTest, VaultManagerDBusCall_InterfaceInvalid_ReturnsNullVariant)
{
    QVariant result = VaultDBusUtils::instance()->vaultManagerDBusCall("GetLastestTime");
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.isNull());
}

TEST_F(VaultDBusUtilsTest, GetLeftoverErrorInputTimes_NoDaemon_ReturnsMinusOne)
{
    EXPECT_EQ(VaultDBusUtils::instance()->getLeftoverErrorInputTimes(), -1);
}

TEST_F(VaultDBusUtilsTest, GetNeedWaitMinutes_NoDaemon_ReturnsDefault100)
{
    EXPECT_EQ(VaultDBusUtils::instance()->getNeedWaitMinutes(), 100);
}

TEST_F(VaultDBusUtilsTest, LeftoverErrorInputTimesMinusOne_NoDaemon_NoCrash)
{
    VaultDBusUtils::instance()->leftoverErrorInputTimesMinusOne();
    VaultDBusUtils::instance()->restoreLeftoverErrorInputTimes();
    VaultDBusUtils::instance()->restoreNeedWaitMinutes();
    VaultDBusUtils::instance()->startTimerOfRestorePasswordInput();
    SUCCEED();
}

TEST_F(VaultDBusUtilsTest, IsServiceRegister_SessionBusUnknownService_ReturnsFalse)
{
    EXPECT_FALSE(VaultDBusUtils::instance()->isServiceRegister(
            QDBusConnection::SessionBus, "org.ut.not.registered.vault"));
    EXPECT_FALSE(VaultDBusUtils::instance()->isServiceRegister(
            QDBusConnection::SystemBus, "org.ut.not.registered.vault"));
}

TEST_F(VaultDBusUtilsTest, IsFullConnectInternet_PropertyInvalid_ReturnsFalse)
{
    stub.set_lamda(&QDBusAbstractInterface::property,
                   [](QObject *, const char *) -> QVariant {
                       return QVariant();
                   });
    EXPECT_FALSE(VaultDBusUtils::instance()->isFullConnectInternet());
}

TEST_F(VaultDBusUtilsTest, HandleChangedVaultState_EncryptedEntry_UpdatesState)
{
    VaultState captured = kUnknow;
    stub.set_lamda(&VaultHelper::updateState,
                   [&captured](VaultHelper *, VaultState state) -> bool {
                       captured = state;
                       return true;
                   });

    QVariantMap map;
    const QString unlockPath = PathManager::vaultUnlockPath();
    map.insert(unlockPath, static_cast<int>(VaultState::kEncrypted));
    VaultDBusUtils::instance()->handleChangedVaultState(map);

    EXPECT_EQ(captured, VaultState::kEncrypted);
}

TEST_F(VaultDBusUtilsTest, HandleChangedVaultState_OtherEntries_NoStateChange)
{
    VaultState captured = kUnknow;
    stub.set_lamda(&VaultHelper::updateState,
                   [&captured](VaultHelper *, VaultState state) -> bool {
                       captured = state;
                       return true;
                   });

    QVariantMap map;
    map.insert("/tmp/other_path", static_cast<int>(VaultState::kEncrypted));
    VaultDBusUtils::instance()->handleChangedVaultState(map);

    EXPECT_EQ(captured, VaultState::kUnknow);
}

TEST_F(VaultDBusUtilsTest, HandleLockScreenDBus_WrongArgumentCount_Ignored)
{
    QDBusMessage msg = QDBusMessage::createSignal("/tmp", "org.ut.vault", "PropertiesChanged");
    msg.setArguments({ QVariant("iface") });
    VaultDBusUtils::instance()->handleLockScreenDBus(msg);
    SUCCEED();
}

TEST_F(VaultDBusUtilsTest, HandleLockScreenDBus_LockedProperty_UpdatesStateToUnknown)
{
    VaultState captured = kEncrypted;
    stub.set_lamda(&VaultHelper::updateState,
                   [&captured](VaultHelper *, VaultState state) -> bool {
                       captured = state;
                       return true;
                   });
    // local QDBusArgument objects are write-only, so qdbus_cast is stubbed
    stub.set_lamda(static_cast<QVariantMap (*)(const QDBusArgument &)>(&qdbus_cast<QVariantMap>),
                   [](const QDBusArgument &) -> QVariantMap {
                       QVariantMap m;
                       m.insert("Locked", true);
                       return m;
                   });

    QDBusArgument arg;
    QDBusMessage msg = QDBusMessage::createSignal("/tmp", "org.ut.vault", "PropertiesChanged");
    msg.setArguments({ QVariant(kAppSessionService), QVariant::fromValue(arg), QVariant(QStringList()) });
    VaultDBusUtils::instance()->handleLockScreenDBus(msg);

    EXPECT_EQ(captured, VaultState::kUnknow);
}

TEST_F(VaultDBusUtilsTest, HandleLockScreenDBus_OtherInterface_Ignored)
{
    VaultState captured = kEncrypted;
    stub.set_lamda(&VaultHelper::updateState,
                   [&captured](VaultHelper *, VaultState state) -> bool {
                       captured = state;
                       return true;
                   });

    QDBusArgument arg;
    QDBusMessage msg = QDBusMessage::createSignal("/tmp", "org.ut.vault", "PropertiesChanged");
    msg.setArguments({ QVariant("org.other.Interface"), QVariant::fromValue(arg), QVariant(QStringList()) });
    VaultDBusUtils::instance()->handleLockScreenDBus(msg);

    EXPECT_EQ(captured, VaultState::kEncrypted);
}
