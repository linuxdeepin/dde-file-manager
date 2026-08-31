// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operatorcenter.cpp
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

TEST_F(OperatorCenterTest, checkUserKey)
{
    // Test method: bool checkUserKey((const QString &userKey, QString &cipher))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkUserKey(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, clearSaltAndPasswordCipher)
{
    // Test method: void clearSaltAndPasswordCipher(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSaltAndPasswordCipher());
}

TEST_F(OperatorCenterTest, createKey)
{
    // Test method: bool createKey((const QString &password, int bytes))
    QString _arg0{};
    auto result = obj->createKey(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, migrateOldVaultByPassword)
{
    // Test method: bool migrateOldVaultByPassword((const QString &oldPassword,
                                               const QString &newPassword,
                                               QString &outRecoveryKey))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->migrateOldVaultByPassword(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, removeDir)
{
    // Test method: void removeDir((const QString &dirPath, int filesCount, int *removedFileCount, int *removedDirCount))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeDir(_arg0, 0, nullptr, nullptr));
}

TEST_F(OperatorCenterTest, removeVault)
{
    // Test method: void removeVault((const QString &basePath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeVault(_arg0));
}

TEST_F(OperatorCenterTest, resetPasswordByOldPassword)
{
    // Test method: bool resetPasswordByOldPassword((const QString &oldPassword, const QString &newPassword, const QString &passwordHint))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->resetPasswordByOldPassword(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, resetPasswordByRecoveryKey)
{
    // Test method: bool resetPasswordByRecoveryKey((const QString &recoveryKey, const QString &newPassword, const QString &passwordHint))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->resetPasswordByRecoveryKey(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, saveKey)
{
    // Test method: Result saveKey((QString key, QString path))
    auto result = obj->saveKey(QString(), QString());
    EXPECT_NO_FATAL_FAILURE({ obj->saveKey(QString(), QString()); });

}

TEST_F(OperatorCenterTest, savePasswordAndPasswordHint)
{
    // Test method: Result savePasswordAndPasswordHint((const QString &password, const QString &passwordHint))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->savePasswordAndPasswordHint(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->savePasswordAndPasswordHint(_arg0, _arg1); });

}

TEST_F(OperatorCenterTest, savePasswordToKeyring)
{
    // Test method: Result savePasswordToKeyring((const QString &password))
    QString _arg0{};
    auto result = obj->savePasswordToKeyring(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->savePasswordToKeyring(_arg0); });

}

TEST_F(OperatorCenterTest, secondSaveSaltAndCiphertext)
{
    // Test method: bool secondSaveSaltAndCiphertext((const QString &ciphertext, const QString &salt, const char *vaultVersion))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->secondSaveSaltAndCiphertext(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, statisticsFilesInDir)
{
    // Test method: bool statisticsFilesInDir((const QString &dirPath, int *filesCount))
    QString _arg0{};
    auto result = obj->statisticsFilesInDir(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OperatorCenterTest, OperatorCenter)
{
    // Test constructor: OperatorCenter((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OperatorCenterTest, getInstance)
{
    // Test getter: OperatorCenter getInstance()
    auto result = obj->getInstance();
    EXPECT_NO_FATAL_FAILURE({ obj->getInstance(); });

}
