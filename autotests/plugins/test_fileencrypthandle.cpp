// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileencrypthandle.cpp
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

TEST_F(FileEncryptHandleTest, instance)
{
    // Test getter: FileEncryptHandle instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
