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

#include "vault/vaultclock.h"

namespace  {
    class TestVaultClock : public testing::Test
    {
    public:
        VaultClock *m_vaultClock;
        virtual void SetUp() override
        {
            m_vaultClock = new VaultClock();
            std::cout << "start TestVaultClock" << std::endl;
        }

        virtual void TearDown() override
        {
            delete m_vaultClock;
            m_vaultClock = nullptr;
            std::cout << "end TestVaultClock" << std::endl;
        }
    };
}

TEST_F(TestVaultClock, can_set_get_refreshTime)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultClock->setRefreshTime(1));
    EXPECT_EQ(1, m_vaultClock->getLastestTime());
}

TEST_F(TestVaultClock, can_getSelfTime)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultClock->getSelfTime());
}

TEST_F(TestVaultClock, can_set_get_lockEvent)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultClock->triggerLockEvent());
    EXPECT_TRUE(m_vaultClock->isLockEventTriggered());
}

TEST_F(TestVaultClock, can_clearEvent)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultClock->clearLockEvent());
}
