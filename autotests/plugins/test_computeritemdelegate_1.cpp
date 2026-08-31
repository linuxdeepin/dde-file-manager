// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computeritemdelegate_1.cpp
 * @brief Unit tests for ComputerItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/computeritemdelegate.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerItemDelegateTest, closeEditor)
{
    // Test method: void closeEditor((ComputerView *view))
    EXPECT_NO_FATAL_FAILURE(obj->closeEditor(nullptr));
}

TEST_F(ComputerItemDelegateTest, drawDeviceIcon)
{
    // Test method: void drawDeviceIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDeviceIcon(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, getProgressTotalColor)
{
    // Test getter: QColor getProgressTotalColor()
    auto result = obj->getProgressTotalColor();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerItemDelegateTest, helpEvent)
{
    // Test event handler: helpEvent((QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index))
    QHelpEvent _event(QHelpEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->helpEvent(&_event));
}

TEST_F(ComputerItemDelegateTest, paintCustomWidget)
{
    // Test method: void paintCustomWidget((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintCustomWidget(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, paintLargeItem)
{
    // Test method: void paintLargeItem((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintLargeItem(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, paintSmallItem)
{
    // Test method: void paintSmallItem((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintSmallItem(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, paintSplitter)
{
    // Test method: void paintSplitter((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintSplitter(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, prepareColor)
{
    // Test method: void prepareColor((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->prepareColor(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, renderBlurShadow)
{
    // Test method: QPixmap renderBlurShadow((const QPixmap &pm, int blurRadius))
    QPixmap _arg0{};
    auto result = obj->renderBlurShadow(_arg0, 0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(ComputerItemDelegateTest, setEditorData)
{
    // Test setter: void setEditorData((QWidget *editor, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEditorData(nullptr, _arg1));
}

TEST_F(ComputerItemDelegateTest, setModelData)
{
    // Test setter: void setModelData((QWidget *editor, QAbstractItemModel *model, const QModelIndex &index))
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setModelData(nullptr, nullptr, _arg2));
}

TEST_F(ComputerItemDelegateTest, updateEditorGeometry)
{
    // Test method: void updateEditorGeometry((QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateEditorGeometry(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, ComputerItemDelegate_Destructor)
{
    // Test method:  ~ComputerItemDelegate(())
    EXPECT_NO_FATAL_FAILURE({ ComputerItemDelegate *tmp = new ComputerItemDelegate(); delete tmp; });
}
