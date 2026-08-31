// SPDX-FileCopyrightText: 2026 UnionTech Software Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskmanager_grading.cpp
 * @brief Unit tests for the core scheduling business logic of TaskManager:
 *        canRun() (grade×environment matrix), gradeFileListTask() (threshold
 *        boundaries), currentIndexStatus() (status string generation),
 *        gradePriority(), gradeToString().
 *
 *        These tests verify the decision logic that maps task grades and
 *        environment states to run/queue/preempt decisions, covering
 *        acceptance criteria AC-020 ~ AC-040 from the strategy-optimization
 *        requirements document.
 *
 *        Private members are accessed directly (the test build uses
 *        -fno-access-control).  EnvDetector is a singleton whose m_state
 *        can be set directly to simulate different environment conditions.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/task/taskmanager.h"
#include "services/textindex/task/indextask.h"
#include "services/textindex/env/envdetector.h"
#include "services/textindex/utils/indexutility.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// ---------------------------------------------------------------------------
// Fixture – creates a Content-profile runtime in a temp directory.
// ---------------------------------------------------------------------------

class TaskManagerGradingTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        runtime = std::make_unique<IndexRuntime>(
            IndexProfile(IndexProfile::Type::Content, "grade", "grade_status.json",
                         "grade_ver", 1,
                         [this]() -> QString { return tmp.path(); },
                         []() -> bool { return true; },
                         [](const QString &) -> bool { return true; },
                         [](const QString &) -> bool { return true; }));
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);

        // Reset EnvDetector singleton to a clean default state before each test.
        EnvDetector::instance().m_state = EnvState {};
    }

    void TearDown() override
    {
        EnvDetector::instance().m_state = EnvState {};
    }

    /// Helper: create a sparse temp file of the given size in bytes.
    QString createTempFile(qint64 sizeBytes)
    {
        auto *f = new QTemporaryFile();
        f->setAutoRemove(false);
        f->open();
        if (sizeBytes > 0)
            f->resize(sizeBytes);
        f->close();
        QString path = f->fileName();
        delete f;
        return path;
    }
};

// ===========================================================================
// canRun() – the grade × environment decision matrix
//   Light:  continues on battery, pauses on power-save, no idle requirement
//   Medium: pauses on battery, pauses on power-save, requires idle
//   Heavy:  pauses on battery, pauses on power-save, requires idle
//   Manual: bypasses all environment checks
//   forceBypass: bypasses all environment checks
// ===========================================================================

TEST_F(TaskManagerGradingTest, CanRun_Light_OnAc_NoPowersave_NoIdleRequirement)
{
    EnvState env { false, false, false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Light, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Light_OnBattery_Continues)
{
    EnvState env { true, false, false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Light, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Light_PowerSave_Pauses)
{
    EnvState env { false, true, false };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Light, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Light_DoesNotRequireIdle)
{
    EnvState env { false, false, false };   // idle = false
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Light, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Light_BatteryAndPowerSave_Pauses)
{
    EnvState env { true, true, false };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Light, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Medium_OnAc_NoPowersave_Idle_Runs)
{
    EnvState env { false, false, true };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Medium, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Medium_OnBattery_Pauses)
{
    EnvState env { true, false, true };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Medium, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Medium_PowerSave_Pauses)
{
    EnvState env { false, true, true };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Medium, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Medium_NotIdle_Pauses)
{
    EnvState env { false, false, false };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Medium, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Heavy_OnAc_NoPowersave_Idle_Runs)
{
    EnvState env { false, false, true };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Heavy, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Heavy_OnBattery_Pauses)
{
    EnvState env { true, false, true };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Heavy, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Heavy_PowerSave_Pauses)
{
    EnvState env { false, true, true };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Heavy, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Heavy_NotIdle_Pauses)
{
    EnvState env { false, false, false };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Heavy, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Manual_PowerSave_Pauses)
{
    EnvState env { true, true, false };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::Manual, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Manual_NoPowerSave_Runs)
{
    EnvState env { true, false, false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Manual, false, env));
}

TEST_F(TaskManagerGradingTest, CanRun_Manual_ForceBypass_BypassesAll)
{
    EnvState env { true, true, false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Manual, true, env));
}

TEST_F(TaskManagerGradingTest, CanRun_ForceBypass_BypassesAll)
{
    EnvState env { true, true, false };
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Heavy, true, env));
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Medium, true, env));
    EXPECT_TRUE(mgr->canRun(IndexTask::Grade::Light, true, env));
}

TEST_F(TaskManagerGradingTest, CanRun_NoneGrade_NeverRuns)
{
    EnvState env { false, false, true };
    EXPECT_FALSE(mgr->canRun(IndexTask::Grade::None, false, env));
}

// ===========================================================================
// gradeFileListTask() – threshold boundary testing
//   Text profile: count threshold = 100, size threshold = 200 MB
//   OCR profile:  count threshold = 30
//   Either count > threshold OR size > threshold → Medium
// ===========================================================================

TEST_F(TaskManagerGradingTest, GradeFileList_SingleFile_Light)
{
    QStringList files { "/tmp/single.txt" };
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Light);
}

TEST_F(TaskManagerGradingTest, GradeFileList_AtCountThreshold_Light)
{
    // Exactly 100 files (text threshold) → Light, not Medium (boundary)
    QStringList files;
    for (int i = 0; i < 100; ++i)
        files << QString("/tmp/file_%1.txt").arg(i);
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Light);
}

TEST_F(TaskManagerGradingTest, GradeFileList_AboveCountThreshold_Medium)
{
    // 101 files → Medium
    QStringList files;
    for (int i = 0; i < 101; ++i)
        files << QString("/tmp/file_%1.txt").arg(i);
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Medium);
}

TEST_F(TaskManagerGradingTest, GradeFileList_SizeOverThreshold_Medium)
{
    // Few files but total size > 200 MB → Medium
    // Create a real 201 MB temp file (sparse — seek + write 1 byte)
    QString bigFile = createTempFile(201 * 1024 * 1024);
    QStringList files { bigFile };
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Medium);
    QFile::remove(bigFile);
}

TEST_F(TaskManagerGradingTest, GradeFileList_SizeAtThreshold_Light)
{
    // Total size exactly at 200 MB → Light (boundary, not exceeding)
    QString file200MB = createTempFile(200 * 1024 * 1024);
    QStringList files { file200MB };
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Light);
    QFile::remove(file200MB);
}

TEST_F(TaskManagerGradingTest, GradeFileList_AnyConditionOver_Medium)
{
    // 99 files (under count threshold) but one big file pushes size over → Medium
    QStringList files;
    for (int i = 0; i < 99; ++i)
        files << QString("/tmp/small_%1.txt").arg(i);
    QString bigFile = createTempFile(201 * 1024 * 1024);
    files << bigFile;
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Medium);
    QFile::remove(bigFile);
}

TEST_F(TaskManagerGradingTest, GradeFileList_RemoveFileList_AlwaysLight)
{
    // RemoveFileList type should always be Light regardless of count
    QStringList files;
    for (int i = 0; i < 500; ++i)
        files << QString("/tmp/del_%1.txt").arg(i);
    // The caller (startFileListTask) forces Light for RemoveFileList;
    // gradeFileListTask itself still grades by thresholds, but the caller
    // overrides. Here we test the raw grading function's behavior.
    // With 500 files, raw grading returns Medium:
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Medium);
}

// ===========================================================================
// gradeFileListTask() – OCR profile thresholds
// ===========================================================================

class TaskManagerGradingOcrTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        runtime = std::make_unique<IndexRuntime>(
            IndexProfile(IndexProfile::Type::Ocr, "ocr_grade", "ocr_grade_status.json",
                         "ocr_grade_ver", 1,
                         [this]() -> QString { return tmp.path(); },
                         []() -> bool { return true; },
                         [](const QString &) -> bool { return true; },
                         [](const QString &) -> bool { return true; }));
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);
    }

    QString createTempFile(qint64 sizeBytes)
    {
        auto *f = new QTemporaryFile();
        f->setAutoRemove(false);
        f->open();
        if (sizeBytes > 0)
            f->resize(sizeBytes);
        f->close();
        QString path = f->fileName();
        delete f;
        return path;
    }
};

TEST_F(TaskManagerGradingOcrTest, GradeFileList_OcrAtThreshold_Light)
{
    // OCR threshold = 30; exactly 30 files → Light
    QStringList files;
    for (int i = 0; i < 30; ++i)
        files << QString("/tmp/ocr_%1.png").arg(i);
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Light);
}

TEST_F(TaskManagerGradingOcrTest, GradeFileList_OcrAboveThreshold_Medium)
{
    // 31 files → Medium
    QStringList files;
    for (int i = 0; i < 31; ++i)
        files << QString("/tmp/ocr_%1.png").arg(i);
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Medium);
}

TEST_F(TaskManagerGradingOcrTest, GradeFileList_OcrSizeOverThreshold_Medium)
{
    QString bigFile = createTempFile(201 * 1024 * 1024);
    QStringList files { bigFile };
    EXPECT_EQ(mgr->gradeFileListTask(files), IndexTask::Grade::Medium);
    QFile::remove(bigFile);
}

// ===========================================================================
// currentIndexStatus() – status string generation
// ===========================================================================

TEST_F(TaskManagerGradingTest, Status_NoTaskNoQueue_CleanState_Idle)
{
    // Set index state to Clean with matching version → "Idle"
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Idle"));
}

TEST_F(TaskManagerGradingTest, Status_LastTaskFailed_Failed)
{
    mgr->m_lastTaskFailed = true;
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Failed"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedMedium_Battery_WaitingPower)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { true, false, false };   // on battery

    TaskQueueItem item;
    item.type = IndexTask::Type::Update;
    item.grade = IndexTask::Grade::Medium;
    item.path = tmp.path();
    item.pathList = QStringList { tmp.path() };
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingPower"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedHeavy_Battery_WaitingPower)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { true, false, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::Create;
    item.grade = IndexTask::Grade::Heavy;
    item.path = tmp.path();
    item.pathList = QStringList { tmp.path() };
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingPower"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedLight_Battery_Running)
{
    // Light tasks can run on battery, so queued Light on battery → "Running"
    // (canRun returns true, so the status logic reports Running)
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { true, false, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::UpdateFileList;
    item.grade = IndexTask::Grade::Light;
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("Running"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedMedium_PowerSave_WaitingPowerSave)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { false, true, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::Update;
    item.grade = IndexTask::Grade::Medium;
    item.path = tmp.path();
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingPowerSave"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedLight_PowerSave_WaitingPowerSave)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { false, true, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::UpdateFileList;
    item.grade = IndexTask::Grade::Light;
    mgr->taskQueue.enqueue(item);

    // Light also pauses on power-save
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingPowerSave"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedMedium_NotIdle_WaitingIdle)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { false, false, false };   // not idle

    TaskQueueItem item;
    item.type = IndexTask::Type::Update;
    item.grade = IndexTask::Grade::Medium;
    item.path = tmp.path();
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingIdle"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedHeavy_NotIdle_WaitingIdle)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { false, false, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::Create;
    item.grade = IndexTask::Grade::Heavy;
    item.path = tmp.path();
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingIdle"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedLight_NotIdle_Running)
{
    // Light does not require idle → canRun returns true → "Running"
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { false, false, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::UpdateFileList;
    item.grade = IndexTask::Grade::Light;
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("Running"));
}

TEST_F(TaskManagerGradingTest, Status_QueuedForceBypass_AnyEnvironment_Running)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EnvDetector::instance().m_state = EnvState { true, true, false };

    TaskQueueItem item;
    item.type = IndexTask::Type::Update;
    item.grade = IndexTask::Grade::Heavy;
    item.forceBypass = true;
    item.path = tmp.path();
    mgr->taskQueue.enqueue(item);

    EXPECT_EQ(mgr->currentIndexStatus(), QString("Running"));
}

// ===========================================================================
// gradePriority() – priority ordering
// ===========================================================================

TEST_F(TaskManagerGradingTest, GradePriority_Manual_Highest)
{
    EXPECT_EQ(mgr->gradePriority(IndexTask::Grade::Manual), 4);
}

TEST_F(TaskManagerGradingTest, GradePriority_Heavy)
{
    EXPECT_EQ(mgr->gradePriority(IndexTask::Grade::Heavy), 3);
}

TEST_F(TaskManagerGradingTest, GradePriority_Medium)
{
    EXPECT_EQ(mgr->gradePriority(IndexTask::Grade::Medium), 2);
}

TEST_F(TaskManagerGradingTest, GradePriority_Light_LowestNonZero)
{
    EXPECT_EQ(mgr->gradePriority(IndexTask::Grade::Light), 1);
}

TEST_F(TaskManagerGradingTest, GradePriority_None_Zero)
{
    EXPECT_EQ(mgr->gradePriority(IndexTask::Grade::None), 0);
}

// ===========================================================================
// gradeToString() – string mapping
// ===========================================================================

TEST_F(TaskManagerGradingTest, GradeToString_AllGrades)
{
    EXPECT_EQ(mgr->gradeToString(IndexTask::Grade::None), QString("none"));
    EXPECT_EQ(mgr->gradeToString(IndexTask::Grade::Light), QString("light"));
    EXPECT_EQ(mgr->gradeToString(IndexTask::Grade::Medium), QString("medium"));
    EXPECT_EQ(mgr->gradeToString(IndexTask::Grade::Heavy), QString("heavy"));
    EXPECT_EQ(mgr->gradeToString(IndexTask::Grade::Manual), QString("manual"));
}

// ===========================================================================
// gradeUpdateTask() – Update task auto-grading
// ===========================================================================

TEST_F(TaskManagerGradingTest, GradeUpdate_CleanState_Light)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    EXPECT_EQ(mgr->gradeUpdateTask(), IndexTask::Grade::Light);
}

TEST_F(TaskManagerGradingTest, GradeUpdate_CreateInProgress_Heavy)
{
    // When create is in progress, Update should be graded Heavy to resume
    runtime->stateStore().setCreateInProgress(true);
    EXPECT_EQ(mgr->gradeUpdateTask(), IndexTask::Grade::Heavy);
}

TEST_F(TaskManagerGradingTest, GradeUpdate_DirtyState_Light)
{
    // Dirty state (not create in progress) → Light
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    EXPECT_EQ(mgr->gradeUpdateTask(), IndexTask::Grade::Light);
}

// ===========================================================================
// currentIndexStatus() – no task, dirty index, bad environment
//
// Two sub-scenarios depending on whether the index DB actually exists:
//
//   A) DB doesn't exist (fresh temp dir, no status file) → Heavy
//      Heavy requires !onBattery, !powerSaveMode, idle — all three checked.
//
//   B) DB exists, dirty, not CreateInProgress → Light (开机全盘扫描对比)
//      Light only requires !powerSaveMode — idle is NOT checked.
//      This is the user's actual scenario: startup with dirty index should
//      NOT show WaitingIdle because the update task is Light.
// ===========================================================================

// --- Sub-scenario A: DB doesn't exist → Heavy → always WaitingUpgrade ---

TEST_F(TaskManagerGradingTest, Status_NoTaskNoDB_GoodEnv_WaitingUpgrade)
{
    // Fresh temp dir: no status file → state is Unknown (not Clean), no DB → Heavy.
    // Heavy always shows WaitingUpgrade during the update interval.
    EnvDetector::instance().m_state = EnvState { false, false, true };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTaskNoDB_NotIdle_WaitingUpgrade)
{
    // No DB → Heavy. Not idle, but during interval → WaitingUpgrade.
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTaskNoDB_Battery_WaitingUpgrade)
{
    // No DB → Heavy. On battery, but during interval → WaitingUpgrade.
    EnvDetector::instance().m_state = EnvState { true, false, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTaskNoDB_PowerSave_WaitingUpgrade)
{
    // No DB → Heavy. Power save, but during interval → WaitingUpgrade.
    EnvDetector::instance().m_state = EnvState { false, true, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyNoDB_GoodEnv_WaitingUpgrade)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    EnvDetector::instance().m_state = EnvState { false, false, true };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

// --- Sub-scenario B: DB exists, dirty, not CreateInProgress → Light ---
//   Light shows "Idle" (completed) to the user — pending changes will be
//   silently updated when the timer fires. Only power-save blocks Light.

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyDB_NotIdle_Idle)
{
    // DB exists, dirty, Light → environment OK (not powerSave) → Idle.
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EnvDetector::instance().m_state = EnvState { false, false, false };   // not idle
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Idle"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyDB_OnBattery_Idle)
{
    // DB exists, dirty, Light → can run on battery → Idle.
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EnvDetector::instance().m_state = EnvState { true, false, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Idle"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyDB_PowerSave_WaitingPowerSave)
{
    // DB exists, dirty, Light → only blocked by powerSave.
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EnvDetector::instance().m_state = EnvState { false, true, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingPowerSave"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyDB_GoodEnv_Idle)
{
    // DB exists, dirty, Light → all good → Idle (will silently update).
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EnvDetector::instance().m_state = EnvState { false, false, true };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Idle"));
}

// --- Sub-scenario C: DB exists, dirty, CreateInProgress → Heavy ---
//   Heavy always shows WaitingUpgrade during the interval.

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyDB_CreateInProgress_WaitingUpgrade)
{
    // DB exists, dirty, CreateInProgress → Heavy → WaitingUpgrade.
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    runtime->stateStore().setCreateInProgress(true);
    EnvDetector::instance().m_state = EnvState { true, false, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_DirtyDB_CreateInProgress_NotIdle_WaitingUpgrade)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    runtime->stateStore().setCreateInProgress(true);
    EnvDetector::instance().m_state = EnvState { false, false, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_CleanIndex_Battery_Idle)
{
    // Clean index with matching version, no task, on battery → Idle (nothing to do)
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EnvDetector::instance().m_state = EnvState { true, false, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Idle"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_CleanIndex_PowerSave_Idle)
{
    // Clean index with matching version, no task, power-save → Idle (nothing to do)
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime());
    EnvDetector::instance().m_state = EnvState { false, true, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("Idle"));
}

// --- Sub-scenario D: Clean state but version mismatch → Heavy → WaitingUpgrade ---
//   When the index version is upgraded, the old status.json may still say
//   "clean" with an old version number.  Before silentStart fires, the status
//   should show "WaitingUpgrade", not "Idle".

TEST_F(TaskManagerGradingTest, Status_NoTask_CleanState_VersionMismatch_WaitingUpgrade)
{
    // Clean state but with an old version (0 instead of runtime version 1)
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime(), 0);
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_CleanState_VersionMismatch_Battery_WaitingUpgrade)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime(), 0);
    EnvDetector::instance().m_state = EnvState { true, false, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

TEST_F(TaskManagerGradingTest, Status_NoTask_CleanState_VersionMismatch_PowerSave_WaitingUpgrade)
{
    runtime->stateStore().setIndexState(IndexUtility::IndexState::Clean);
    runtime->stateStore().saveIndexStatus(QDateTime::currentDateTime(), 0);
    EnvDetector::instance().m_state = EnvState { false, true, false };
    EXPECT_EQ(mgr->currentIndexStatus(), QString("WaitingUpgrade"));
}

// ===========================================================================
// currentOrQueuedGrade() – grade from running task or queue head
// ===========================================================================

TEST_F(TaskManagerGradingTest, CurrentOrQueuedGrade_NoTaskNoQueue_Nullopt)
{
    EXPECT_FALSE(mgr->currentOrQueuedGrade().has_value());
}

TEST_F(TaskManagerGradingTest, CurrentOrQueuedGrade_QueuedTask_ReturnsQueueHeadGrade)
{
    TaskQueueItem item;
    item.type = IndexTask::Type::Update;
    item.grade = IndexTask::Grade::Heavy;
    item.path = tmp.path();
    mgr->taskQueue.enqueue(item);
    EXPECT_EQ(mgr->currentOrQueuedGrade().value(), IndexTask::Grade::Heavy);
}

TEST_F(TaskManagerGradingTest, CurrentOrQueuedGrade_QueuedLight_ReturnsLight)
{
    TaskQueueItem item;
    item.type = IndexTask::Type::UpdateFileList;
    item.grade = IndexTask::Grade::Light;
    mgr->taskQueue.enqueue(item);
    EXPECT_EQ(mgr->currentOrQueuedGrade().value(), IndexTask::Grade::Light);
}
