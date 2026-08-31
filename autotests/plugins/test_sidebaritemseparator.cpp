// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebaritemseparator.cpp
 * @brief Unit tests for SideBarItemSeparator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebaritem.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarItemSeparatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarItemSeparator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarItemSeparator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarItemSeparatorTest, SideBarItemSeparator)
{
    // Test constructor: SideBarItemSeparator((const QString &group))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarItemSeparatorTest, isExpanded)
{
    // Test bool getter: isExpanded()
    bool result = obj->isExpanded();
    EXPECT_FALSE(result);

}

TEST_F(SideBarItemSeparatorTest, isVisible)
{
    // Test bool getter: isVisible()
    bool result = obj->isVisible();
    EXPECT_FALSE(result);

}

TEST_F(SideBarItemSeparatorTest, setExpanded)
{
    // Test setter: void setExpanded((bool state))
    EXPECT_NO_FATAL_FAILURE(obj->setExpanded(false));
}

TEST_F(SideBarItemSeparatorTest, setVisible)
{
    // Test setter: void setVisible((bool value))
    EXPECT_NO_FATAL_FAILURE(obj->setVisible(false));
}
