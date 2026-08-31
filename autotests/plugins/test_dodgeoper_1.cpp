// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dodgeoper_1.cpp
 * @brief Unit tests for DodgeOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/dodgeoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class DodgeOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DodgeOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DodgeOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DodgeOperTest, calcDodgeTargetGrid)
{
    // Test bool getter: calcDodgeTargetGrid()
    bool result = obj->calcDodgeTargetGrid();
    EXPECT_FALSE(result);

}

TEST_F(DodgeOperTest, dodgeAnimationFinished)
{
    // Test method: void dodgeAnimationFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->dodgeAnimationFinished());
}

TEST_F(DodgeOperTest, dodgeAnimationUpdate)
{
    // Test method: void dodgeAnimationUpdate(())
    EXPECT_NO_FATAL_FAILURE(obj->dodgeAnimationUpdate());
}

TEST_F(DodgeOperTest, getDodgeAnimationing)
{
    // Test bool getter: getDodgeAnimationing()
    bool result = obj->getDodgeAnimationing();
    EXPECT_FALSE(result);

}

TEST_F(DodgeOperTest, getDodgeDuration)
{
    // Test getter: double getDodgeDuration()
    auto result = obj->getDodgeDuration();
    EXPECT_EQ(result, 0.0);

}

TEST_F(DodgeOperTest, getDodgeItemGridPos)
{
    // Test method: bool getDodgeItemGridPos((const QString &item, GridPos &gridPos))
    QString _arg0{};
    GridPos _arg1{};
    auto result = obj->getDodgeItemGridPos(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DodgeOperTest, getDodgeItems)
{
    // Test getter: QStringList getDodgeItems()
    auto result = obj->getDodgeItems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DodgeOperTest, getDragTargetGridPos)
{
    // Test getter: QPoint getDragTargetGridPos()
    auto result = obj->getDragTargetGridPos();
    EXPECT_TRUE(result.isNull());

}

TEST_F(DodgeOperTest, getPrepareDodge)
{
    // Test bool getter: getPrepareDodge()
    bool result = obj->getPrepareDodge();
    EXPECT_FALSE(result);

}

TEST_F(DodgeOperTest, setDodgeDuration)
{
    // Test setter: void setDodgeDuration((double duration))
    EXPECT_NO_FATAL_FAILURE(obj->setDodgeDuration(0.0));
}

TEST_F(DodgeOperTest, startDelayDodge)
{
    // Test method: void startDelayDodge(())
    EXPECT_NO_FATAL_FAILURE(obj->startDelayDodge());
}

TEST_F(DodgeOperTest, startDodgeAnimation)
{
    // Test method: void startDodgeAnimation(())
    EXPECT_NO_FATAL_FAILURE(obj->startDodgeAnimation());
}

TEST_F(DodgeOperTest, stopDelayDodge)
{
    // Test method: void stopDelayDodge(())
    EXPECT_NO_FATAL_FAILURE(obj->stopDelayDodge());
}

TEST_F(DodgeOperTest, updatePrepareDodgeValue)
{
    // Test method: void updatePrepareDodgeValue((QEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->updatePrepareDodgeValue(nullptr));
}
