// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridbroker.cpp
 * @brief Unit tests for CanvasGridBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasgridbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridBrokerTest, item)
{
    // Test method: QString item((int index, const QPoint &gridPos))
    QPoint _arg1{};
    auto result = obj->item(0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridBrokerTest, items)
{
    // Test method: QStringList items((int index))
    auto result = obj->items(0);
    EXPECT_TRUE(result.isEmpty());

}
