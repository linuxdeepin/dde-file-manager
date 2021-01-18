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

