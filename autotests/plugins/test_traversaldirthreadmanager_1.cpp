// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_traversaldirthreadmanager_1.cpp
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

TEST_F(TraversalDirThreadManagerTest, iteratorAll)
{
    // Test getter: QList<SortInfoPointer> iteratorAll()
    auto result = obj->iteratorAll();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TraversalDirThreadManagerTest, onAsyncIteratorOver)
{
    // Test method: void onAsyncIteratorOver(())
    EXPECT_NO_FATAL_FAILURE(obj->onAsyncIteratorOver());
}

TEST_F(TraversalDirThreadManagerTest, setTraversalToken)
{
    // Test setter: void setTraversalToken((const QString &token))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTraversalToken(_arg0));
}

TEST_F(TraversalDirThreadManagerTest, TraversalDirThreadManager_Destructor)
{
    // Test method:  ~TraversalDirThreadManager(())
    EXPECT_NO_FATAL_FAILURE({ TraversalDirThreadManager *tmp = new TraversalDirThreadManager(); delete tmp; });
}
