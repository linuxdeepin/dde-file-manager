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

#include "interfaceactivevault.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "stub.h"

#include "dfmstandardpaths.h"
#include "controllers/vaultcontroller.h"
#include "vault/vaultconfig.h"

#include <QPixmap>
#include <QStandardPaths>
#include <QtTest/QTest>
#include <QTextStream>

#include "controllers/vaultcontroller.h"
#include "testhelper.h"
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

    Stub stub;
    QString (*st_makeVaultLocalPath)(const QString &before, const QString &behind) =
            [](const QString &before, const QString &behind)->QString {
        Q_UNUSED(behind)
        Q_UNUSED(before)

        return DFMStandardPaths::location(DFMStandardPaths::CachePath);
    };
    stub.set(ADDR(OperatorCenter, makeVaultLocalPath), st_makeVaultLocalPath);

    EXPECT_FALSE(m_opCenter->createDirAndFile());
}

/**
 * @brief TEST_F 保存密码文件
 */
TEST_F(TestOperatorCenter, saveSaltAndClipher)
{
    QString pswd("123456");
    QString hint("unit test.");

    Stub stub;
    QString (*st_makeVaultLocalPath)(const QString &before, const QString &behind) =
            [](const QString &before, const QString &behind)->QString {
        Q_UNUSED(behind)
        Q_UNUSED(before)

        return DFMStandardPaths::location(DFMStandardPaths::CachePath) + "/" + RSA_PUB_KEY_FILE_NAME;
    };
    stub.set(ADDR(OperatorCenter, makeVaultLocalPath), st_makeVaultLocalPath);

    void (*st_set)(const QString &, const QString &, QVariant) =
            [](const QString &, const QString &, QVariant){
        // do nothing.
    };
    stub.set(ADDR(VaultConfig, set), st_set);

    EXPECT_TRUE(m_opCenter->saveSaltAndCiphertext(pswd, hint));
}

/**
 * @brief TEST_F 生成密钥对
 */
TEST_F(TestOperatorCenter, createKey)
{
    QString pswd("123456");
    QString hint("unit test.");

    QString (*st_makeVaultLocalPath)(const QString &before, const QString &behind) =
            [](const QString &before, const QString &behind)->QString {
        Q_UNUSED(behind)
        Q_UNUSED(before)

        return DFMStandardPaths::location(DFMStandardPaths::CachePath) + "/" + RSA_PUB_KEY_FILE_NAME;
    };

    Stub stub;
    stub.set(ADDR(OperatorCenter, makeVaultLocalPath), st_makeVaultLocalPath);

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

    QByteArray (*st_readAll)() = []()->QByteArray {
            static int readOrder = 0;
            QString ret = "-----BEGIN RSA PUBLIC KEY-----\n"
                          "MIGJAoGBAMKc0HfUY+s+bygbQqp7Lqjf\n"
                          "9Ad6rhYD69reQiBAlGPCcR6A6oEUkpLYBDU3OpX25iAcFmni89+92eOfdYvneY9M\n"
                          "pimc5a1TQwMtRN4Ky7RXZQfm975Uuo8fJrhTxR8esNCw5nXodSS/AgMBAAE=\n"
                          "-----END RSA PUBLIC KEY-----";
            if (readOrder) {
                ret = "rrbe9m+V5241rCMniVcBFsCIC6Nlros8XWbslzpYQPFCTrOuF4ycxmE0QN0jqXtqBSr/7Xuh7"
                      "LCeOqn7pTQfJUubEUesjtEmfp0rQCvsYeO5JYU0ps4RNDerpA07qvNeEB87PCLOo3g7dkVVdU0XGMyktzadPVJt2RCR/fcZBrs=";
            }
            readOrder = 1 - readOrder;
            std::string str = ret.toStdString();
            return str.c_str();
    };
    Stub stub;
    stub.set(ADDR(QFile, readAll), st_readAll);

    QString (*st_makeVaultLocalPath)(const QString &before, const QString &behind) =
            [](const QString &before, const QString &behind)->QString {
        Q_UNUSED(behind)
        Q_UNUSED(before)

        return DFMStandardPaths::location(DFMStandardPaths::CachePath) + "/" + RSA_PUB_KEY_FILE_NAME;
    };
    stub.set(ADDR(OperatorCenter, makeVaultLocalPath), st_makeVaultLocalPath);

    userKey = "dHabNHW8QoOrttmsIlVSYq7OiDuuqarL";
    EXPECT_NO_FATAL_FAILURE(m_opCenter->checkUserKey(userKey, cliper));
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
