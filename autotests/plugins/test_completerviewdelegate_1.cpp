// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_completerviewdelegate_1.cpp
 * @brief Unit tests for CompleterViewDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/completerviewdelegate.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CompleterViewDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CompleterViewDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CompleterViewDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CompleterViewDelegateTest, createCustomOpacityPixmap)
{
    // Test method: QPixmap createCustomOpacityPixmap((const QPixmap &px, float opacity))
    QPixmap _arg0{};
    auto result = obj->createCustomOpacityPixmap(_arg0, 0.0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CompleterViewDelegateTest, paintItemIcon)
{
    // Test method: void paintItemIcon((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintItemIcon(nullptr, _arg1, _arg2));
}

TEST_F(CompleterViewDelegateTest, sizeHint)
{
    // Test method: QSize sizeHint((const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg0{};
    QModelIndex _arg1{};
    auto result = obj->sizeHint(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
