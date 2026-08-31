// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelbroker.cpp
 * @brief Unit tests for CanvasModelBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasmodelbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasModelBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasModelBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasModelBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelBrokerTest, index)
{
    // Test method: QModelIndex index((int row))
    auto result = obj->index(0);
    EXPECT_FALSE(result.isValid());

}
