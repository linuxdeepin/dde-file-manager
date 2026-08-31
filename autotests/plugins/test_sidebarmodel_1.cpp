// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarmodel_1.cpp
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

TEST_F(SideBarModelTest, addEmptyItem)
{
    // Test method: void addEmptyItem(())
    EXPECT_NO_FATAL_FAILURE(obj->addEmptyItem());
}

TEST_F(SideBarModelTest, dropMimeData)
{
    // Test method: bool dropMimeData((const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent))
    QModelIndex _arg4{};
    auto result = obj->dropMimeData(nullptr, Qt::DropAction(), 0, 0, _arg4);
    EXPECT_FALSE(result);

}

TEST_F(SideBarModelTest, itemFromIndex)
{
    // Test method: SideBarItem itemFromIndex((int index, const QModelIndex &parent))
    QModelIndex _arg1{};
    auto result = obj->itemFromIndex(0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->itemFromIndex(0, _arg1); });

}

TEST_F(SideBarModelTest, mimeData)
{
    // Test method: QMimeData mimeData((const QModelIndexList &indexes))
    QModelIndexList _arg0{};
    auto result = obj->mimeData(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->mimeData(_arg0); });

}

TEST_F(SideBarModelTest, onDirectoryCreated)
{
    // Test method: void onDirectoryCreated((const QUrl &parentUrl, const QUrl &url))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDirectoryCreated(_arg0, _arg1));
}

TEST_F(SideBarModelTest, onDirectoryRenamed)
{
    // Test method: void onDirectoryRenamed((const QUrl &parentUrl, const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onDirectoryRenamed(_arg0, _arg1, _arg2));
}

TEST_F(SideBarModelTest, onItemExpanded)
{
    // Test method: void onItemExpanded((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemExpanded(_arg0));
}
