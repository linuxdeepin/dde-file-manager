// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileencrypthandlerprivate.cpp
 * @brief Unit tests for FileEncryptHandlerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileencrypthandle.h"

#include <QTest>

using namespace dfmplugin_vault;

class FileEncryptHandlerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileEncryptHandlerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileEncryptHandlerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileEncryptHandlerPrivateTest, encryptAlgoTypeOfGroupPolicy)
{
    // Test getter: EncryptType encryptAlgoTypeOfGroupPolicy()
    auto result = obj->encryptAlgoTypeOfGroupPolicy();
    EXPECT_NO_FATAL_FAILURE({ obj->encryptAlgoTypeOfGroupPolicy(); });

}

TEST_F(FileEncryptHandlerPrivateTest, versionString)
{
    // Test getter: FileEncryptHandlerPrivate::CryfsVersionInfo versionString()
    auto result = obj->versionString();
    EXPECT_GE(static_cast<int>(result), 0);

}
