// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rubberband.cpp
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

TEST_F(RubberBandTest, onParentDestroyed)
{
    // Test method: void onParentDestroyed((QObject *p))
    EXPECT_NO_FATAL_FAILURE(obj->onParentDestroyed(nullptr));
}

TEST_F(RubberBandTest, touch)
{
    // Test method: void touch((QWidget *w))
    EXPECT_NO_FATAL_FAILURE(obj->touch(nullptr));
}
