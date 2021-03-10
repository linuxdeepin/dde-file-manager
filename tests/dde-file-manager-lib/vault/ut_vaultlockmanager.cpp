/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "controllers/vaultcontroller.h"

#include "app/define.h"
#include "dbusinterface/vault_interface.h"
#include "stub.h"

#define private public
#define protected public
#include "vault/vaultlockmanager.h"
#include "testhelper.h"

namespace  {
    class TestVaultLockManager : public testing::Test
    {
    public:
        VaultLockManager *m_vaultLockManager;
        virtual void SetUp() override
        {
            m_vaultLockManager = &VaultLockManager::getInstance();
            std::cout << "start TestVaultLockManager" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestVaultLockManager" << std::endl;
        }
    };
}

/**
 * @brief TEST_F 自动上锁状态
 */
TEST_F(TestVaultLockManager, autoLockState)
{
    TestHelper::runInLoop([](){});
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->autoLockState());
}

/**
 * @brief TEST_F 设置自动锁状态
 */
TEST_F(TestVaultLockManager, autoLock)
{
    TestHelper::runInLoop([](){});
    VaultLockManager::AutoLockState oldState = m_vaultLockManager->autoLockState();
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->autoLock(VaultLockManager::Never));
    m_vaultLockManager->autoLock(oldState);
}

TEST_F(TestVaultLockManager, tst_checkAuthentication)
{
#if 0
    // those code may cause pop dialog.
    QDBusPendingReply<bool> (*st_checkAuthentication)(const QString &) = [](const QString &)->QDBusPendingReply<bool>{
            return  QDBusPendingCall::fromCompletedCall(QDBusMessage());
    };

    Stub stub;
    stub.set(ADDR(VaultInterface, checkAuthentication), st_checkAuthentication);
    m_vaultLockManager->checkAuthentication(VAULT_CREATE);
#endif
}

TEST_F(TestVaultLockManager, tst_processAutoLock)
{
    VaultController::VaultState (*st_state)(QString) =
            [](QString str)->VaultController::VaultState{
        Q_UNUSED(str)
        return VaultController::Unlocked;
    };
    Stub stub;
    stub.set(ADDR(VaultController, state), st_state);

    VaultLockManager::AutoLockState oldState = m_vaultLockManager->autoLockState();
    m_vaultLockManager->autoLock(VaultLockManager::FiveMinutes);

    m_vaultLockManager->processAutoLock();
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->autoLock(oldState));
}

TEST_F(TestVaultLockManager, tst_slotLockVault)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->slotLockVault(0));
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->slotLockVault(-1));
}

TEST_F(TestVaultLockManager, tst_dbusSetRefreshTIme)
{
    bool (*st_isValid)() = [](){
        return true;
    };
    Stub stub;
    stub.set(ADDR(VaultInterface, isValid), st_isValid);

    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->dbusSetRefreshTime(0));
}

TEST_F(TestVaultLockManager, tst_dbusGetLastestTime)
{
    bool (*st_isValid)() = [](){
        return true;
    };
    Stub stub;
    stub.set(ADDR(VaultInterface, isValid), st_isValid);

    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->dbusGetLastestTime());
}

TEST_F(TestVaultLockManager, tst_dbusGetSelfTime)
{
    bool (*st_isValid)() = [](){
        return true;
    };
    Stub stub;
    stub.set(ADDR(VaultInterface, isValid), st_isValid);

    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->dbusGetSelfTime());
}

TEST_F(TestVaultLockManager, tst_slotUnLockVault)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->slotUnlockVault(0));
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->slotUnlockVault(1));
}

TEST_F(TestVaultLockManager, tst_processLockEvent)
{
    void (*st_lockVault)(QString, QString) = [](QString lockBaseDir, QString unlockFileDir){
        Q_UNUSED(lockBaseDir)
        Q_UNUSED(unlockFileDir)
    };
    Stub stub;
    stub.set(ADDR(VaultController, lockVault), st_lockVault);

    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->processLockEvent());
}

TEST_F(TestVaultLockManager, tst_slotLockEvent)
{
    char *loginUser = getlogin();

    void (*st_lockVault)(QString, QString) = [](QString lockBaseDir, QString unlockFileDir){
        Q_UNUSED(lockBaseDir)
        Q_UNUSED(unlockFileDir)
    };
    Stub stub;
    stub.set(ADDR(VaultController, lockVault), st_lockVault);

    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->slotLockEvent(loginUser));
}

TEST_F(TestVaultLockManager, tst_construct)
{
    bool (*st_isValid)() = [](){
        return true;
    };
    Stub stub;
    stub.set(ADDR(VaultLockManager, isValid), st_isValid);

    QSharedPointer<VaultLockManager> ptr = QSharedPointer<VaultLockManager>(new VaultLockManager());
    EXPECT_TRUE(ptr->isValid());
}

TEST_F(TestVaultLockManager, tst_refreshAccessTime)
{
    bool (*st_isValid)() = [](){
        return true;
    };
    Stub stub;
    stub.set(ADDR(VaultLockManager, isValid), st_isValid);
    m_vaultLockManager->refreshAccessTime();
}

TEST_F(TestVaultLockManager, tst_isValid)
{
    bool (*st_isValid)() = [](){
        return true;
    };
    Stub stub;
    stub.set(ADDR(VaultInterface, isValid), st_isValid);

    EXPECT_TRUE(m_vaultLockManager->isValid());
}
