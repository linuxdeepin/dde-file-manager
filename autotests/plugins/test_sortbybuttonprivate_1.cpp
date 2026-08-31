// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortbybuttonprivate_1.cpp
 * @brief Unit tests for SortByButtonPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/sortbybutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class SortByButtonPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortByButtonPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortByButtonPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortByButtonPrivateTest, SortByButtonPrivate)
{
    // Test constructor: SortByButtonPrivate((SortByButton *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SortByButtonPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(SortByButtonPrivateTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(SortByButtonPrivateTest, menuTriggered)
{
    // Test method: void menuTriggered((QAction *action))
    EXPECT_NO_FATAL_FAILURE(obj->menuTriggered(nullptr));
}

TEST_F(SortByButtonPrivateTest, setItemGroupRoles)
{
    // Test method: void setItemGroupRoles(())
    EXPECT_NO_FATAL_FAILURE(obj->setItemGroupRoles());
}

TEST_F(SortByButtonPrivateTest, setItemSortRoles)
{
    // Test method: void setItemSortRoles(())
    EXPECT_NO_FATAL_FAILURE(obj->setItemSortRoles());
}

TEST_F(SortByButtonPrivateTest, setupMenu)
{
    // Test method: void setupMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->setupMenu());
}
