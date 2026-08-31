// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_passwordmanager_1.cpp
 * @brief Unit tests for PasswordManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryption/passwordmanager.h"

#include <QTest>

using namespace dfmplugin_vault;

class PasswordManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PasswordManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PasswordManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PasswordManagerTest, addNewPassword)
{
    // Test method: int addNewPassword((const char *path,
                                     const char *existingPassword,
                                     const char *newPassword,
                                     int &newKeySlotId))
    int _arg3{};
    auto result = obj->addNewPassword(nullptr, nullptr, nullptr, _arg3);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, addNewPasswordByKeyslot)
{
    // Test method: int addNewPasswordByKeyslot((const char *path,
                                              const char *existingPassword,
                                              int existingPasswordKeyslot,
                                              const char *newPassword,
                                              int &newKeySlotId))
    int _arg4{};
    auto result = obj->addNewPasswordByKeyslot(nullptr, nullptr, 0, nullptr, _arg4);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, changePassword)
{
    // Test method: int changePassword((const char *path,
                                    const char *oldPassword,
                                    const char *newPassword,
                                    int &newKeySlotId))
    int _arg3{};
    auto result = obj->changePassword(nullptr, nullptr, nullptr, _arg3);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, createLuksContainer)
{
    // Test method: int createLuksContainer((const char *path,
                                        const char *masterKey,
                                        size_t masterKeySize,
                                        const char *password,
                                        int &slotID))
    int _arg4{};
    auto result = obj->createLuksContainer(nullptr, nullptr, {}, nullptr, _arg4);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, createPasswordContainerFile)
{
    // Test method: int createPasswordContainerFile((const char *path))
    auto result = obj->createPasswordContainerFile(nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, exportMasterKey)
{
    // Test method: int exportMasterKey((const char *path,
                                     const char *password,
                                     char *masterKey,
                                     size_t *masterKeySize))
    auto result = obj->exportMasterKey(nullptr, nullptr, nullptr, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, exportMasterKeyByKeyslot)
{
    // Test method: int exportMasterKeyByKeyslot((const char *path,
                                              const char *password,
                                              int keyslot,
                                              char *masterKey,
                                              size_t *masterKeySize))
    auto result = obj->exportMasterKeyByKeyslot(nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, findKeyslotByPassword)
{
    // Test method: int findKeyslotByPassword((const char *path,
                                            const char *password,
                                            int &keyslotId))
    int _arg2{};
    auto result = obj->findKeyslotByPassword(nullptr, nullptr, _arg2);
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, generateRandomBytes)
{
    // Test method: int generateRandomBytes((char *output, size_t size))
    auto result = obj->generateRandomBytes(nullptr, {});
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, generateSecureRecoveryKey)
{
    // Test method: int generateSecureRecoveryKey((char *output, size_t outputSize))
    auto result = obj->generateSecureRecoveryKey(nullptr, {});
    EXPECT_GE(result, 0);

}

TEST_F(PasswordManagerTest, verifyPassword)
{
    // Test method: int verifyPassword((const char *path,
                                    const char *password,
                                    bool &isRight))
    bool _arg2{};
    auto result = obj->verifyPassword(nullptr, nullptr, _arg2);
    EXPECT_GE(result, 0);

}
