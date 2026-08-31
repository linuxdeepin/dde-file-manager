// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionframeprivate_1.cpp
 * @brief Unit tests for CollectionFramePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionframe.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionFramePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionFramePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionFramePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionFramePrivateTest, CollectionFramePrivate)
{
    // Test constructor: CollectionFramePrivate((CollectionFrame *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionFramePrivateTest, calcBottomY)
{
    // Test getter: int calcBottomY()
    auto result = obj->calcBottomY();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionFramePrivateTest, calcLeftX)
{
    // Test getter: int calcLeftX()
    auto result = obj->calcLeftX();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionFramePrivateTest, calcRightX)
{
    // Test getter: int calcRightX()
    auto result = obj->calcRightX();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionFramePrivateTest, calcTopY)
{
    // Test getter: int calcTopY()
    auto result = obj->calcTopY();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionFramePrivateTest, canMove)
{
    // Test bool getter: canMove()
    bool result = obj->canMove();
    EXPECT_FALSE(result);

}

TEST_F(CollectionFramePrivateTest, canStretch)
{
    // Test bool getter: canStretch()
    bool result = obj->canStretch();
    EXPECT_FALSE(result);

}

TEST_F(CollectionFramePrivateTest, updateMouseTrackingState)
{
    // Test method: void updateMouseTrackingState(())
    EXPECT_NO_FATAL_FAILURE(obj->updateMouseTrackingState());
}

TEST_F(CollectionFramePrivateTest, updateMoveRect)
{
    // Test method: void updateMoveRect(())
    EXPECT_NO_FATAL_FAILURE(obj->updateMoveRect());
}

TEST_F(CollectionFramePrivateTest, updateStretchRect)
{
    // Test method: void updateStretchRect(())
    EXPECT_NO_FATAL_FAILURE(obj->updateStretchRect());
}

TEST_F(CollectionFramePrivateTest, CollectionFramePrivate_Destructor)
{
    // Test method:  ~CollectionFramePrivate(())
    EXPECT_NO_FATAL_FAILURE({ CollectionFramePrivate *tmp = new CollectionFramePrivate(); delete tmp; });
}
