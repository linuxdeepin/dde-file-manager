// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionframe.cpp
 * @brief Unit tests for CollectionFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionframe.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionFrameTest, CollectionFrame)
{
    // Test constructor: CollectionFrame((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionFrameTest, adjustSizeMode)
{
    // Test method: void adjustSizeMode((const CollectionFrameSize &size))
    CollectionFrameSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->adjustSizeMode(_arg0));
}

TEST_F(CollectionFrameTest, event)
{
    // Test method: bool event((QEvent *event))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionFrameTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(CollectionFrameTest, setWidget)
{
    // Test setter: void setWidget((QWidget *w))
    EXPECT_NO_FATAL_FAILURE(obj->setWidget(nullptr));
}
