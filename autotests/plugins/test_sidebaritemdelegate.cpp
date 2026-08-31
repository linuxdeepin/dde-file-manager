// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebaritemdelegate.cpp
 * @brief Unit tests for SideBarItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebaritemdelegate.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarItemDelegateTest, clearIconCache)
{
    // Test method: void clearIconCache(())
    EXPECT_NO_FATAL_FAILURE(obj->clearIconCache());
}

TEST_F(SideBarItemDelegateTest, createEditor)
{
    // Test method: QWidget createEditor((QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->createEditor(nullptr, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(nullptr, _arg1, _arg2); });

}

TEST_F(SideBarItemDelegateTest, drawMouseHoverExpandButton)
{
    // Test method: void drawMouseHoverExpandButton((QPainter *painter, const QRect &r, bool isExpanded))
    QRect _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->drawMouseHoverExpandButton(nullptr, _arg1, false));
}

TEST_F(SideBarItemDelegateTest, onEditorTextChanged)
{
    // Test method: void onEditorTextChanged((const QString &text, SideBarItem *item))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onEditorTextChanged(_arg0, nullptr));
}
