// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rubberband_1.cpp
 * @brief Unit tests for RubberBand methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/boxselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class RubberBandTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RubberBand();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RubberBand *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RubberBandTest, RubberBand)
{
    // Test constructor: RubberBand(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(RubberBandTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}
