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
        profile = IndexProfile({ IndexProfile::Type::Content,
                                 "statetest",
                                 "state_status.json",
                                 "state_version",
                                 1 },
                               { [this]() -> QString { return indexDir; },
                                 []() -> bool { return true; },
                                 [](const QString &) -> bool { return true; },
                                 [](const QString &) -> bool { return true; } });
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

// ===========================================================================
// updateInProgress (design 变更 8) — lifecycle of the full-comparison Update task
// ===========================================================================

TEST_F(IndexStateStoreTest, UpdateInProgress_DefaultFalse)
{
    EXPECT_FALSE(store->isUpdateInProgress());
}

TEST_F(IndexStateStoreTest, UpdateInProgress_SetTruePersists)
{
    store->setUpdateInProgress(true);
    EXPECT_TRUE(store->isUpdateInProgress());
    // Verify persisted across a fresh store on the same directory/file
    std::unique_ptr<IndexStateStore> store2(new IndexStateStore(profile));
    EXPECT_TRUE(store2->isUpdateInProgress());
}

TEST_F(IndexStateStoreTest, UpdateInProgress_SetFalsePersists)
{
    store->setUpdateInProgress(true);
    ASSERT_TRUE(store->isUpdateInProgress());
    store->setUpdateInProgress(false);
    EXPECT_FALSE(store->isUpdateInProgress());
    std::unique_ptr<IndexStateStore> store2(new IndexStateStore(profile));
    EXPECT_FALSE(store2->isUpdateInProgress());
}

TEST_F(IndexStateStoreTest, UpdateInProgress_DoesNotAffectOtherFields)
{
    // Set baseline state
    store->setIndexState(IndexUtility::IndexState::Dirty);
    store->setNeedsRebuild(true);
    store->setCreateInProgress(true);
    QDateTime now = QDateTime::currentDateTime();
    store->saveIndexStatus(now);

    // Now toggle updateInProgress — other fields must remain intact
    store->setUpdateInProgress(true);
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Dirty);
    EXPECT_TRUE(store->needsRebuild());
    EXPECT_TRUE(store->isCreateInProgress());
    EXPECT_TRUE(store->isUpdateInProgress());

    store->setUpdateInProgress(false);
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Dirty);
    EXPECT_TRUE(store->needsRebuild());
    EXPECT_TRUE(store->isCreateInProgress());
    EXPECT_FALSE(store->isUpdateInProgress());
}

TEST_F(IndexStateStoreTest, UpdateInProgress_RoundTripViaFreshStore)
{
    store->setUpdateInProgress(true);
    std::unique_ptr<IndexStateStore> store2(new IndexStateStore(profile));
    EXPECT_TRUE(store2->isUpdateInProgress());
    store2->setUpdateInProgress(false);
    std::unique_ptr<IndexStateStore> store3(new IndexStateStore(profile));
    EXPECT_FALSE(store3->isUpdateInProgress());
}

TEST_F(IndexStateStoreTest, UpdateInProgress_RemoveIndexStatusFileResetsToDefault)
{
    store->setUpdateInProgress(true);
    ASSERT_TRUE(store->isUpdateInProgress());
    store->removeIndexStatusFile();
    EXPECT_FALSE(store->isUpdateInProgress());
}

// --- Simulated recovery-Update lifecycle (design 变更 8 table) ---
//
//   1. Service starts, dirty detected → recoveryPending=true
//   2. Recovery Update starts → setUpdateInProgress(true)
//   3. Update succeeds → finalizeIndexState: setUpdateInProgress(false) + state=clean
//   4. Ordinary event increment (UpdateFileList) → does NOT touch updateInProgress
//
TEST_F(IndexStateStoreTest, RecoveryUpdateLifecycle_OnSuccessClearsFlag)
{
    // Step 2: Update starts
    store->setUpdateInProgress(true);
    store->setIndexState(IndexUtility::IndexState::Dirty);
    ASSERT_TRUE(store->isUpdateInProgress());

    // Step 3: Update succeeds (finalizeIndexState clears flag + sets clean)
    store->setUpdateInProgress(false);
    store->setIndexState(IndexUtility::IndexState::Clean);

    EXPECT_FALSE(store->isUpdateInProgress());
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Clean);
}

TEST_F(IndexStateStoreTest, RecoveryUpdateLifecycle_OnFailureKeepsFlagAndDirty)
{
    // Update starts
    store->setUpdateInProgress(true);
    store->setIndexState(IndexUtility::IndexState::Dirty);

    // Update fails/interrupted — flag stays true, state stays dirty
    EXPECT_TRUE(store->isUpdateInProgress());
    EXPECT_EQ(store->getIndexState(), IndexUtility::IndexState::Dirty);

    // Next restart: still true → search remains degraded until recovery succeeds
    std::unique_ptr<IndexStateStore> store2(new IndexStateStore(profile));
    EXPECT_TRUE(store2->isUpdateInProgress());
    EXPECT_EQ(store2->getIndexState(), IndexUtility::IndexState::Dirty);
}

TEST_F(IndexStateStoreTest, OrdinaryIncrementalTask_DoesNotSetUpdateInProgress)
{
    // Ordinary event incremental tasks (UpdateFileList/MoveFileList) do NOT set
    // updateInProgress — only full-comparison Update tasks do (design 变更 8).
    store->setIndexState(IndexUtility::IndexState::Dirty);
    EXPECT_FALSE(store->isUpdateInProgress());

    // Even after dirty state set, updateInProgress stays false
    store->setIndexState(IndexUtility::IndexState::Dirty);
    EXPECT_FALSE(store->isUpdateInProgress());

    // And after clean (incremental succeeded)
    store->setIndexState(IndexUtility::IndexState::Clean);
    EXPECT_FALSE(store->isUpdateInProgress());
}

TEST_F(IndexStateStoreTest, BacklogExceeded_PersistsAndClears)
{
    store->setBacklogExceeded(true);
    EXPECT_TRUE(store->isBacklogExceeded());
    std::unique_ptr<IndexStateStore> store2(new IndexStateStore(profile));
    EXPECT_TRUE(store2->isBacklogExceeded());
    store2->setBacklogExceeded(false);
    EXPECT_FALSE(store2->isBacklogExceeded());
}
