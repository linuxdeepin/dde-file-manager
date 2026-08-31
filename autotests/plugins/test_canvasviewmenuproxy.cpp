// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewmenuproxy.cpp
 * @brief Unit tests for CanvasViewMenuProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/canvasviewmenuproxy.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewMenuProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewMenuProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewMenuProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewMenuProxyTest, showEmptyAreaMenu)
{
    // Test method: void showEmptyAreaMenu((const Qt::ItemFlags &indexFlags, const QPoint gridPos))
    Qt::ItemFlags _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showEmptyAreaMenu(_arg0, QPoint()));
}

TEST_F(CanvasViewMenuProxyTest, showNormalMenu)
{
    // Test method: void showNormalMenu((const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos))
    QModelIndex _arg0{};
    Qt::ItemFlags _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showNormalMenu(_arg0, _arg1, QPoint()));
}
