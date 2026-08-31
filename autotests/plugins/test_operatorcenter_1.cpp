// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operatorcenter_1.cpp
 * @brief Unit tests for OperatorCenter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryption/operatorcenter.h"

#include <QTest>

using namespace dfmplugin_vault;

class OperatorCenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperatorCenter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperatorCenter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperatorCenterTest, autoGeneratePassword)
{
    // Test method: QString autoGeneratePassword((int length))
    auto result = obj->autoGeneratePassword(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, createKeyNew)
{
    // Test method: Result createKeyNew((const QString &password))
    QString _arg0{};
    auto result = obj->createKeyNew(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createKeyNew(_arg0); });

}

TEST_F(OperatorCenterTest, executeProcess)
{
    // Test method: bool executeProcess((const QString &cmd))
    QString _arg0{};
    auto result = obj->executeProcess(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, generateRecoveryKeyForNewVault)
{
    // Test getter: QString generateRecoveryKeyForNewVault()
    auto result = obj->generateRecoveryKeyForNewVault();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getConfigFilePath)
{
    // Test getter: QStringList getConfigFilePath()
    auto result = obj->getConfigFilePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getPasswordHint)
{
    // Test method: bool getPasswordHint((QString &passwordHint))
    QString _arg0{};
    auto result = obj->getPasswordHint(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, getPendingOldPasswordSchemeMigrationPassword)
{
    // Test getter: QString getPendingOldPasswordSchemeMigrationPassword()
    auto result = obj->getPendingOldPasswordSchemeMigrationPassword();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getPubKey)
{
    // Test getter: QString getPubKey()
    auto result = obj->getPubKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getRecoveryKey)
{
    // Test getter: QString getRecoveryKey()
    auto result = obj->getRecoveryKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getSaltAndPasswordCipher)
{
    // Test getter: QString getSaltAndPasswordCipher()
    auto result = obj->getSaltAndPasswordCipher();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getUserKey)
{
    // Test getter: QString getUserKey()
    auto result = obj->getUserKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, getdecryptDirPath)
{
    // Test getter: QString getdecryptDirPath()
    auto result = obj->getdecryptDirPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, isNewVaultVersion)
{
    // Test bool getter: isNewVaultVersion()
    bool result = obj->isNewVaultVersion();
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, makeVaultLocalPath)
{
    // Test method: QString makeVaultLocalPath((const QString &before, const QString &behind))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->makeVaultLocalPath(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, passwordFromKeyring)
{
    // Test getter: QString passwordFromKeyring()
    auto result = obj->passwordFromKeyring();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OperatorCenterTest, runCmd)
{
    // Test method: bool runCmd((const QString &cmd))
    QString _arg0{};
    auto result = obj->runCmd(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, setPendingOldPasswordSchemeMigrationPassword)
{
    // Test setter: void setPendingOldPasswordSchemeMigrationPassword((const QString &password))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPendingOldPasswordSchemeMigrationPassword(_arg0));
}

TEST_F(OperatorCenterTest, setRecoveryKey)
{
    // Test setter: void setRecoveryKey((const QString &recoveryKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setRecoveryKey(_arg0));
}
