// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_folderviewdelegate.cpp
 * @brief Unit tests for FolderViewDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/folderviewdelegate.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class FolderViewDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FolderViewDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FolderViewDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FolderViewDelegateTest, createCustomOpacityPixmap)
{
    // Test method: QPixmap createCustomOpacityPixmap((const QPixmap &px, float opacity))
    QPixmap _arg0{};
    auto result = obj->createCustomOpacityPixmap(_arg0, 0.0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(FolderViewDelegateTest, helpEvent)
{
    // Test event handler: helpEvent((QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index))
    QHelpEvent _event(QHelpEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->helpEvent(&_event));
}

TEST_F(FolderViewDelegateTest, hideTooltipImmediately)
{
    // Test method: void hideTooltipImmediately(())
    EXPECT_NO_FATAL_FAILURE(obj->hideTooltipImmediately());
}

TEST_F(FolderViewDelegateTest, paintItemIcon)
{
    // Test method: void paintItemIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintItemIcon(nullptr, _arg1, _arg2));
}

TEST_F(FolderViewDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
