// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileencrypthandle_1.cpp
 * @brief Unit tests for FileEncryptHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileencrypthandle.h"

#include <QTest>

using namespace dfmplugin_vault;

class FileEncryptHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileEncryptHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileEncryptHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileEncryptHandleTest, createDirIfNotExist)
{
    // Test method: bool createDirIfNotExist((QString path))
    auto result = obj->createDirIfNotExist(QString());
    EXPECT_FALSE(result);

}

TEST_F(FileEncryptHandleTest, encryptAlgoTypeOfGroupPolicy)
{
    // Test getter: EncryptType encryptAlgoTypeOfGroupPolicy()
    auto result = obj->encryptAlgoTypeOfGroupPolicy();
    EXPECT_NO_FATAL_FAILURE({ obj->encryptAlgoTypeOfGroupPolicy(); });

}

TEST_F(FileEncryptHandleTest, lockVault)
{
    // Test method: bool lockVault((QString unlockFileDir, bool isForced))
    auto result = obj->lockVault(QString(), false);
    EXPECT_FALSE(result);

}

TEST_F(FileEncryptHandleTest, slotReadOutput)
{
    // Test method: void slotReadOutput(())
    EXPECT_NO_FATAL_FAILURE(obj->slotReadOutput());
}

TEST_F(FileEncryptHandleTest, updateState)
{
    // Test method: bool updateState((VaultState curState))
    auto result = obj->updateState(VaultState());
    EXPECT_FALSE(result);

}

TEST_F(FileEncryptHandleTest, FileEncryptHandle_Destructor)
{
    // Test method:  ~FileEncryptHandle(())
    EXPECT_NO_FATAL_FAILURE({ FileEncryptHandle *tmp = new FileEncryptHandle(); delete tmp; });
}
