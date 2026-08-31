// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortanimationoper_1.cpp
 * @brief Unit tests for SortAnimationOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/sortanimationoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class SortAnimationOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortAnimationOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortAnimationOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortAnimationOperTest, calcMoveTargetGrid)
{
    // Test method: bool calcMoveTargetGrid((const QStringList &existItems))
    QStringList _arg0{};
    auto result = obj->calcMoveTargetGrid(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SortAnimationOperTest, findPixmap)
{
    // Test method: QPixmap findPixmap((const QString &item, int screenNum))
    QString _arg0{};
    auto result = obj->findPixmap(_arg0, 0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(SortAnimationOperTest, getMoveAnimationing)
{
    // Test bool getter: getMoveAnimationing()
    bool result = obj->getMoveAnimationing();
    EXPECT_FALSE(result);

}

TEST_F(SortAnimationOperTest, getMoveDuration)
{
    // Test getter: double getMoveDuration()
    auto result = obj->getMoveDuration();
    EXPECT_EQ(result, 0.0);

}

TEST_F(SortAnimationOperTest, getMoveItemGridPos)
{
    // Test method: bool getMoveItemGridPos((const QString &item, GridPos &gridPos))
    QString _arg0{};
    GridPos _arg1{};
    auto result = obj->getMoveItemGridPos(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SortAnimationOperTest, getMoveItems)
{
    // Test getter: QStringList getMoveItems()
    auto result = obj->getMoveItems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SortAnimationOperTest, getOriginItemGridPos)
{
    // Test method: bool getOriginItemGridPos((const QString &item, GridPos &gridPos))
    QString _arg0{};
    GridPos _arg1{};
    auto result = obj->getOriginItemGridPos(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SortAnimationOperTest, instance)
{
    // Test getter: SortAnimationOper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(SortAnimationOperTest, moveAnimationFinished)
{
    // Test method: void moveAnimationFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->moveAnimationFinished());
}

TEST_F(SortAnimationOperTest, moveAnimationUpdate)
{
    // Test method: void moveAnimationUpdate(())
    EXPECT_NO_FATAL_FAILURE(obj->moveAnimationUpdate());
}

TEST_F(SortAnimationOperTest, setItemPixmap)
{
    // Test setter: void setItemPixmap((const QString &item, const QPixmap &pix, int screenNum))
    QString _arg0{};
    QPixmap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setItemPixmap(_arg0, _arg1, 0));
}

TEST_F(SortAnimationOperTest, setMoveDuration)
{
    // Test setter: void setMoveDuration((double duration))
    EXPECT_NO_FATAL_FAILURE(obj->setMoveDuration(0.0));
}

TEST_F(SortAnimationOperTest, setMoveValue)
{
    // Test setter: void setMoveValue((const QStringList &moveItems))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setMoveValue(_arg0));
}

TEST_F(SortAnimationOperTest, startDelayMove)
{
    // Test method: void startDelayMove(())
    EXPECT_NO_FATAL_FAILURE(obj->startDelayMove());
}

TEST_F(SortAnimationOperTest, stopDelayMove)
{
    // Test method: void stopDelayMove(())
    EXPECT_NO_FATAL_FAILURE(obj->stopDelayMove());
}

TEST_F(SortAnimationOperTest, tryMove)
{
    // Test method: bool tryMove((const QStringList &existItems))
    QStringList _arg0{};
    auto result = obj->tryMove(_arg0);
    EXPECT_FALSE(result);

}
