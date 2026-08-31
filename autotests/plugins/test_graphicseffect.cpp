// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_graphicseffect.cpp
 * @brief Unit tests for GraphicsEffect methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionview.h"

#include <QTest>

using namespace ddplugin_organizer;

class GraphicsEffectTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GraphicsEffect();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GraphicsEffect *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GraphicsEffectTest, GraphicsEffect)
{
    // Test constructor: GraphicsEffect((CollectionView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(GraphicsEffectTest, boundingRectFor)
{
    // Test method: QRectF boundingRectFor((const QRectF &sourceRect))
    QRectF _arg0{};
    auto result = obj->boundingRectFor(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(GraphicsEffectTest, sourceChanged)
{
    // Test method: void sourceChanged((QGraphicsEffect::ChangeFlags flags))
    EXPECT_NO_FATAL_FAILURE(obj->sourceChanged(QGraphicsEffect::ChangeFlags()));
}
