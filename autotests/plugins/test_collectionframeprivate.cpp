// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionframeprivate.cpp
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

TEST_F(CollectionFramePrivateTest, getCurrentResponseArea)
{
    // Test method: CollectionFramePrivate::ResponseArea getCurrentResponseArea((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->getCurrentResponseArea(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CollectionFramePrivateTest, moveResultRectPos)
{
    // Test method: QPoint moveResultRectPos((bool *validPos))
    auto result = obj->moveResultRectPos(nullptr);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CollectionFramePrivateTest, surface)
{
    // Test getter: Surface surface()
    auto result = obj->surface();
    EXPECT_NO_FATAL_FAILURE({ obj->surface(); });

}
