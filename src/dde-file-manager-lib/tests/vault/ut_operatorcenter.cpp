#include "interfaceactivevault.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPixmap>
#include <QStandardPaths>
#include "QtTest/QTest"
#include <QTextStream>

#include "controllers/vaultcontroller.h"

#include "stub.h"

#define private public
#include "operatorcenter.h"
#undef private


DFM_USE_NAMESPACE
namespace  {
class TestOperatorCenter : public testing::Test
{
public:
    OperatorCenter *m_opCenter;
    virtual void SetUp() override
    {
        m_opCenter = OperatorCenter::getInstance();
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


    QString(*stubFun)(const QString & before, const QString & behind) = [](const QString & before, const QString & behind)->QString{
        Q_UNUSED(before)
        Q_UNUSED(behind)
        //do nothing.
        return "";
    };

    Stub stub;
    stub.set(ADDR(OperatorCenter, makeVaultLocalPath), stubFun);
    EXPECT_FALSE(m_opCenter->createDirAndFile());
}

/**
 * @brief TEST_F 保存密码文件
 */
TEST_F(TestOperatorCenter, saveSaltAndClipher)
{
    QString pswd("123456");
    QString hint("unit test.");

    void (*st_operator)(void *obj, const QString & str) = [](void *obj, const QString & str) {
        Q_UNUSED(obj)
        Q_UNUSED(str)
        //do nothing.
    };

    Stub stub;
    stub.set((QTextStream & (QTextStream::*)(const QString &)) ADDR(QTextStream, operator<<), st_operator);
    EXPECT_TRUE(m_opCenter->saveSaltAndCiphertext(pswd, hint));
}

/**
 * @brief TEST_F 生成密钥对
 */
TEST_F(TestOperatorCenter, createKey)
{
    QString pswd("123456");
    QString hint("unit test.");

    void (*st_operator)(void *obj, const QString & str) = [](void *obj, const QString & str) {
        Q_UNUSED(obj)
        Q_UNUSED(str)
        //do nothing.
    };

    Stub stub;
    stub.set((QTextStream & (QTextStream::*)(const QString &)) ADDR(QTextStream, operator<<), st_operator);
    EXPECT_TRUE(m_opCenter->createKey(pswd, 0));
    EXPECT_FALSE(m_opCenter->createKey(pswd, 2048));
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
    EXPECT_FALSE(m_opCenter->checkUserKey(userKey, cliper));

    userKey.resize(USER_KEY_LENGTH);
    EXPECT_FALSE(m_opCenter->checkUserKey(userKey, cliper));

    QString(*st_makeVaultLocalPath)(const QString & before, const QString & behind) =
    [](const QString & before, const QString & behind)->QString{
        Q_UNUSED(before)
        Q_UNUSED(behind)
        //do nothing.
        return "";
    };

    Stub stub;
    stub.set(ADDR(OperatorCenter, makeVaultLocalPath), st_makeVaultLocalPath);
    EXPECT_FALSE(m_opCenter->checkUserKey(userKey, cliper));


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
    QString strPasswordHintFilePath = VaultController::makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);
    QFile passwordHintFile(strPasswordHintFilePath);
    QString passwordHint = "";
    if (passwordHintFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        passwordHint = QString(passwordHintFile.readAll());
    }
    passwordHintFile.close();

    QString opPswdHint;
    m_opCenter->getPasswordHint(opPswdHint);

    EXPECT_EQ(passwordHint, opPswdHint);
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
    EXPECT_NO_FATAL_FAILURE(m_opCenter->getSaltAndPasswordCipher());
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

/**
 * @brief TEST_F 执行进程
 */
TEST_F(TestOperatorCenter, tst_exeuteProcess)
{
    QStringList msgOut;

    m_opCenter->executeProcess("sudo ls");
    m_opCenter->executeProcess("ls");

    m_opCenter->executeProcess("lu");
}
