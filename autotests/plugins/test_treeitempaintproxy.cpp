// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_treeitempaintproxy.cpp
 * @brief Unit tests for TreeItemPaintProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/treeitempaintproxy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class TreeItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TreeItemPaintProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TreeItemPaintProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TreeItemPaintProxyTest, arrowRect)
{
    // Test method: QRectF arrowRect((const QRectF &iconRect))
    QRectF _arg0{};
    auto result = obj->arrowRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TreeItemPaintProxyTest, iconRect)
{
    // Test method: QRectF iconRect((const QModelIndex &index, const QRect &itemRect))
    QModelIndex _arg0{};
    QRect _arg1{};
    auto result = obj->iconRect(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}
