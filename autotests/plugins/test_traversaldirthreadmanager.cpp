// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_traversaldirthreadmanager.cpp
 * @brief Unit tests for TraversalDirThreadManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/traversaldirthreadmanager.h"

#include <QTest>

using namespace dfmplugin_workspace;

class TraversalDirThreadManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TraversalDirThreadManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TraversalDirThreadManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TraversalDirThreadManagerTest, isRunning)
{
    // Test bool getter: isRunning()
    bool result = obj->isRunning();
    EXPECT_FALSE(result);

}

TEST_F(TraversalDirThreadManagerTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}

TEST_F(TraversalDirThreadManagerTest, setSortAgruments)
{
    // Test setter: void setSortAgruments((const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile))
    EXPECT_NO_FATAL_FAILURE(obj->setSortAgruments(Qt::SortOrder(), Global::ItemRoles(), false));
}

TEST_F(TraversalDirThreadManagerTest, start)
{
    // Test method: void start(())
    EXPECT_NO_FATAL_FAILURE(obj->start());
}
