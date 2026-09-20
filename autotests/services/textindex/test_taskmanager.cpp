// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskmanager.cpp
 * @brief Additional TaskManager tests: enqueueCompensationTask (empty/valid),
 *        applyDirectoryMovePlans (empty/no-directory-move/single-directory),
 *        onTaskProgress with null currentTask, startTask with empty pathList,
 *        startFileListTask with empty fileList, startFileMoveTask with empty
 *        move map — all early-return paths without spawning threads.
 */

#include <gtest/gtest.h>
#include <QDateTime>
#include <QTemporaryDir>
#include <QString>
#include <QStringList>
#include <QHash>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/state/indexstatestore.h"
#include "services/textindex/task/taskmanager.h"
#include "services/textindex/task/indextask.h"
#include "services/textindex/env/envdetector.h"
#include "services/textindex/utils/indexutility.h"

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

class TaskManagerTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        runtime = std::make_unique<IndexRuntime>(
            IndexProfile({ IndexProfile::Type::Content, "tm13", "tm13_status.json", "tm13_ver", 1 },
                         { [this]() -> QString { return tmp.path(); },
                           []() -> bool { return true; },
                           [](const QString &) -> bool { return true; },
                           [](const QString &) -> bool { return true; } }));
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);
    }
};

TEST_F(TaskManagerTest, EnqueueCompensationTaskEmptyPathsReturnsFalse)
{
    EXPECT_FALSE(mgr->enqueueCompensationTask({}));
}

TEST_F(TaskManagerTest, EnqueueCompensationTaskWithPathsReturnsTrue)
{
    EXPECT_TRUE(mgr->enqueueCompensationTask({"/tmp/a.txt"}));
    EXPECT_TRUE(mgr->hasQueuedTasks());
}

TEST_F(TaskManagerTest, ApplyDirectoryMovePlansEmptyReturnsEmpty)
{
    QHash<QString, QString> empty;
    QStringList result = mgr->applyDirectoryMovePlans(empty);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TaskManagerTest, ApplyDirectoryMovePlansNonDirectoryReturnsEmpty)
{
    QHash<QString, QString> moves { {"/old/file.txt", "/new/file.txt"} };
    QStringList result = mgr->applyDirectoryMovePlans(moves);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TaskManagerTest, ApplyDirectoryMovePlansDirectoryMove)
{
    QHash<QString, QString> moves { {"/old/dir/", "/new/dir/"} };
    QStringList result = mgr->applyDirectoryMovePlans(moves);
    // compensationPaths has at least /new/dir/
    EXPECT_GE(result.size(), 1);
}

TEST_F(TaskManagerTest, OnTaskProgressWithNullTaskIsNoOp)
{
    EXPECT_NO_FATAL_FAILURE({ mgr->onTaskProgress(IndexTask::Type::Create, 10, 100); });
}

TEST_F(TaskManagerTest, StartTaskEmptyPathListReturnsFalse)
{
    // startTask(QString path) with empty path should fail
    EXPECT_FALSE(mgr->startTask(IndexTask::Type::Create, QString()));
}

TEST_F(TaskManagerTest, StartFileListTaskEmptyListReturnsFalse)
{
    EXPECT_FALSE(mgr->startFileListTask(IndexTask::Type::CreateFileList, QStringList()));
}

TEST_F(TaskManagerTest, StartFileMoveTaskEmptyMovesReturnsFalse)
{
    QHash<QString, QString> empty;
    EXPECT_FALSE(mgr->startFileMoveTask(empty));
}

// ===========================================================================
// Filename profile: env policy exemption (design 3.2.2 / 变更 4) and
// updateInProgress lifecycle (design 变更 8).
// ===========================================================================

class TaskManagerFilenameTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };
    stub_ext::StubExt stub;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        // Redirect Global::fileNameIndexDirectory to the temp dir so the
        // filename-profile runtime does not touch the real $HOME index path.
        stub.set_lamda(ADDR(Global, fileNameIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/fn-index";
                       });
        stub.set_lamda(ADDR(Global, isFileNameIndexDirectoryAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(ADDR(Global, isPathInFileNameIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });
        // Stub content/ocr dirs too (filename blacklistProvider references them).
        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/c-index";
                       });
        stub.set_lamda(ADDR(Global, ocrTextIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/o-index";
                       });
        stub.set_lamda(ADDR(Global, defaultIndexedDirectory),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() };
                       });
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });
        stub.set_lamda(ADDR(Global, isFileNameIndexReadyForSearch),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return false;
                       });
        // Stub AnythingConfigWatcher::defaultAnythingIndexPaths so
        // IndexUtility::isDefaultIndexedDirectory(tmp.path()) returns true
        // (startTask validates all paths are in the indexed-dir list).
        stub.set_lamda(ADDR(IndexUtility::AnythingConfigWatcher, defaultAnythingIndexPaths),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() };
                       });

        // Use the real filename() factory — it wires the full policy (env exemption,
        // 1s collection window, existing commit mechanism, 30s recovery delay,
        // hidden-file indexing, blacklist).
        runtime = std::make_unique<IndexRuntime>(IndexProfile::filename());
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);
    }
};

TEST_F(TaskManagerFilenameTest, CanRun_BypassesEnvChecksOnBattery)
{
    // Filename profile should be runnable even on battery (env-exempt)
    EnvState batteryEnv { /*onBattery*/ true, /*powerSaveMode*/ true, /*idle*/ false };
    // Heavy grade would normally be blocked on battery; filename bypasses.
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Heavy, false, batteryEnv));
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Medium, false, batteryEnv));
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Light, false, batteryEnv));
}

TEST_F(TaskManagerFilenameTest, CanRun_BypassesEnvChecksWhenNotIdle)
{
    EnvState busyEnv { false, false, /*idle*/ false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Heavy, false, busyEnv));
}

TEST_F(TaskManagerFilenameTest, CanRun_ManualGradeAlwaysRunnable)
{
    EnvState hostileEnv { true, true, false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Manual, false, hostileEnv));
}

TEST_F(TaskManagerFilenameTest, StartUpdate_SetsUpdateInProgress)
{
    // Starting an Update task on the filename profile must persist updateInProgress=true
    // (design 变更 8) so that isReady flips to false and search degrades to Realtime
    // during the full-comparison scan.
    ASSERT_FALSE(runtime->stateStore().isUpdateInProgress());
    // The temp dir is the "indexed directory"; startTask requires a path within scope.
    // We avoid real traversal by not waiting for completion — the flag is set at start.
    EXPECT_NO_FATAL_FAILURE({ (void)mgr->startTask(IndexTask::Type::Update, tmp.path()); });
    // Flag should be set regardless of whether the task actually started (queue/blocked).
    // Give the worker a moment then stop to keep the test deterministic.
    mgr->stopCurrentTask();
    EXPECT_TRUE(runtime->stateStore().isUpdateInProgress())
        << "Update task must set updateInProgress (design 变更 8)";
}

TEST_F(TaskManagerFilenameTest, StartCreate_DoesNotSetUpdateInProgress)
{
    // Create tasks set createInProgress, NOT updateInProgress (they are distinct windows)
    ASSERT_FALSE(runtime->stateStore().isUpdateInProgress());
    EXPECT_NO_FATAL_FAILURE({ (void)mgr->startTask(IndexTask::Type::Create, tmp.path()); });
    mgr->stopCurrentTask();
    // updateInProgress should remain false for a Create task
    EXPECT_FALSE(runtime->stateStore().isUpdateInProgress());
    // createInProgress should be true (existing behavior)
    EXPECT_TRUE(runtime->stateStore().isCreateInProgress());
}

TEST_F(TaskManagerFilenameTest, StartFileListTask_DoesNotSetUpdateInProgress)
{
    // Ordinary event incremental tasks (UpdateFileList/MoveFileList) must NOT set
    // updateInProgress — only full-comparison Update tasks do (design 变更 8 table).
    ASSERT_FALSE(runtime->stateStore().isUpdateInProgress());
    QStringList files { tmp.path() + "/a.txt", tmp.path() + "/b.txt" };
    EXPECT_NO_FATAL_FAILURE({ (void)mgr->startFileListTask(IndexTask::Type::UpdateFileList, files); });
    mgr->stopCurrentTask();
    EXPECT_FALSE(runtime->stateStore().isUpdateInProgress());
}

TEST_F(TaskManagerFilenameTest, StartFileMoveTask_DoesNotSetUpdateInProgress)
{
    ASSERT_FALSE(runtime->stateStore().isUpdateInProgress());
    QHash<QString, QString> moves { { tmp.path() + "/a.txt", tmp.path() + "/b.txt" } };
    EXPECT_NO_FATAL_FAILURE({ (void)mgr->startFileMoveTask(moves); });
    mgr->stopCurrentTask();
    EXPECT_FALSE(runtime->stateStore().isUpdateInProgress());
}

TEST_F(TaskManagerFilenameTest, RecoveryPendingLifecycle)
{
    // Verify the recoveryPending memory flag API used at startup (design 5.8).
    EXPECT_FALSE(mgr->isRecoveryPending());
    mgr->setRecoveryPending(true);
    EXPECT_TRUE(mgr->isRecoveryPending());
    mgr->setRecoveryPending(false);
    EXPECT_FALSE(mgr->isRecoveryPending());
}

TEST_F(TaskManagerFilenameTest, CurrentIndexStatus_NoTaskNoQueue)
{
    // With no running/queued task, status reflects idle/pending-work state.
    // Filename profile with a fresh (empty) state store → no lastUpdateTime → dbExists=false
    // → heavy pending → "WaitingUpgrade".
    QString status = mgr->currentIndexStatus();
    EXPECT_FALSE(status.isEmpty());
}

TEST_F(TaskManagerFilenameTest, StartupResetsStaleUpdateInProgressOnCleanState)
{
    // Reproduce the stale-flag path: an Update task failed/interrupted, then an
    // ordinary incremental task succeeded and set Clean without clearing
    // updateInProgress. On next startup TaskManager must defensively reset the
    // flag — otherwise startup detection sees Clean, recovery is skipped, and
    // search stays degraded to Realtime forever.
    {
        IndexStateStore store(IndexProfile::filename());
        store.saveLastUpdateTime(QDateTime::currentDateTime());
        store.setIndexState(IndexUtility::IndexState::Clean);
        store.setUpdateInProgress(true);
        ASSERT_TRUE(store.isUpdateInProgress());
        ASSERT_EQ(store.getIndexState(), IndexUtility::IndexState::Clean);
    }

    runtime.reset();   // release the old runtime (stops its worker thread)
    runtime = std::make_unique<IndexRuntime>(IndexProfile::filename());
    EXPECT_FALSE(runtime->stateStore().isUpdateInProgress())
        << "Stale updateInProgress on clean state must be reset at startup";
    EXPECT_EQ(runtime->stateStore().getIndexState(), IndexUtility::IndexState::Clean);
}

TEST_F(TaskManagerFilenameTest, StartupKeepsUpdateInProgressOnDirtyState)
{
    // dirty + updateInProgress is the legitimate crash-recovery state: the flag
    // must survive startup so the pending recovery Update keeps covering the
    // degraded-search window until it succeeds.
    {
        IndexStateStore store(IndexProfile::filename());
        store.saveLastUpdateTime(QDateTime::currentDateTime());
        store.setIndexState(IndexUtility::IndexState::Dirty);
        store.setUpdateInProgress(true);
        ASSERT_TRUE(store.isUpdateInProgress());
        ASSERT_EQ(store.getIndexState(), IndexUtility::IndexState::Dirty);
    }

    runtime.reset();
    runtime = std::make_unique<IndexRuntime>(IndexProfile::filename());
    EXPECT_TRUE(runtime->stateStore().isUpdateInProgress())
        << "dirty + updateInProgress must be kept for recovery";
    EXPECT_EQ(runtime->stateStore().getIndexState(), IndexUtility::IndexState::Dirty);
}

TEST_F(TaskManagerFilenameTest, BurstAccumulationTriggersBacklogExceeded)
{
    // 设计 5.7.4：大突发（静默期内累计 ≥ 5000 文件）必须置位 backlogExceeded，
    // 即使每个 1s 收集窗口的批次都远小于阈值（波间排空、task 层瞬时积压恒低）。
    ASSERT_FALSE(runtime->stateStore().isBacklogExceeded());

    constexpr int kBatchSize = 1000;
    for (int i = 0; i < 5; ++i) {
        QStringList batch;
        batch.reserve(kBatchSize);
        for (int j = 0; j < kBatchSize; ++j)
            batch.append(tmp.path() + QString("/burst%1_%2.txt").arg(i).arg(j));
        EXPECT_TRUE(mgr->enqueueCompensationTask(batch));

        if (i < 4) {
            EXPECT_FALSE(runtime->stateStore().isBacklogExceeded())
                << "must not trigger below threshold at batch " << i;
        }
    }

    EXPECT_TRUE(runtime->stateStore().isBacklogExceeded())
        << "cumulative burst >= 5000 must trigger degradation";
}

TEST_F(TaskManagerFilenameTest, SmallBurstDoesNotTriggerBacklogExceeded)
{
    // 小批量（远低于阈值）不应触发降级——普通小变化保持索引搜索。
    ASSERT_FALSE(runtime->stateStore().isBacklogExceeded());

    QStringList batch;
    batch.reserve(1000);
    for (int j = 0; j < 1000; ++j)
        batch.append(tmp.path() + QString("/small%1.txt").arg(j));
    EXPECT_TRUE(mgr->enqueueCompensationTask(batch));

    EXPECT_FALSE(runtime->stateStore().isBacklogExceeded());
}
