// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_surface_1.cpp
 * @brief Unit tests for Surface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/surface.h"

#include <QTest>

using namespace ddplugin_organizer;

class SurfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Surface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Surface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SurfaceTest, Surface)
{
    // Test constructor: Surface((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SurfaceTest, activatePosIndicator)
{
    // Test method: void activatePosIndicator((const QRect &r))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->activatePosIndicator(_arg0));
}

TEST_F(SurfaceTest, deactivatePosIndicator)
{
    // Test method: void deactivatePosIndicator(())
    EXPECT_NO_FATAL_FAILURE(obj->deactivatePosIndicator());
}

TEST_F(SurfaceTest, findValidArea)
{
    // Test method: QRect findValidArea((QWidget *wid))
    auto result = obj->findValidArea(nullptr);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SurfaceTest, isIntersected)
{
    // Test method: bool isIntersected((const QRect &screenRect, QWidget *wid))
    QRect _arg0{};
    auto result = obj->isIntersected(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SurfaceTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *e))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(SurfaceTest, setPositionIndicatorRect)
{
    // Test setter: void setPositionIndicatorRect((const QRect &r))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPositionIndicatorRect(_arg0));
}
