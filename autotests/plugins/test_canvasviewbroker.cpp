// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewbroker.cpp
 * @brief Unit tests for CanvasViewBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasviewbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewBrokerTest, gridPos)
{
    // Test method: QPoint gridPos((int idx, const QPoint &viewPoint))
    QPoint _arg1{};
    auto result = obj->gridPos(0, _arg1);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CanvasViewBrokerTest, select)
{
    // Test method: void select((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->select(_arg0));
}

TEST_F(CanvasViewBrokerTest, selectedUrls)
{
    // Test method: QList<QUrl> selectedUrls((int idx))
    auto result = obj->selectedUrls(0);
    EXPECT_TRUE(result.isEmpty());

}
