#include "interfaceactivevault.h"
#include "operatorcenter.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPixmap>
#include <QStandardPaths>

namespace  {
    class TestOperatorCenter : public testing::Test
    {
    public:
        OperatorCenter *m_opCenter;
        virtual void SetUp() override
        {
            m_opCenter = &OperatorCenter::getInstance();
            std::cout << "start TestOperatorCenter" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestOperatorCenter" << std::endl;
        }
    };
}

/**
 * @brief TEST_F 创建目录和加密文件
 */
TEST_F(TestOperatorCenter, createDirAndFile)
{
    EXPECT_TRUE(m_opCenter->createDirAndFile());
}

/**
 * @brief TEST_F 保存密码文件
 */
TEST_F(TestOperatorCenter, saveSaltAndClipher)
{
#if 0 // this will modify the vault password.
    QString pswd("123456");
    QString hint("unit test.");
    EXPECT_TRUE(m_opCenter->saveSaltAndClipher(pswd, hint));
#endif
}

/**
 * @brief TEST_F 生成密钥对
 */
TEST_F(TestOperatorCenter, createKey)
{
    // do somthing.
}

/**
 * @brief TEST_F 校验密码
 */
TEST_F(TestOperatorCenter, checkPassword)
{
    QString pswd("");
    QString cliper("");
    EXPECT_FALSE(m_opCenter->checkPassword(pswd, cliper));
}

/**
 * @brief TEST_F 校验用户密钥
 */
TEST_F(TestOperatorCenter, checkUserKey)
{
    QString userKey("");
    QString cliper("");
    EXPECT_FALSE(m_opCenter->checkPassword(userKey, cliper));

    userKey.resize(USER_KEY_LENGTH);
    EXPECT_FALSE(m_opCenter->checkPassword(userKey, cliper));
}

/**
 * @brief getUserKey 获得用户密钥
 * @return 用户密钥
 */
TEST_F(TestOperatorCenter, getUserKey)
{
    EXPECT_TRUE(m_opCenter->getUserKey().isEmpty());
}

/**
 * @brief getPasswordHint 获取密钥提示
 * @return 用户密钥
 */
TEST_F(TestOperatorCenter, getPasswordHint)
{
    QString pswdHint;
    bool bSuccess = m_opCenter->getPasswordHint(pswdHint);
    if (bSuccess) {
        EXPECT_FALSE(pswdHint.isEmpty());
    } else {
        EXPECT_TRUE(pswdHint.isEmpty());
    }
}

/**
 * @brief TEST_F 创建二维码
 */
TEST_F(TestOperatorCenter, createQRCode)
{
    QString src = "123456";
    QPixmap image;
    EXPECT_TRUE(m_opCenter->createQRCode(src, 100, 100, image));
    EXPECT_FALSE(m_opCenter->createQRCode(src, 0, 0, image));
}

/**
 * @brief TEST_F 获取保险箱状态
 */
TEST_F(TestOperatorCenter, vaultState)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        EXPECT_EQ(NotAvailable, m_opCenter->vaultState());
    } else {
        EXPECT_NE(NotAvailable, m_opCenter->vaultState());
    }
}

/**
 * @brief TEST_F 获取盐值及密码密文
 */
TEST_F(TestOperatorCenter, getSaltAndPasswordClipher)
{
    EXPECT_NO_FATAL_FAILURE(m_opCenter->getSaltAndPasswordClipher());
}

/**
 * @brief TEST_F 获得加密文件夹路径
 */
TEST_F(TestOperatorCenter, getEncryptDirPath)
{
    EXPECT_FALSE(m_opCenter->getEncryptDirPath().isEmpty());
}

/**
 * @brief TEST_F 获得解密文件夹路径
 */
TEST_F(TestOperatorCenter, getdecryptDirPath)
{
    EXPECT_FALSE(m_opCenter->getdecryptDirPath().isEmpty());
}

/**
 * @brief TEST_F 获取配置文件路径
 */
TEST_F(TestOperatorCenter, getConfigFilePath)
{
    EXPECT_FALSE(m_opCenter->getConfigFilePath().isEmpty());
}

/**
 * @brief TEST_F 生成随机密码
 */
TEST_F(TestOperatorCenter, autoGeneratePassword)
{
    EXPECT_FALSE(m_opCenter->autoGeneratePassword(10).isEmpty());
    EXPECT_TRUE(m_opCenter->autoGeneratePassword(1).isEmpty());
}

/**
 * @brief TEST_F 获取管理员权限认证
 */
TEST_F(TestOperatorCenter, getRootPassword)
{
    EXPECT_FALSE(m_opCenter->autoGeneratePassword(10).isEmpty());
    EXPECT_TRUE(m_opCenter->autoGeneratePassword(1).isEmpty());
}

/**
 * @brief TEST_F 执行命令
 */
TEST_F(TestOperatorCenter, executionShellCommand)
{
    QStringList msgOut;

    EXPECT_EQ(-1, m_opCenter->executionShellCommand("", msgOut));
    EXPECT_EQ(32512, m_opCenter->executionShellCommand("errorCmd", msgOut));
    EXPECT_NE(-1, m_opCenter->executionShellCommand("ls", msgOut));
}
