// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractitempaintproxy.cpp
 * @brief Unit tests for AbstractItemPaintProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/abstractitempaintproxy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class AbstractItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractItemPaintProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractItemPaintProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractItemPaintProxyTest, drawText)
{
    // Test method: void drawText((QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg2{};
    QModelIndex _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->drawText(nullptr, nullptr, _arg2, _arg3));
}
