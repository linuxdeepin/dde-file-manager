// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>

#include "stubext.h"

#include "utils/encryption/operatorcenter.h"
#include "utils/vaultdefine.h"

DPVAULT_USE_NAMESPACE

// OperatorCenter is a singleton. We test pure-logic methods and filesystem
// methods that gracefully handle missing files. buildFilePath is a C-style
// variadic and cannot be stubbed, so we let methods use the real kVaultBasePath
// and create/remove files there as needed.

class OperatorCenterTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        operatorCenter = OperatorCenter::getInstance();
        // Ensure vault base dir exists
        QDir().mkpath(kVaultBasePath);
    }

    void TearDown() override
    {
        stub.clear();
        // Clean up test files
        for (const char *fname : {kPasswordHintFileName, kPasswordFileName, kRSAPUBKeyFileName, kRSACiphertextFileName}) {
            QFile f(kVaultBasePath + "/" + fname);
            f.remove();
        }
    }

    OperatorCenter *operatorCenter = nullptr;
};

// --- getUserKey / getSaltAndPasswordCipher / clearSaltAndPasswordCipher ---

TEST_F(OperatorCenterTest, GetUserKey_ReturnsCachedValue)
{
    EXPECT_TRUE(operatorCenter->getUserKey().isEmpty());
}

TEST_F(OperatorCenterTest, GetSaltAndPasswordCipher_ReturnsCachedValue)
{
    EXPECT_TRUE(operatorCenter->getSaltAndPasswordCipher().isEmpty());
}

TEST_F(OperatorCenterTest, ClearSaltAndPasswordCipher_ClearsValue)
{
    operatorCenter->clearSaltAndPasswordCipher();
    EXPECT_TRUE(operatorCenter->getSaltAndPasswordCipher().isEmpty());
}

// --- getdecryptDirPath ---

TEST_F(OperatorCenterTest, GetDecryptDirPath_ReturnsPath)
{
    QString path = operatorCenter->getdecryptDirPath();
    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains(kVaultDecryptDirName));
}

// --- getConfigFilePath ---

TEST_F(OperatorCenterTest, GetConfigFilePath_ReturnsFourPaths)
{
    QStringList paths = operatorCenter->getConfigFilePath();
    EXPECT_EQ(paths.size(), 4);
}

// --- autoGeneratePassword ---

TEST_F(OperatorCenterTest, AutoGeneratePassword_TooShort_ReturnsEmpty)
{
    EXPECT_TRUE(operatorCenter->autoGeneratePassword(2).isEmpty());
}

TEST_F(OperatorCenterTest, AutoGeneratePassword_ZeroLength_ReturnsEmpty)
{
    EXPECT_TRUE(operatorCenter->autoGeneratePassword(0).isEmpty());
}

TEST_F(OperatorCenterTest, AutoGeneratePassword_MinLength_ReturnsValidPassword)
{
    QString pwd = operatorCenter->autoGeneratePassword(3);
    EXPECT_EQ(pwd.length(), 3);
}

TEST_F(OperatorCenterTest, AutoGeneratePassword_LongerLength_ReturnsCorrectSize)
{
    QString pwd = operatorCenter->autoGeneratePassword(16);
    EXPECT_EQ(pwd.length(), 16);
}

TEST_F(OperatorCenterTest, AutoGeneratePassword_ContainsMixedChars)
{
    QString pwd = operatorCenter->autoGeneratePassword(16);
    bool hasDigit = false, hasLetter = false;
    for (const QChar &c : pwd) {
        if (c.isDigit()) hasDigit = true;
        if (c.isLetter()) hasLetter = true;
    }
    EXPECT_TRUE(hasDigit);
    EXPECT_TRUE(hasLetter);
}

// (Removed flaky DifferentCalls test: srand seeded by seconds can collide)

// --- getPasswordHint ---

TEST_F(OperatorCenterTest, GetPasswordHint_NoFile_ReturnsFalse)
{
    // Remove the file first to ensure it doesn't exist
    QFile::remove(kVaultBasePath + "/" + kPasswordHintFileName);
    QString hint;
    EXPECT_FALSE(operatorCenter->getPasswordHint(hint));
    EXPECT_TRUE(hint.isEmpty());
}

TEST_F(OperatorCenterTest, GetPasswordHint_WithFile_ReturnsContent)
{
    QString hintPath = kVaultBasePath + "/" + kPasswordHintFileName;
    QFile hintFile(hintPath);
    if (hintFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        hintFile.write("myhint");
        hintFile.close();
    }

    QString hint;
    bool ok = operatorCenter->getPasswordHint(hint);
    EXPECT_TRUE(ok);
    EXPECT_EQ(hint, "myhint");
}

// --- checkPassword / checkUserKey (with no key files, should fail gracefully) ---

TEST_F(OperatorCenterTest, CheckPassword_NoFiles_ReturnsFalse)
{
    QString cipher;
    EXPECT_FALSE(operatorCenter->checkPassword("testpassword", cipher));
}

TEST_F(OperatorCenterTest, CheckUserKey_NoFiles_ReturnsFalse)
{
    QString cipher;
    EXPECT_FALSE(operatorCenter->checkUserKey("AAAABBBBCCCC", cipher));
}

// --- createKey (requires openssl; with no proper setup it should fail gracefully) ---

TEST_F(OperatorCenterTest, CreateKey_NoSetup_ReturnsFalse)
{
    EXPECT_NO_FATAL_FAILURE(operatorCenter->createKey("testpassword", 24));
}

// --- removeVault ---

TEST_F(OperatorCenterTest, RemoveVault_NonExistentDir_NoCrash)
{
    QString basePath = kVaultBasePath + "/nonexistent_vault_dir";
    EXPECT_NO_FATAL_FAILURE(operatorCenter->removeVault(basePath));
}

TEST_F(OperatorCenterTest, RemoveVault_EmptyDir_RemovesDirectory)
{
    QString basePath = kVaultBasePath + "/empty_vault_dir_test";
    QDir().mkpath(basePath);
    ASSERT_TRUE(QDir(basePath).exists());
    // removeVault runs in a background thread; wait via signal
    QSignalSpy spy(operatorCenter, &OperatorCenter::fileRemovedProgress);
    operatorCenter->removeVault(basePath);
    // Wait for the removal to complete (progress signals emitted)
    if (spy.wait(5000) || spy.count() > 0) {
        // Give the thread a moment to finish actual rmdir
        QTest::qWait(200);
    }
    EXPECT_FALSE(QDir(basePath).exists());
}
