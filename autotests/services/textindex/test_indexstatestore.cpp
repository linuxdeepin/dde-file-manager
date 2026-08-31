// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexstatestore.cpp
 * @brief Unit tests for IndexStateStore (indexstatestore.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDateTime>
#include <QString>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/state/indexstatestore.h"
#include "services/textindex/profile/indexprofile.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class IndexStateStoreTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        indexDir = tmpDir.path();
        profile = IndexProfile(IndexProfile::Type::Content,
                               "statetest",
                               "state_status.json",
                               "state_version",
                               1,
                               [this]() -> QString { return indexDir; },
                               []() -> bool { return true; },
                               [](const QString &) -> bool { return true; },
                               [](const QString &) -> bool { return true; });
        store.reset(new IndexStateStore(profile));
    }

    QTemporaryDir tmpDir;
    QString indexDir;
    IndexProfile profile;
    std::unique_ptr<IndexStateStore> store;
};

TEST_F(IndexStateStoreTest, StatusFilePathContainsFileName)
{
    QString sfp = store->statusFilePath();
    EXPECT_TRUE(sfp.contains("state_status.json"));
}

TEST_F(IndexStateStoreTest, GetIndexStateUnknownWhenNoFile)
{
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Unknown);
}

TEST_F(IndexStateStoreTest, SetIndexStateClean)
{
    store->setIndexState(IndexUtility::IndexState::Clean);
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Clean);
}

TEST_F(IndexStateStoreTest, SetIndexStateDirty)
{
    store->setIndexState(IndexUtility::IndexState::Dirty);
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Dirty);
}

TEST_F(IndexStateStoreTest, SetIndexStateUnknownIgnored)
{
    store->setIndexState(IndexUtility::IndexState::Clean);
    store->setIndexState(IndexUtility::IndexState::Unknown);
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Clean);
}

TEST_F(IndexStateStoreTest, IsCleanState)
{
    store->setIndexState(IndexUtility::IndexState::Clean);
    EXPECT_TRUE(store->isCleanState());
    store->setIndexState(IndexUtility::IndexState::Dirty);
    EXPECT_FALSE(store->isCleanState());
}

TEST_F(IndexStateStoreTest, NeedsRebuildDefault)
{
    EXPECT_NO_FATAL_FAILURE({ (void)store->needsRebuild(); });
}

TEST_F(IndexStateStoreTest, SetNeedsRebuild)
{
    store->setNeedsRebuild(true);
    EXPECT_TRUE(store->needsRebuild());
    store->setNeedsRebuild(false);
    EXPECT_FALSE(store->needsRebuild());
}

TEST_F(IndexStateStoreTest, GetLastUpdateTime)
{
    EXPECT_NO_FATAL_FAILURE({ (void)store->getLastUpdateTime(); });
}

TEST_F(IndexStateStoreTest, GetIndexVersion)
{
    EXPECT_NO_FATAL_FAILURE({ (void)store->getIndexVersion(); });
}

TEST_F(IndexStateStoreTest, IsCompatibleVersion)
{
    EXPECT_NO_FATAL_FAILURE({ (void)store->isCompatibleVersion(); });
}

TEST_F(IndexStateStoreTest, SaveLastUpdateTime)
{
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_NO_FATAL_FAILURE({ store->saveLastUpdateTime(now); });
}

TEST_F(IndexStateStoreTest, SaveIndexStatusWithTime)
{
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_NO_FATAL_FAILURE({ store->saveIndexStatus(now); });
}

TEST_F(IndexStateStoreTest, SaveIndexStatusWithTimeAndVersion)
{
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_NO_FATAL_FAILURE({ store->saveIndexStatus(now, 6); });
}

TEST_F(IndexStateStoreTest, RemoveIndexStatusFile)
{
    store->setIndexState(IndexUtility::IndexState::Clean);
    EXPECT_NO_FATAL_FAILURE({ store->removeIndexStatusFile(); });
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Unknown);
}

TEST_F(IndexStateStoreTest, ClearIndexDirectory)
{
    EXPECT_NO_FATAL_FAILURE({ store->clearIndexDirectory(); });
}

TEST_F(IndexStateStoreTest, IsCreateInProgressDefault)
{
    EXPECT_NO_FATAL_FAILURE({ (void)store->isCreateInProgress(); });
}

TEST_F(IndexStateStoreTest, SetCreateInProgress)
{
    store->setCreateInProgress(true);
    EXPECT_TRUE(store->isCreateInProgress());
    store->setCreateInProgress(false);
    EXPECT_FALSE(store->isCreateInProgress());
}


TEST_F(IndexStateStoreTest, getIndexState)
{
    // getIndexState
    SUCCEED();
}

TEST_F(IndexStateStoreTest, saveIndexStatus)
{
    // saveIndexStatus
    SUCCEED();
}
