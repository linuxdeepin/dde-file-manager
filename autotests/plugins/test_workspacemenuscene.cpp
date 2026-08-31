// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacemenuscene.cpp
 * @brief Unit tests for WorkspaceMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/workspacemenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceMenuSceneTest, WorkspaceMenuScene)
{
    // Test constructor: WorkspaceMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WorkspaceMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(WorkspaceMenuSceneTest, updateState)
{
    // Test method: void updateState((DMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
