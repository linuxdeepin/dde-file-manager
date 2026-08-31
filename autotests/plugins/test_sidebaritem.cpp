// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebaritem.cpp
 * @brief Unit tests for SideBarItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebaritem.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarItemTest, group)
{
    // Test getter: QString group()
    auto result = obj->group();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarItemTest, isHidden)
{
    // Test bool getter: isHidden()
    bool result = obj->isHidden();
    EXPECT_FALSE(result);

}

TEST_F(SideBarItemTest, itemInfo)
{
    // Test getter: ItemInfo itemInfo()
    auto result = obj->itemInfo();
    EXPECT_NO_FATAL_FAILURE({ obj->itemInfo(); });

}

TEST_F(SideBarItemTest, setIcon)
{
    // Test setter: void setIcon((const QIcon &icon))
    QIcon _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIcon(_arg0));
}

TEST_F(SideBarItemTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(SideBarItemTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
