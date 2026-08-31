// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarmodel.cpp
 * @brief Unit tests for SideBarModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treemodels/sidebarmodel.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarModelTest, addSubItems)
{
    // Test method: void addSubItems((const QModelIndex &index, const QList<QUrl> &urls))
    QModelIndex _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addSubItems(_arg0, _arg1));
}

TEST_F(SideBarModelTest, findGroupIndex)
{
    // Test method: QModelIndex findGroupIndex((const QString &name))
    QString _arg0{};
    auto result = obj->findGroupIndex(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarModelTest, groupItems)
{
    // Test getter: QList<SideBarItemSeparator *> groupItems()
    auto result = obj->groupItems();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarModelTest, onDirectoryRemoved)
{
    // Test method: void onDirectoryRemoved((const QUrl &parentUrl, const QUrl &url))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDirectoryRemoved(_arg0, _arg1));
}

TEST_F(SideBarModelTest, onItemCollapsed)
{
    // Test method: void onItemCollapsed((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemCollapsed(_arg0));
}

TEST_F(SideBarModelTest, subItems)
{
    // Test method: QList<SideBarItem *> subItems((const QString &groupName))
    QString _arg0{};
    auto result = obj->subItems(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarModelTest, updateRow)
{
    // Test method: void updateRow((const QUrl &url, const ItemInfo &newInfo))
    QUrl _arg0{};
    ItemInfo _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateRow(_arg0, _arg1));
}
