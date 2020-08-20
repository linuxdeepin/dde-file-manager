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
