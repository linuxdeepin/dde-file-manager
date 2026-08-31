// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitempaintproxy.cpp
 * @brief Unit tests for ListItemPaintProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/listitempaintproxy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ListItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ListItemPaintProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ListItemPaintProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemPaintProxyTest, ListItemPaintProxy)
{
    // Test constructor: ListItemPaintProxy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ListItemPaintProxyTest, iconRect)
{
    // Test method: QRectF iconRect((const QModelIndex &index, const QRect &itemRect))
    QModelIndex _arg0{};
    QRect _arg1{};
    auto result = obj->iconRect(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}
