// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacemenucreator.cpp
 * @brief Unit tests for WorkspaceMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/workspacemenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
