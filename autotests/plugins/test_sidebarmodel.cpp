// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarmodel.cpp
 * @brief Unit tests for SideBarModel Mid-priority methods (dfmplugin-sidebar)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "treemodels/sidebarmodel.h"

using namespace dfmplugin_sidebar;

class SideBarModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(SideBarModelTest, addSubItems)
{
    // Instance method addSubItems
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.addSubItems(QModelIndex(), QList<QUrl>{QUrl("file:///tmp/test")}); });
}

TEST_F(SideBarModelTest, findGroupIndex)
{
    // Instance method findGroupIndex
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.findGroupIndex(QString("test")); (void)r; });
}

TEST_F(SideBarModelTest, groupItems)
{
    // Instance method groupItems
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.groupItems(); (void)r; });
}

TEST_F(SideBarModelTest, onDirectoryRemoved)
{
    // Instance method onDirectoryRemoved
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.onDirectoryRemoved(QUrl("file:///tmp/test"), QUrl("file:///tmp/test")); });
}

TEST_F(SideBarModelTest, onItemCollapsed)
{
    // Instance method onItemCollapsed
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.onItemCollapsed(QModelIndex()); });
}

TEST_F(SideBarModelTest, subItems)
{
    // Instance method subItems
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.subItems(QString("test")); (void)r; });
}

TEST_F(SideBarModelTest, updateRow)
{
    // Instance method updateRow
    SideBarModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.updateRow(QUrl("file:///tmp/test"), ItemInfo()); });
}
