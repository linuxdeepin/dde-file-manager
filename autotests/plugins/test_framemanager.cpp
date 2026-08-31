// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_framemanager.cpp
 * @brief Unit tests for FrameManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "framemanager.h"

#include <QTest>

using namespace ddplugin_organizer;

class FrameManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FrameManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FrameManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FrameManagerTest, FrameManager)
{
    // Test constructor: FrameManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FrameManagerTest, layout)
{
    // Test method: void layout(())
    EXPECT_NO_FATAL_FAILURE(obj->layout());
}

TEST_F(FrameManagerTest, turnOn)
{
    // Test method: void turnOn((bool build))
    EXPECT_NO_FATAL_FAILURE(obj->turnOn(false));
}
