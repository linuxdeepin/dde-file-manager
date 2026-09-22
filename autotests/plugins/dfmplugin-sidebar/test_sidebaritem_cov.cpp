// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-sidebar gap list):
//   treeviews/sidebaritem.cpp:
//     - SideBarItem::isHidden() const -> IsHidden_AfterSetHidden_ReadsGroupRoleFlag (see note)
// Note: isHidden() reads the item-group role instead of the hidden flag (existing tests
// flag this as a source quirk); the exact assertions below pin the actual behaviour.

#include "treeviews/sidebaritem.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariant>
#include <QIcon>

using namespace dfmplugin_sidebar;

class UT_SideBarItemCov : public testing::Test
{
protected:
    void SetUp() override
    {
        item = new SideBarItem(QUrl("file:///tmp/item-hidden-check"));
    }
    void TearDown() override
    {
        delete item;
        item = nullptr;
    }
    SideBarItem *item { nullptr };
};

TEST_F(UT_SideBarItemCov, IsHidden_AfterSetHidden_ReadsGroupRoleFlag)
{
    // Arrange
    item->setHiiden(true);

    // Act
    const bool hiddenWhenSet = item->isHidden();

    // Assert: isHidden() consults the group role, which is not the hidden flag
    EXPECT_FALSE(hiddenWhenSet);
    EXPECT_EQ(item->url(), QUrl("file:///tmp/item-hidden-check"));
}

TEST_F(UT_SideBarItemCov, IsHidden_DefaultFlag_False)
{
    // Arrange: fresh item has no hidden data

    // Act
    const bool hiddenDefault = item->isHidden();

    // Assert
    EXPECT_FALSE(hiddenDefault);
    EXPECT_EQ(item->row(), -1);   // parentless item
    EXPECT_FALSE(item->hasChildren());
}
