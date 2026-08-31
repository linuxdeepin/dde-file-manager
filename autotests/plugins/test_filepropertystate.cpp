// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filepropertystate.cpp
 * @brief Unit tests for FilePropertyState methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfmplugin_propertydialog_global.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class FilePropertyStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FilePropertyState();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FilePropertyState *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FilePropertyStateTest, operator==)
{
    // Test bool getter: operator==()
    bool result = obj->operator==();
    EXPECT_FALSE(result);
}

TEST_F(FilePropertyStateTest, operator!=)
{
    // Test bool getter: operator!=()
    bool result = obj->operator!=();
    EXPECT_FALSE(result);
}

TEST_F(FilePropertyStateTest, hideState)
{
    // Test getter: int hideState()
    auto result = obj->hideState();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyStateTest, ownerIndex)
{
    // Test getter: int ownerIndex()
    auto result = obj->ownerIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyStateTest, groupIndex)
{
    // Test getter: int groupIndex()
    auto result = obj->groupIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyStateTest, otherIndex)
{
    // Test getter: int otherIndex()
    auto result = obj->otherIndex();
    EXPECT_EQ(result, 0);

}
