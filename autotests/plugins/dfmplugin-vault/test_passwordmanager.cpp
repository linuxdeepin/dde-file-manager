// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PasswordManager operates on LUKS containers via libcryptsetup. These tests
// only exercise paths that never format a real container: nonexistent paths
// make crypt_init() fail gracefully, and random-bytes helpers are pure.

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QString>

#include "utils/encryption/passwordmanager.h"

DPVAULT_USE_NAMESPACE

class PasswordManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override
    {
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(PasswordManagerTest, CreateLuksContainer_NonExistentPath_ReturnsFailure)
{
    int slotID = -1;
    int ret = PasswordManager::createLuksContainer(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "masterkey", 8, "pwd", slotID);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(slotID, -1);
}

TEST_F(PasswordManagerTest, ExportMasterKey_NonExistentPath_ReturnsFailure)
{
    char buf[64] = { 0 };
    size_t size = sizeof(buf);
    int ret = PasswordManager::exportMasterKey(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "pwd", buf, &size);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(size, sizeof(buf));
}

TEST_F(PasswordManagerTest, ExportMasterKeyByKeyslot_NonExistentPath_ReturnsFailure)
{
    char buf[64] = { 0 };
    size_t size = sizeof(buf);
    int ret = PasswordManager::exportMasterKeyByKeyslot(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "pwd", 1, buf, &size);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(size, sizeof(buf));
}

TEST_F(PasswordManagerTest, AddNewPassword_NonExistentPath_ReturnsFailure)
{
    int newSlot = -1;
    int ret = PasswordManager::addNewPassword(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "oldpwd", "newpwd", newSlot);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(newSlot, -1);
}

TEST_F(PasswordManagerTest, AddNewPasswordByKeyslot_NonExistentPath_ReturnsFailure)
{
    int newSlot = -1;
    int ret = PasswordManager::addNewPasswordByKeyslot(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "oldpwd", 0, "newpwd", newSlot);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(newSlot, -1);
}

TEST_F(PasswordManagerTest, ChangePassword_NonExistentPath_ReturnsFailure)
{
    int newSlot = -1;
    int ret = PasswordManager::changePassword(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "oldpwd", "newpwd", newSlot);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(newSlot, -1);
}

TEST_F(PasswordManagerTest, DeleteKeyslot_NonExistentPath_ReturnsFailure)
{
    int ret = PasswordManager::deleteKeyslot(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(), 0);
    EXPECT_EQ(ret, -1);
}

TEST_F(PasswordManagerTest, FindKeyslotByPassword_NonExistentPath_ReturnsFailure)
{
    int slotId = -1;
    int ret = PasswordManager::findKeyslotByPassword(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "pwd", slotId);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(slotId, -1);
}

TEST_F(PasswordManagerTest, VerifyPassword_NonExistentPath_ReturnsFailure)
{
    bool isRight = true;
    int ret = PasswordManager::verifyPassword(
            (tempDir->path() + "/no_such_container.bin").toUtf8().constData(),
            "pwd", isRight);
    EXPECT_EQ(ret, -1);
    EXPECT_FALSE(isRight);
}

TEST_F(PasswordManagerTest, GenerateRandomBytes_NullOutput_ReturnsFailure)
{
    EXPECT_EQ(PasswordManager::generateRandomBytes(nullptr, 8), -1);
}

TEST_F(PasswordManagerTest, GenerateRandomBytes_ZeroSize_ReturnsFailure)
{
    char buf[4] = { 0 };
    EXPECT_EQ(PasswordManager::generateRandomBytes(buf, 0), -1);
}

TEST_F(PasswordManagerTest, GenerateRandomBytes_ValidBuffer_ReturnsPrintableKey)
{
    const int size = 16;
    char buf[size + 1] = { 0 };
    int ret = PasswordManager::generateRandomBytes(buf, size);
    ASSERT_EQ(ret, 0);
    EXPECT_EQ(QString::fromLatin1(buf).length(), size);
    for (int i = 0; i < size; ++i) {
        bool printable = (buf[i] >= 'A' && buf[i] <= 'Z')
                || (buf[i] >= 'a' && buf[i] <= 'z')
                || (buf[i] >= '0' && buf[i] <= '9');
        EXPECT_TRUE(printable) << "unexpected char " << buf[i];
    }
}

TEST_F(PasswordManagerTest, GenerateSecureRecoveryKey_SmallBuffer_ReturnsFailure)
{
    char buf[32] = { 0 };
    EXPECT_EQ(PasswordManager::generateSecureRecoveryKey(buf, sizeof(buf)), -1);
}

TEST_F(PasswordManagerTest, GenerateSecureRecoveryKey_EnoughBuffer_Returns32CharKey)
{
    char buf[33] = { 0 };
    int ret = PasswordManager::generateSecureRecoveryKey(buf, sizeof(buf));
    ASSERT_EQ(ret, 0);
    EXPECT_EQ(QString::fromLatin1(buf).length(), 32);
}

TEST_F(PasswordManagerTest, CreatePasswordContainerFile_ValidPath_Creates16MBFile)
{
    QString path = tempDir->path() + "/container.bin";
    int ret = PasswordManager::createPasswordContainerFile(path.toUtf8().constData());
    ASSERT_EQ(ret, 0);
    EXPECT_EQ(QFileInfo(path).size(), qint64(16 * 1024 * 1024));
}

TEST_F(PasswordManagerTest, CreatePasswordContainerFile_InvalidPath_ReturnsFailure)
{
    int ret = PasswordManager::createPasswordContainerFile("/no_such_dir_xyz/container.bin");
    EXPECT_EQ(ret, -1);
    EXPECT_FALSE(QFile::exists("/no_such_dir_xyz/container.bin"));
}
