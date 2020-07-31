#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <DSysInfo>

#include "vault/vaulthelper.h"

DFM_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace  {
    class TestVaultHelper : public testing::Test
    {
    public:
        VaultHelper *m_vaultHelper;
        virtual void SetUp() override
        {
            m_vaultHelper = new VaultHelper();
            std::cout << "start TestVaultHelper" << std::endl;
        }

        virtual void TearDown() override
        {
            delete m_vaultHelper;
            m_vaultHelper = nullptr;
            std::cout << "end TestVaultHelper" << std::endl;
        }
    };
}

/**
 * @brief TEST_F 置顶保险箱任务窗口
 */
TEST_F(TestVaultHelper, topVaultTasks)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultHelper->topVaultTasks());
}

/**
 * @brief TEST_F 终止保险箱任务
 */
TEST_F(TestVaultHelper, killVaultTasks)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultHelper->killVaultTasks());
}

/**
 * @brief TEST_F 是否启动保险箱
 */
TEST_F(TestVaultHelper, isVaultEnabled)
{
    if(!DSysInfo::isCommunityEdition()){
       DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
        if(DSysInfo::DeepinType::DeepinPersonal != deepinType && DSysInfo::DeepinType::UnknownDeepin != deepinType){
            EXPECT_TRUE(m_vaultHelper->isVaultEnabled());
        } else {
            EXPECT_FALSE(m_vaultHelper->isVaultEnabled());
        }
    }
}
