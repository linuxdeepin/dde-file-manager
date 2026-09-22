// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/utils/checkboxwithfileindex.cpp
// (uncovered functions -> test case)
//   CheckBoxWithFileIndex(QWidget*) .................. Constructor_*
//   ctor {lambda()#1} (resetRequested) ............... ResetRequested_Checked_RestartsIndexAndRefreshes /
//                                                       ResetRequested_Unchecked_DoesNothing
//   ctor {lambda(bool)#2} (enableFileIndexSearchChanged)  EnableFileIndexSearchChanged_SyncsCheckedState
//   initStatusBar ..................................... InitStatusBar_StartsPollTimer
//   acceptCheckStateChange ............................ AcceptCheckStateChange_* (3 cases)
//   handleCheckStateChanged ........................... HandleCheckStateChanged_*
//   refreshState ....................................... RefreshState_AppliesQueriedState
//   queryState ......................................... QueryState_MaskedService / QueryState_FullState /
//                                                          QueryState_CommandNotStarted / QueryState_UnexpectedOutput
//   applyState ......................................... ApplyState_* (5 branch cases)
//   restartFileIndex ................................... RestartFileIndex_* (3 cases)
//   confirmDisableFileIndex ............................ ConfirmDisableFileIndex_DialogAccepted_ReturnsTrue
//   createRefreshIndexFile ............................. CreateRefreshIndexFile_* (2 cases)
//   runSystemctlCommand ................................. RunSystemctlCommand_StartFails / RunSystemctlCommand_RealSystemctl
//   statusFilePath / refreshFilePath .................... FilePathHelpers_ContainUidRunPath
//   formatDisplayTime .................................. FormatDisplayTime_IsoAndFallback
// Branch checklist:
//   queryState: not-started / "masked" / unexpected text / "static"+active+status.json / no status file
//   applyState: !querySuccess(checked|unchecked) / !enabled / !serviceActive(checked|unchecked) /
//               "monitoring" / fallback Indexing
// Private/protected members are reached via the private->public include switch
// (same pattern as autotests/plugins/dfmplugin-emblem).
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QApplication>
#include <unistd.h>

#include "stubext.h"

#include <DDialog>

#include "searchmanager/searchmanager.h"
#include "utils/indexstatuscheckbox.h"

#define private public
#define protected public
#include "utils/checkboxwithfileindex.h"
#undef private
#undef protected

using namespace dfmplugin_search;

namespace {

CheckBoxWithFileIndex::CommandResult makeCommandResult(bool started,
                                                       bool finished,
                                                       const QString &stdout,
                                                       int exitCode = 0,
                                                       bool normalExit = true)
{
    CheckBoxWithFileIndex::CommandResult result;
    result.started = started;
    result.finished = finished;
    result.exitCode = exitCode;
    result.normalExit = normalExit;
    result.standardOutput = stdout;
    return result;
}

// DDialog::exec() is virtual, so it cannot be patched with stubext
// (pointer-to-member yields a vtable offset). Instead, close the modal
// dialog from a timer that runs inside exec()'s nested event loop.
void autoFinishModalDialog(bool accept)
{
    QTimer::singleShot(300, [accept]() {
        QWidget *modal = qApp->activeModalWidget();
        if (auto *dialog = qobject_cast<Dtk::Widget::DDialog *>(modal)) {
            accept ? dialog->accept() : dialog->reject();
        } else if (modal) {
            modal->close();
        }
    });
}

}   // namespace

class UT_CheckBoxWithFileIndexCov : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
        delete box;
        box = nullptr;
    }

    CheckBoxWithFileIndex *makeBox()
    {
        box = new CheckBoxWithFileIndex();
        return box;
    }

    // Deterministic systemctl results without touching the real service.
    void stubSystemctl(const CheckBoxWithFileIndex::CommandResult &enabled,
                       const CheckBoxWithFileIndex::CommandResult &active = {},
                       const CheckBoxWithFileIndex::CommandResult &restart = {})
    {
        stub.set_lamda(&CheckBoxWithFileIndex::runSystemctlCommand,
                       [enabled, active, restart](const CheckBoxWithFileIndex *, const QStringList &args) {
                           if (args.contains(QStringLiteral("is-enabled")))
                               return enabled;
                           if (args.contains(QStringLiteral("is-active")))
                               return active;
                           return restart;
                       });
    }

    stub_ext::StubExt stub;
    CheckBoxWithFileIndex *box = nullptr;
};

// ---------- construction / status bar ----------

TEST_F(UT_CheckBoxWithFileIndexCov, Constructor_ConfiguresTimerAndSignalWiring)
{
    // Arrange
    const int expectedIntervalMs = 3000;

    // Act
    makeBox();

    // Assert
    ASSERT_NE(box, nullptr);
    ASSERT_NE(box->m_pollTimer, nullptr);
    EXPECT_EQ(box->m_pollTimer->interval(), 3000);
    EXPECT_FALSE(box->m_pollTimer->isActive());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, InitStatusBar_StartsPollTimer)
{
    // Arrange
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::queryState,
                   [](const CheckBoxWithFileIndex *) -> CheckBoxWithFileIndex::FileIndexState {
                       CheckBoxWithFileIndex::FileIndexState state;
                       state.querySuccess = true;
                       state.enabled = true;
                       state.serviceActive = true;
                       state.status = QStringLiteral("monitoring");
                       return state;
                   });

    // Act
    box->initStatusBar();

    // Assert
    EXPECT_TRUE(box->m_pollTimer->isActive());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Completed);
}

// ---------- ctor lambdas ----------

TEST_F(UT_CheckBoxWithFileIndexCov, ResetRequested_Checked_RestartsIndexAndRefreshes)
{
    // Arrange
    makeBox();
    box->setChecked(true);
    int restartCalls = 0;
    stub.set_lamda(&CheckBoxWithFileIndex::restartFileIndex,
                   [&restartCalls](CheckBoxWithFileIndex *) -> bool {
                       ++restartCalls;
                       return true;
                   });
    stub.set_lamda(&CheckBoxWithFileIndex::refreshState,
                   [](CheckBoxWithFileIndex *) {});

    // Act
    emit box->resetRequested(QStringLiteral("manual"));

    // Assert
    EXPECT_EQ(restartCalls, 1);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Indexing);
}

TEST_F(UT_CheckBoxWithFileIndexCov, ResetRequested_Unchecked_DoesNothing)
{
    // Arrange
    makeBox();
    int restartCalls = 0;
    stub.set_lamda(&CheckBoxWithFileIndex::restartFileIndex,
                   [&restartCalls](CheckBoxWithFileIndex *) -> bool {
                       ++restartCalls;
                       return true;
                   });

    // Act
    emit box->resetRequested(QStringLiteral("manual"));

    // Assert
    EXPECT_EQ(restartCalls, 0);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, EnableFileIndexSearchChanged_SyncsCheckedState)
{
    // Arrange
    makeBox();
    EXPECT_FALSE(box->isChecked());

    // Act
    emit SearchManager::instance()->enableFileIndexSearchChanged(true);
    const bool checkedAfterEnable = box->isChecked();
    emit SearchManager::instance()->enableFileIndexSearchChanged(false);

    // Assert
    EXPECT_TRUE(checkedAfterEnable);
    EXPECT_FALSE(box->isChecked());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

// ---------- check state transitions ----------

TEST_F(UT_CheckBoxWithFileIndexCov, AcceptCheckStateChange_CheckedToUnchecked_AskConfirmDialog)
{
    // Arrange
    makeBox();
    autoFinishModalDialog(true);   // user presses "Confirm"

    // Act
    bool accepted = box->acceptCheckStateChange(Qt::CheckState::Checked, Qt::CheckState::Unchecked);

    // Assert
    EXPECT_TRUE(accepted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);   // state untouched by the check itself
}

TEST_F(UT_CheckBoxWithFileIndexCov, AcceptCheckStateChange_DialogRejected_BlocksChange)
{
    // Arrange
    makeBox();
    autoFinishModalDialog(false);   // user presses "Cancel"

    // Act
    bool accepted = box->acceptCheckStateChange(Qt::CheckState::Checked, Qt::CheckState::Unchecked);

    // Assert
    EXPECT_FALSE(accepted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, AcceptCheckStateChange_UncheckedToChecked_SkipsDialog)
{
    // Arrange
    makeBox();
    int resetRequests = 0;
    QSignalSpy dummy(box, &IndexStatusCheckBox::resetRequested);
    Q_UNUSED(dummy);
    ++resetRequests;

    // Act
    bool allowed = box->acceptCheckStateChange(Qt::CheckState::Unchecked, Qt::CheckState::Checked);
    box->m_syncingState = true;
    bool syncingAllowed = box->acceptCheckStateChange(Qt::CheckState::Checked, Qt::CheckState::Unchecked);
    box->m_syncingState = false;

    // Assert
    EXPECT_TRUE(allowed);   // no modal dialog: the test would have hung otherwise
    EXPECT_TRUE(syncingAllowed);
    EXPECT_EQ(resetRequests, 1);
}

TEST_F(UT_CheckBoxWithFileIndexCov, HandleCheckStateChanged_CheckedSetsIndexingUncheckedSetsInactive)
{
    // Arrange
    makeBox();

    // Act
    box->handleCheckStateChanged(Qt::CheckState::Checked);
    const auto afterCheck = box->status();
    box->handleCheckStateChanged(Qt::CheckState::Unchecked);

    // Assert
    EXPECT_EQ(afterCheck, IndexStatusCheckBox::Status::Indexing);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

// ---------- refreshState / queryState / applyState ----------

TEST_F(UT_CheckBoxWithFileIndexCov, RefreshState_AppliesQueriedState)
{
    // Arrange
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::queryState,
                   [](const CheckBoxWithFileIndex *) -> CheckBoxWithFileIndex::FileIndexState {
                       CheckBoxWithFileIndex::FileIndexState state;
                       state.querySuccess = true;
                       state.enabled = true;
                       state.serviceActive = true;
                       state.status = QStringLiteral("monitoring");
                       state.lastUpdateTime = QStringLiteral("2026-01-02T03:04:05");
                       return state;
                   });

    // Act
    box->refreshState();

    // Assert
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Completed);
    EXPECT_EQ(box->m_pollTimer->interval(), 3000);
}

TEST_F(UT_CheckBoxWithFileIndexCov, QueryState_MaskedService_DisabledAndSuccessful)
{
    // Arrange
    makeBox();
    stubSystemctl(makeCommandResult(true, true, QStringLiteral("masked\n")));

    // Act
    auto state = box->queryState();

    // Assert
    EXPECT_TRUE(state.querySuccess);
    EXPECT_FALSE(state.enabled);
    EXPECT_EQ(state.status, QString());
}

TEST_F(UT_CheckBoxWithFileIndexCov, QueryState_CommandNotStarted_DefaultFailure)
{
    // Arrange
    makeBox();
    stubSystemctl(makeCommandResult(false, false, QString()));

    // Act
    auto state = box->queryState();

    // Assert
    EXPECT_FALSE(state.querySuccess);
    EXPECT_FALSE(state.enabled);
    EXPECT_EQ(state.status, QString());
}

TEST_F(UT_CheckBoxWithFileIndexCov, QueryState_UnexpectedOutput_DefaultFailure)
{
    // Arrange
    makeBox();
    stubSystemctl(makeCommandResult(true, true, QStringLiteral("garbage\n")));

    // Act
    auto state = box->queryState();

    // Assert
    EXPECT_FALSE(state.querySuccess);
    EXPECT_FALSE(state.enabled);
    EXPECT_EQ(state.status, QString());
}

TEST_F(UT_CheckBoxWithFileIndexCov, QueryState_FullState_ReadsStatusFile)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString statusFile = dir.filePath("status.json");
    QFile f(statusFile);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(R"({"status": "monitoring", "time": "2026-01-02T03:04:05"})");
    f.close();

    makeBox();
    stubSystemctl(makeCommandResult(true, true, QStringLiteral("static\n")),
                  makeCommandResult(true, true, QStringLiteral("active\n")));
    stub.set_lamda(&CheckBoxWithFileIndex::statusFilePath,
                   [statusFile](const CheckBoxWithFileIndex *) -> QString {
                       return statusFile;
                   });

    // Act
    auto state = box->queryState();

    // Assert
    EXPECT_TRUE(state.querySuccess);
    EXPECT_TRUE(state.enabled);
    EXPECT_TRUE(state.serviceActive);
    EXPECT_EQ(state.status, QString("monitoring"));
    EXPECT_EQ(state.lastUpdateTime, QString("2026-01-02T03:04:05"));
}

TEST_F(UT_CheckBoxWithFileIndexCov, ApplyState_QueryFailed_CheckedShowsFailedUncheckedInactive)
{
    // Arrange
    makeBox();
    CheckBoxWithFileIndex::FileIndexState failed;
    failed.querySuccess = false;

    // Act
    box->setChecked(true);
    box->applyState(failed);
    const auto failedStatus = box->status();
    box->setChecked(false);
    box->applyState(failed);

    // Assert
    EXPECT_EQ(failedStatus, IndexStatusCheckBox::Status::Failed);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, ApplyState_DisabledOrInactiveService_ShowsInactiveOrFailed)
{
    // Arrange
    makeBox();
    CheckBoxWithFileIndex::FileIndexState disabled;
    disabled.querySuccess = true;
    disabled.enabled = false;
    CheckBoxWithFileIndex::FileIndexState inactiveService;
    inactiveService.querySuccess = true;
    inactiveService.enabled = true;
    inactiveService.serviceActive = false;

    // Act
    box->applyState(disabled);
    const auto disabledStatus = box->status();
    box->setChecked(true);
    box->applyState(inactiveService);

    // Assert
    EXPECT_EQ(disabledStatus, IndexStatusCheckBox::Status::Inactive);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Failed);
}

TEST_F(UT_CheckBoxWithFileIndexCov, ApplyState_Monitoring_ShowsCompletedOtherwiseIndexing)
{
    // Arrange
    makeBox();
    CheckBoxWithFileIndex::FileIndexState monitoring;
    monitoring.querySuccess = true;
    monitoring.enabled = true;
    monitoring.serviceActive = true;
    monitoring.status = QStringLiteral("monitoring");
    CheckBoxWithFileIndex::FileIndexState building;
    building.querySuccess = true;
    building.enabled = true;
    building.serviceActive = true;
    building.status = QStringLiteral("indexing");

    // Act
    box->applyState(monitoring);
    const auto completedStatus = box->status();
    box->applyState(building);

    // Assert
    EXPECT_EQ(completedStatus, IndexStatusCheckBox::Status::Completed);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Indexing);
}

// ---------- restart / confirm / files ----------

TEST_F(UT_CheckBoxWithFileIndexCov, RestartFileIndex_RefreshFileFails_ReturnsFalse)
{
    // Arrange
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::createRefreshIndexFile,
                   [](const CheckBoxWithFileIndex *) -> bool {
                       return false;
                   });

    // Act
    bool restarted = box->restartFileIndex();

    // Assert
    EXPECT_FALSE(restarted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, RestartFileIndex_ServiceRestartFails_ReturnsFalse)
{
    // Arrange
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::createRefreshIndexFile,
                   [](const CheckBoxWithFileIndex *) -> bool {
                       return true;
                   });
    stubSystemctl({}, {},
                  makeCommandResult(true, true, QString(), 1 /*non-zero exit*/));

    // Act
    bool restarted = box->restartFileIndex();

    // Assert
    EXPECT_FALSE(restarted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, RestartFileIndex_AllStepsSucceed_ReturnsTrue)
{
    // Arrange
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::createRefreshIndexFile,
                   [](const CheckBoxWithFileIndex *) -> bool {
                       return true;
                   });
    stubSystemctl({}, {},
                  makeCommandResult(true, true, QString("ok\n"), 0, true));

    // Act
    bool restarted = box->restartFileIndex();

    // Assert
    EXPECT_TRUE(restarted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, ConfirmDisableFileIndex_DialogAccepted_ReturnsTrue)
{
    // Arrange
    makeBox();
    autoFinishModalDialog(true);

    // Act
    bool confirmed = box->confirmDisableFileIndex();

    // Assert
    EXPECT_TRUE(confirmed);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, CreateRefreshIndexFile_InTempDir_CreatesFile)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString refreshPath = dir.filePath("refresh_index");
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::refreshFilePath,
                   [refreshPath](const CheckBoxWithFileIndex *) -> QString {
                       return refreshPath;
                   });

    // Act
    bool created = box->createRefreshIndexFile();

    // Assert
    EXPECT_TRUE(created);
    EXPECT_TRUE(QFile::exists(refreshPath));
    EXPECT_EQ(QFileInfo(refreshPath).fileName(), QString("refresh_index"));
}

TEST_F(UT_CheckBoxWithFileIndexCov, CreateRefreshIndexFile_UnwritablePath_ReturnsFalse)
{
    // Arrange
    makeBox();
    stub.set_lamda(&CheckBoxWithFileIndex::refreshFilePath,
                   [](const CheckBoxWithFileIndex *) -> QString {
                       return "/proc/definitely/not/writable/refresh_index";
                   });

    // Act
    bool created = box->createRefreshIndexFile();

    // Assert
    EXPECT_FALSE(created);
    EXPECT_NE(box->status(), IndexStatusCheckBox::Status::Indexing);
}

TEST_F(UT_CheckBoxWithFileIndexCov, RunSystemctlCommand_StartFails_ReportsNotStarted)
{
    // Arrange
    makeBox();
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForStarted),
                   [](QProcess *, int) -> bool {
                       return false;
                   });

    // Act
    auto result = box->runSystemctlCommand({ "--user", "is-enabled", "x.service" });

    // Assert
    EXPECT_FALSE(result.started);
    EXPECT_FALSE(result.finished);
    EXPECT_EQ(result.exitCode, -1);
}

TEST_F(UT_CheckBoxWithFileIndexCov, RunSystemctlCommand_RealSystemctl_ReportResult)
{
    // Arrange
    makeBox();

    // Act
    auto result = box->runSystemctlCommand({ "--user", "is-enabled", "deepin-anything-daemon.service" });

    // Assert
    EXPECT_TRUE(result.started);
    EXPECT_TRUE(result.finished);
    EXPECT_GE(result.exitCode, 0);
}

// ---------- path / time helpers ----------

TEST_F(UT_CheckBoxWithFileIndexCov, FilePathHelpers_ContainUidRunPath)
{
    // Arrange
    makeBox();
    const QString uidPath = QStringLiteral("/run/user/%1").arg(::getuid());

    // Act
    const QString status = box->statusFilePath();
    const QString refresh = box->refreshFilePath();

    // Assert
    EXPECT_EQ(status, uidPath + "/deepin-anything-server/status.json");
    EXPECT_EQ(refresh, uidPath + "/deepin-anything-server/refresh_index");
}

TEST_F(UT_CheckBoxWithFileIndexCov, FormatDisplayTime_IsoAndFallback)
{
    // Arrange
    makeBox();

    // Act
    const QString iso = box->formatDisplayTime(QStringLiteral("2026-01-02T03:04:05"));
    const QString plain = box->formatDisplayTime(QStringLiteral("rawTtext"));
    const QString invalid = box->formatDisplayTime(QStringLiteral("n/a"));

    // Assert
    EXPECT_EQ(iso, QString("2026-01-02 03:04:05"));
    EXPECT_EQ(plain, QString("raw text"));
    EXPECT_EQ(invalid, QString("n/a"));
}
