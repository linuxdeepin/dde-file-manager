// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacemenuscene_1.cpp
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

TEST_F(WorkspaceMenuSceneTest, create)
{
    // Test method: bool create((DMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}
