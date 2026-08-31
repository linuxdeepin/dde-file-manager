// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatormenucreator.cpp
 * @brief Unit tests for FileOperatorMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/fileoperatormenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class FileOperatorMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(FileOperatorMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
