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
#include "operatorcenter.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestInterfaceActiveVault : public testing::Test
{
public:
    InterfaceActiveVault *m_iActiveVault;
    OperatorCenter *m_opCenter;

    virtual void SetUp() override
    {
        m_iActiveVault = new InterfaceActiveVault();
        m_opCenter = OperatorCenter::getInstance();

        std::cout << "start TestInterfaceActiveVault" << std::endl;
    }

    virtual void TearDown() override
    {
        delete m_iActiveVault;
        m_iActiveVault = nullptr;

        std::cout << "end TestInterfaceActiveVault" << std::endl;
    }
};
}

/**
 * @brief TEST_F 获取密码
 */
TEST_F(TestInterfaceActiveVault, getPassword)
{
    EXPECT_EQ(m_opCenter->getSaltAndPasswordCipher(), m_iActiveVault->getPassword());
}

/**
 * @brief TEST_F 获取密码提示
 */
TEST_F(TestInterfaceActiveVault, getPasswordHint)
{
    QString passwordHint;
    QString iPasswordHint;

    EXPECT_EQ(m_opCenter->getPasswordHint(passwordHint),
              m_iActiveVault->getPasswordHint(iPasswordHint));

    EXPECT_EQ(passwordHint, iPasswordHint);
}

/**
 * @brief TEST_F 校验密码
 */
TEST_F(TestInterfaceActiveVault, checkPassword)
{
    QString password;
    QString cliper;

    EXPECT_EQ(m_opCenter->checkPassword(password, cliper),
              m_iActiveVault->checkPassword(password, cliper));
}

/**
 * @brief TEST_F 校验userkey
 */
TEST_F(TestInterfaceActiveVault, checkUserKey)
{
    QString userkey;
    QString cliper;


    EXPECT_EQ(m_opCenter->checkUserKey(userkey, cliper),
              m_iActiveVault->checkUserKey(userkey, cliper));
}

/**
 * @brief TEST_F 获取加密目录
 */
TEST_F(TestInterfaceActiveVault, getEncryptDir)
{
    EXPECT_EQ(m_opCenter->getEncryptDirPath(), m_iActiveVault->getEncryptDir());
}

/**
 * @brief TEST_F 获取解密目录
 */
TEST_F(TestInterfaceActiveVault, getDecryptDir)
{
    EXPECT_EQ(m_opCenter->getdecryptDirPath(), m_iActiveVault->getDecryptDir());
}

/**
 * @brief TEST_F 获取配置文件目录
 */
TEST_F(TestInterfaceActiveVault, getConfigFilePath)
{
    EXPECT_EQ(m_opCenter->getConfigFilePath(), m_iActiveVault->getConfigFilePath());
}

/**
 * @brief TEST_F 保险箱状态
 */
TEST_F(TestInterfaceActiveVault, vaultState)
{
    EXPECT_EQ(m_opCenter->vaultState(), m_iActiveVault->vaultState());
}


/**
 * @brief TEST_F 获取管理员密码
 */
TEST_F(TestInterfaceActiveVault, getRootPassword)
{
//    EXPECT_EQ(m_opCenter->getRootPassword(), m_iActiveVault->getRootPassword());
}

/**
 * @brief TEST_F 执行命令
 */
TEST_F(TestInterfaceActiveVault, executionShellCommand)
{
    QString strCmd("ls");
    QStringList strMsgs;
    QStringList iStrMsgs;

    EXPECT_EQ(m_opCenter->executionShellCommand(strCmd, strMsgs),
              m_iActiveVault->executionShellCommand(strCmd, iStrMsgs));

    EXPECT_EQ(strMsgs, iStrMsgs);
}
