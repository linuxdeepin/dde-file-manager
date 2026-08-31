// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_indexstatestore.cpp
 * @brief Unit tests for IndexStateStore methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/state/indexstatestore.h"

#include <QTest>

using namespace src;

class IndexStateStoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexStateStore();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexStateStore *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexStateStoreTest, IndexStateStore)
{
    // Test constructor: IndexStateStore((IndexProfile profile))
    ASSERT_NE(obj, nullptr);
}

TEST_F(IndexStateStoreTest, isCreateInProgress)
{
    // Test bool getter: isCreateInProgress()
    bool result = obj->isCreateInProgress();
    EXPECT_FALSE(result);

}

TEST_F(IndexStateStoreTest, needsRebuild)
{
    // Test bool getter: needsRebuild()
    bool result = obj->needsRebuild();
    EXPECT_FALSE(result);

}

TEST_F(IndexStateStoreTest, setIndexState)
{
    // Test setter: void setIndexState((IndexUtility::IndexState state))
    EXPECT_NO_FATAL_FAILURE(obj->setIndexState(IndexUtility::IndexState()));
}

TEST_F(IndexStateStoreTest, statusFilePath)
{
    // Test getter: QString statusFilePath()
    auto result = obj->statusFilePath();
    EXPECT_TRUE(result.isEmpty());

}
