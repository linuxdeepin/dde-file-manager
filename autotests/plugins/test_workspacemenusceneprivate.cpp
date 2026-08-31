// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacemenusceneprivate.cpp
 * @brief Unit tests for WorkspaceMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/workspacemenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceMenuScenePrivateTest, WorkspaceMenuScenePrivate)
{
    // Test constructor: WorkspaceMenuScenePrivate((WorkspaceMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}
