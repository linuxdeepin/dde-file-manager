// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgroundservice.cpp
 * @brief Unit tests for BackgroundService methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgroundservice.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundService();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundService *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundServiceTest, getCurrentWorkspaceIndex)
{
    // Test getter: int getCurrentWorkspaceIndex()
    auto result = obj->getCurrentWorkspaceIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(BackgroundServiceTest, getDefaultBackground)
{
    // Test getter: QString getDefaultBackground()
    auto result = obj->getDefaultBackground();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BackgroundServiceTest, onWorkspaceSwitched)
{
    // Test method: void onWorkspaceSwitched((int from, int to))
    EXPECT_NO_FATAL_FAILURE(obj->onWorkspaceSwitched(0, 0));
}

TEST_F(BackgroundServiceTest, BackgroundService_Destructor)
{
    // Test method:  ~BackgroundService(())
    EXPECT_NO_FATAL_FAILURE({ BackgroundService *tmp = new BackgroundService(); delete tmp; });
}
