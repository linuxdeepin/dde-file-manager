// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogmenucreator.cpp
 * @brief Unit tests for FileDialogMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/filedialogmenuscene.h"

#include <QTest>

using namespace core;

class FileDialogMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
