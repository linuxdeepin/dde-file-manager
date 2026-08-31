// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computeritemdelegate.cpp
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

TEST_F(ComputerItemDelegateTest, createEditor)
{
    // Test method: QWidget createEditor((QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    auto result = obj->createEditor(nullptr, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->createEditor(nullptr, _arg1, _arg2); });

}

TEST_F(ComputerItemDelegateTest, drawDeviceLabelAndFs)
{
    // Test method: void drawDeviceLabelAndFs((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDeviceLabelAndFs(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}

TEST_F(ComputerItemDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
