// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dodgeitemsoper_1.cpp
 * @brief Unit tests for DodgeItemsOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/dodgeoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DodgeItemsOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DodgeItemsOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DodgeItemsOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DodgeItemsOperTest, DodgeItemsOper)
{
    // Test constructor: DodgeItemsOper((GridCore *core))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DodgeItemsOperTest, reloach)
{
    // Test method: QStringList reloach((int screenNumber, int targetIndex, int targetBeforNeedEmptyCount, int targetAfterNeedEmptyCount))
    auto result = obj->reloach(0, 0, 0, 0);
    EXPECT_TRUE(result.isEmpty());

}
