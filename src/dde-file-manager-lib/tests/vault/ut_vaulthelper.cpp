#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <DSysInfo>

#include "interfaceactivevault.h"
#include "vault/vaulthelper.h"
#include "dialogs/dtaskdialog.h"

#include "stub.h"

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

    bool (*st_bHaveVaultTask_true)() = []()->bool{
        return true;
    };

    Stub stub;
    stub.set(ADDR(DTaskDialog, haveNotCompletedVaultTask), st_bHaveVaultTask_true);
    EXPECT_TRUE(m_vaultHelper->topVaultTasks());

    int (*st_executionShellCommand)(const QString &, QStringList &) =
    [](const QString & strCmd, QStringList & lstShellOutput)->int{
        Q_UNUSED(strCmd)
        lstShellOutput << "ls" << "ls";
        return 0;
    };

    bool (*st_bHaveVaultTask_false)() = []()->bool{
        return false;
    };
    stub.set(ADDR(DTaskDialog, haveNotCompletedVaultTask), st_bHaveVaultTask_false);
    stub.set(ADDR(InterfaceActiveVault, executionShellCommand), st_executionShellCommand);
    EXPECT_NO_FATAL_FAILURE(m_vaultHelper->topVaultTasks());
}

/**
 * @brief TEST_F 终止保险箱任务
 */
TEST_F(TestVaultHelper, killVaultTasks)
{
    EXPECT_NO_FATAL_FAILURE(m_vaultHelper->killVaultTasks());

    // use stub
    bool (*st_bHaveVaultTask)() = []()->bool{
        return true;
    };

    Stub stub;
    stub.set(ADDR(DTaskDialog, haveNotCompletedVaultTask), st_bHaveVaultTask);
    EXPECT_TRUE(m_vaultHelper->topVaultTasks());

    int (*st_executionShellCommand)(const QString &, QStringList &) =
    [](const QString & strCmd, QStringList & lstShellOutput)->int{
        Q_UNUSED(strCmd)
        lstShellOutput << "ls" << "ls";
        return 0;
    };
    stub.set(ADDR(InterfaceActiveVault, executionShellCommand), st_executionShellCommand);
    EXPECT_NO_FATAL_FAILURE(m_vaultHelper->killVaultTasks());
}

/**
 * @brief TEST_F 是否启动保险箱
 */
TEST_F(TestVaultHelper, isVaultEnabled)
{
    if (!DSysInfo::isCommunityEdition()) {
        DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
        if (DSysInfo::DeepinType::DeepinPersonal != deepinType && DSysInfo::DeepinType::UnknownDeepin != deepinType) {
            EXPECT_TRUE(m_vaultHelper->isVaultEnabled());
        } else {
            EXPECT_FALSE(m_vaultHelper->isVaultEnabled());
        }
    }
}
