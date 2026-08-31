// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operstate_1.cpp
 * @brief Unit tests for OperState methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/operstate.h"

#include <QTest>

using namespace ddplugin_canvas;

class OperStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperState();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperState *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperStateTest, OperState)
{
    // Test constructor: OperState((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OperStateTest, getContBegin)
{
    // Test getter: QModelIndex getContBegin()
    auto result = obj->getContBegin();
    EXPECT_FALSE(result.isValid());

}

TEST_F(OperStateTest, setView)
{
    // Test setter: void setView((CanvasView *v))
    EXPECT_NO_FATAL_FAILURE(obj->setView(nullptr));
}

TEST_F(OperStateTest, updateExpendedItem)
{
    // Test method: void updateExpendedItem(())
    EXPECT_NO_FATAL_FAILURE(obj->updateExpendedItem());
}
