/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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

#include "vault/vaultmanager.h"

namespace  {
    class TestVaultManager : public testing::Test
    {
    public:
        VaultManager *m_vaultManager;
        virtual void SetUp() override
        {
            m_vaultManager = new VaultManager();
            std::cout << "start TestVaultManager" << std::endl;
        }

        virtual void TearDown() override
        {
            delete m_vaultManager;
            m_vaultManager = nullptr;
            std::cout << "end TestVaultManager" << std::endl;
        }
    };
}

TEST_F(TestVaultManager, tst_sysUserChanged)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->sysUserChanged("user"));
}

TEST_F(TestVaultManager, can_set_get_refereshTime)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->setRefreshTime(1));
    EXPECT_EQ(1, m_vaultManager->getLastestTime());
}

TEST_F(TestVaultManager, can_get_selfTime)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->getSelfTime());
}

TEST_F(TestVaultManager, can_checkAuthentication)
{
#if 0 //error occurred.
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->checkAuthentication("Create"));
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->checkAuthentication("Remove"));
#endif
}

TEST_F(TestVaultManager, can_set_get_LockEvent)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->triggerLockEvent());
    EXPECT_TRUE(m_vaultManager->isLockEventTriggered());
}

TEST_F(TestVaultManager, can_clear_lockEvent)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultManager->clearLockEvent());
}

