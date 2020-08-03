#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "vault/vaultlockmanager.h"

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
    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->autoLockState());
}

/**
 * @brief TEST_F 设置自动锁状态
 */
TEST_F(TestVaultLockManager, autoLock)
{
//    EXPECT_NO_FATAL_FAILURE(m_vaultLockManager->autoLock(VaultLockManager::Never));
}

// The rest will be add later.
