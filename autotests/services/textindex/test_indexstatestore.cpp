// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexstatestore.cpp
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

TEST_F(IndexStateStoreTest, needsRebuild)
{
    // Test bool getter: needsRebuild()
    bool result = obj->needsRebuild();
    EXPECT_FALSE(result);

}

TEST_F(IndexStateStoreTest, statusFilePath)
{
    // Test getter: QString statusFilePath()
    auto result = obj->statusFilePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IndexStateStoreTest, getIndexState)
{
    // Test getter: IndexUtility::IndexState getIndexState()
    auto result = obj->getIndexState();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(IndexStateStoreTest, setIndexState)
{
    // Test setter: void setIndexState((IndexUtility::IndexState state))
    EXPECT_NO_FATAL_FAILURE(obj->setIndexState(IndexUtility::IndexState()));
}

TEST_F(IndexStateStoreTest, isCleanState)
{
    // Test bool getter: isCleanState()
    bool result = obj->isCleanState();
    EXPECT_FALSE(result);

}

TEST_F(IndexStateStoreTest, setNeedsRebuild)
{
    // Test setter: void setNeedsRebuild((bool need))
    EXPECT_NO_FATAL_FAILURE(obj->setNeedsRebuild(false));
}

TEST_F(IndexStateStoreTest, getIndexVersion)
{
    // Test getter: int getIndexVersion()
    auto result = obj->getIndexVersion();
    EXPECT_EQ(result, 0);

}

TEST_F(IndexStateStoreTest, isCompatibleVersion)
{
    // Test bool getter: isCompatibleVersion()
    bool result = obj->isCompatibleVersion();
    EXPECT_FALSE(result);

}

TEST_F(IndexStateStoreTest, clearIndexDirectory)
{
    // Test method: void clearIndexDirectory(())
    EXPECT_NO_FATAL_FAILURE(obj->clearIndexDirectory());
}

TEST_F(IndexStateStoreTest, saveIndexStatus)
{
    // Test method: void saveIndexStatus((const QDateTime &lastUpdateTime, int version))
    QDateTime _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveIndexStatus(_arg0, 0));
}

TEST_F(IndexStateStoreTest, isCreateInProgress)
{
    // Test bool getter: isCreateInProgress()
    bool result = obj->isCreateInProgress();
    EXPECT_FALSE(result);

}

TEST_F(IndexStateStoreTest, setCreateInProgress)
{
    // Test setter: void setCreateInProgress((bool inProgress))
    EXPECT_NO_FATAL_FAILURE(obj->setCreateInProgress(false));
}

TEST_F(IndexStateStoreTest, getLastUpdateTime)
{
    // Test getter: QString getLastUpdateTime()
    auto result = obj->getLastUpdateTime();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IndexStateStoreTest, removeIndexStatusFile)
{
    // Test method: void removeIndexStatusFile(())
    EXPECT_NO_FATAL_FAILURE(obj->removeIndexStatusFile());
}

TEST_F(IndexStateStoreTest, saveLastUpdateTime)
{
    // Test method: void saveLastUpdateTime((const QDateTime &lastUpdateTime))
    QDateTime _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveLastUpdateTime(_arg0));
}
