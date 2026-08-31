// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dodgeitemsoper.cpp
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

TEST_F(DodgeItemsOperTest, findEmptyBackward)
{
    // Test method: int findEmptyBackward((int screenNum, int index, int targetAfterNeedEmptyCount))
    auto result = obj->findEmptyBackward(0, 0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(DodgeItemsOperTest, findEmptyForward)
{
    // Test method: int findEmptyForward((int screenNum, int index, int targetBeforNeedEmptyCount))
    auto result = obj->findEmptyForward(0, 0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(DodgeItemsOperTest, reloachBackward)
{
    // Test method: QStringList reloachBackward((int screenNum, int start, int end))
    auto result = obj->reloachBackward(0, 0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DodgeItemsOperTest, reloachForward)
{
    // Test method: QStringList reloachForward((int screenNum, int start, int end))
    auto result = obj->reloachForward(0, 0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DodgeItemsOperTest, toIndex)
{
    // Test method: QList<int> toIndex((const int screenNumber, const QList<QPoint> &pos))
    QList<QPoint> _arg1{};
    auto result = obj->toIndex(0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DodgeItemsOperTest, toPos)
{
    // Test method: QPoint toPos((const int screenNumber, const int index))
    auto result = obj->toPos(0, 0);
    EXPECT_TRUE(result.isNull());

}
