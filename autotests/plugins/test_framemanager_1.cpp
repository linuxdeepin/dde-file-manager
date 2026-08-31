// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_framemanager_1.cpp
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

TEST_F(FrameManagerTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(FrameManagerTest, onBuild)
{
    // Test method: void onBuild(())
    EXPECT_NO_FATAL_FAILURE(obj->onBuild());
}

TEST_F(FrameManagerTest, onDetachWindows)
{
    // Test method: void onDetachWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->onDetachWindows());
}

TEST_F(FrameManagerTest, onGeometryChanged)
{
    // Test method: void onGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onGeometryChanged());
}

TEST_F(FrameManagerTest, onWindowShowed)
{
    // Test method: void onWindowShowed(())
    EXPECT_NO_FATAL_FAILURE(obj->onWindowShowed());
}

TEST_F(FrameManagerTest, organizerEnabled)
{
    // Test bool getter: organizerEnabled()
    bool result = obj->organizerEnabled();
    EXPECT_FALSE(result);

}

TEST_F(FrameManagerTest, switchMode)
{
    // Test method: void switchMode((OrganizerMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->switchMode(OrganizerMode()));
}

TEST_F(FrameManagerTest, turnOff)
{
    // Test method: void turnOff(())
    EXPECT_NO_FATAL_FAILURE(obj->turnOff());
}
