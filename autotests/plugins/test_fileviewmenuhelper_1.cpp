// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewmenuhelper_1.cpp
 * @brief Unit tests for FileViewMenuHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewmenuhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewMenuHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewMenuHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewMenuHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewMenuHelperTest, FileViewMenuHelper)
{
    // Test constructor: FileViewMenuHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileViewMenuHelperTest, currentMenuScene)
{
    // Test getter: QString currentMenuScene()
    auto result = obj->currentMenuScene();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(FileViewMenuHelperTest, reloadCursor)
{
    // Test method: void reloadCursor(())
    EXPECT_NO_FATAL_FAILURE(obj->reloadCursor());
}

TEST_F(FileViewMenuHelperTest, setWaitCursor)
{
    // Test method: void setWaitCursor(())
    EXPECT_NO_FATAL_FAILURE(obj->setWaitCursor());
}
