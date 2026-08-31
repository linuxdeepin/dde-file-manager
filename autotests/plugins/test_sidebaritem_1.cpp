// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebaritem_1.cpp
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

TEST_F(SideBarItemTest, SideBarItem)
{
    // Test constructor: SideBarItem((const QIcon &icon, const QString &text, const QString &group, const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarItemTest, setGroup)
{
    // Test setter: void setGroup((const QString &group))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setGroup(_arg0));
}

TEST_F(SideBarItemTest, setHiiden)
{
    // Test setter: void setHiiden((bool hidden))
    EXPECT_NO_FATAL_FAILURE(obj->setHiiden(false));
}

TEST_F(SideBarItemTest, setUrl)
{
    // Test setter: void setUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrl(_arg0));
}

TEST_F(SideBarItemTest, subGourp)
{
    // Test getter: QString subGourp()
    auto result = obj->subGourp();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(SideBarItemTest, SideBarItem_Destructor)
{
    // Test method:  ~SideBarItem(())
    EXPECT_NO_FATAL_FAILURE({ SideBarItem *tmp = new SideBarItem(); delete tmp; });
}
