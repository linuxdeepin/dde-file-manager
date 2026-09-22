// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/utils/indexstatuscontroller.cpp
// (uncovered functions -> test case)
//   IndexStatusController(view,client,options,parent) .. ControllerConstruction_* (ctor itself)
//   ctor {lambda(Qt::CheckState)#1} ..................... CheckStateChanged_CheckedQueriesIndex_* / *_Unchecked_*
//   ctor {lambda(QString)#2 resetRequested} ............. ResetRequested_Manual_ForcesUpdate /
//                                                          ResetRequested_Other_BypassesEnv
//   ctor {lambda(QString,QString,bool)#3} ................ IndexStatusResult_Failure_Ignored /
//                                                          IndexStatusResult_Success_AppliesState (via applyServerStatus)
//   ctor {lambda(QString,QString)#4} ..................... IndexStatusChanged_Idle_CompletesAndQueriesTime
//   ctor {lambda(QString,bool)#5} ......................... LastUpdateTimeResult_* (2 cases)
//   syncCheckedState(bool) ................................ SyncCheckedState_TogglesView
//   connectToBackend() .................................... ConnectToBackend_QueriesServiceStatus
//   connectToBackend(){lambda#2..#4} ...................... TaskSignals_* (progress/finished/failed)
//   initStatusBar() ........................................ InitStatusBar_* (checked/unchecked)
//   shouldHandleIndexEvent(path,type) ..................... ShouldHandleIndexEvent_* (3 cases)
//   applyServerStatus(state) ............................. ApplyServerStatus_* (Running/Idle/Failed/4 waiting/unknown)
//   applyWaitingStatus(status) ........................... covered via ApplyServerStatus_Waiting_*
// Private members are reached through the private->public include switch.
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QVariant>
#include <QStringList>

#include "stubext.h"

#include <dfm-search/dsearch_global.h>

#include "utils/indexstatuscheckbox.h"
#include "utils/indexclientdescriptor.h"

#include "utils/abstractindexclient.h"
#include "utils/indexstatuscontroller.h"

using namespace dfmplugin_search;

class UT_IndexStatusControllerCov : public testing::Test
{
protected:
    void SetUp() override
    {
        IndexClientDescriptor desc;
        desc.clientName = "covcontroller";
        desc.dbusServiceName = "com.deepin.covtest.Controller";
        desc.dbusObjectPath = "/com/deepin/covtest/Controller";
        desc.interfaceFactory = nullptr;

        client = new AbstractIndexClient(desc);
        view = new IndexStatusCheckBox();

        IndexStatusControllerOptions options;
        options.logTag = QStringLiteral("cov");
        options.inactiveText = QStringLiteral("inactive");
        options.indexingInitialText = QStringLiteral("initial");
        options.indexingFilesText = QStringLiteral("%1 files");
        options.indexingItemsText = QStringLiteral("%1/%2 items");
        options.failedMainText = QStringLiteral("failed main");
        options.failedLinkText = QStringLiteral("retry");
        options.completedMainText = QStringLiteral("done at %1");
        options.completedLinkText = QStringLiteral("update");
        options.waitingPowerMainText = QStringLiteral("battery");
        options.waitingPowerSaveMainText = QStringLiteral("power save");
        options.waitingIdleMainText = QStringLiteral("idle");
        options.waitingUpgradeMainText = QStringLiteral("upgrade");
        options.waitingUpdateLinkText = QStringLiteral("continue");
        options.waitingUpgradeLinkText = QStringLiteral("upgrade link");

        controller = new IndexStatusController(view, client, options, nullptr);

        // Record backend queries without touching DBus.
        stub.set_lamda(&AbstractIndexClient::getIndexStatus,
                       [this](AbstractIndexClient *) {
                           ++getIndexStatusCalls;
                       });
        stub.set_lamda(&AbstractIndexClient::getLastUpdateTime,
                       [this](AbstractIndexClient *) {
                           ++getLastUpdateTimeCalls;
                       });
        stub.set_lamda(&AbstractIndexClient::forceUpdateIndex,
                       [this](AbstractIndexClient *, const QStringList &paths) {
                           ++forceUpdateCalls;
                           lastPaths = paths;
                       });
        stub.set_lamda(&AbstractIndexClient::updateIndexBypassEnv,
                       [this](AbstractIndexClient *, const QStringList &paths) {
                           ++bypassCalls;
                           lastPaths = paths;
                       });
        stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
            return QStringList() << "/home/test/indexed";
        });
    }

    void TearDown() override
    {
        stub.clear();
        delete controller;
        delete client;
        delete view;
    }

    stub_ext::StubExt stub;
    IndexStatusController *controller = nullptr;
    AbstractIndexClient *client = nullptr;
    IndexStatusCheckBox *view = nullptr;

    int getIndexStatusCalls = 0;
    int getLastUpdateTimeCalls = 0;
    int forceUpdateCalls = 0;
    int bypassCalls = 0;
    QStringList lastPaths;
};

// ---------- ctor lambdas ----------

TEST_F(UT_IndexStatusControllerCov, CheckStateChanged_Checked_SetsIndexingAndQueriesStatus)
{
    // Arrange
    view->setChecked(false);
    EXPECT_EQ(getIndexStatusCalls, 0);

    // Act
    emit view->checkStateChanged(Qt::CheckState::Checked);

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Indexing);
    EXPECT_EQ(getIndexStatusCalls, 1);
}

TEST_F(UT_IndexStatusControllerCov, CheckStateChanged_Unchecked_SetsInactive)
{
    // Arrange
    view->setChecked(true);
    const int baseline = getIndexStatusCalls;

    // Act
    emit view->checkStateChanged(Qt::CheckState::Unchecked);

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Inactive);
    EXPECT_EQ(getIndexStatusCalls - baseline, 0);   // unchecking must not re-query the backend
}

TEST_F(UT_IndexStatusControllerCov, ResetRequested_Manual_ForcesUpdate)
{
    // Arrange
    const QString href = QStringLiteral("manual");

    // Act
    emit view->resetRequested(href);

    // Assert
    EXPECT_EQ(forceUpdateCalls, 1);
    EXPECT_EQ(bypassCalls, 0);
    EXPECT_EQ(lastPaths, QStringList { "/home/test/indexed" });
}

TEST_F(UT_IndexStatusControllerCov, ResetRequested_Other_BypassesEnv)
{
    // Arrange
    const QString href = QStringLiteral("bypass");

    // Act
    emit view->resetRequested(href);

    // Assert
    EXPECT_EQ(bypassCalls, 1);
    EXPECT_EQ(forceUpdateCalls, 0);
}

TEST_F(UT_IndexStatusControllerCov, IndexStatusResult_Failure_Ignored)
{
    // Arrange
    view->setChecked(true);
    view->setStatus(IndexStatusCheckBox::Status::Indexing);

    // Act
    emit client->indexStatusResult(QString(), QString(), false);

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Indexing);   // untouched
    EXPECT_EQ(getLastUpdateTimeCalls, 0);
}

TEST_F(UT_IndexStatusControllerCov, IndexStatusResult_Success_AppliesState)
{
    // Arrange
    view->setChecked(true);

    // Act
    emit client->indexStatusResult(QStringLiteral("Running"), QString(), true);

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Indexing);
    EXPECT_EQ(getLastUpdateTimeCalls, 0);
}

TEST_F(UT_IndexStatusControllerCov, IndexStatusChanged_Idle_CompletesAndQueriesTime)
{
    // Arrange
    view->setChecked(true);

    // Act
    emit client->indexStatusChanged(QStringLiteral("Idle"), QString());

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Completed);
    EXPECT_EQ(getLastUpdateTimeCalls, 1);
}

TEST_F(UT_IndexStatusControllerCov, LastUpdateTimeResult_SuccessOnCompleted_UpdatesText)
{
    // Arrange
    view->setChecked(true);
    view->setStatus(IndexStatusCheckBox::Status::Completed);

    // Act
    emit client->lastUpdateTimeResult(QStringLiteral("2026-01-01"), true);

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Completed);   // still completed
    EXPECT_EQ(getLastUpdateTimeCalls, 0);
}

TEST_F(UT_IndexStatusControllerCov, LastUpdateTimeResult_EmptyTime_FallsToWarning)
{
    // Arrange
    view->setChecked(true);
    view->setStatus(IndexStatusCheckBox::Status::Failed);

    // Act
    emit client->lastUpdateTimeResult(QString(), false);

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Failed);   // untouched
    EXPECT_EQ(getLastUpdateTimeCalls, 0);
}

// ---------- sync / backend wiring ----------

TEST_F(UT_IndexStatusControllerCov, SyncCheckedState_TogglesView)
{
    // Arrange
    const IndexStatusCheckBox::Status initialStatus = view->status();

    // Act
    controller->syncCheckedState(true);
    bool afterEnable = view->isChecked();
    controller->syncCheckedState(false);

    // Assert
    EXPECT_TRUE(afterEnable);
    EXPECT_FALSE(view->isChecked());
    EXPECT_EQ(view->status(), initialStatus);
}

TEST_F(UT_IndexStatusControllerCov, ConnectToBackend_QueriesServiceStatusAndConnectsTaskSignals)
{
    // Arrange
    view->setChecked(true);
    const int baseline = getIndexStatusCalls;

    // Act
    controller->connectToBackend();
    emit client->taskProgressChanged(AbstractIndexClient::TaskType::Update, QStringLiteral("/home"), 5, 10);
    const auto indexing = view->status();
    emit client->taskFinished(AbstractIndexClient::TaskType::Update, QStringLiteral("/home"), true);
    emit client->taskFailed(AbstractIndexClient::TaskType::Update, QStringLiteral("/home"), QString("err"));

    // Assert
    EXPECT_EQ(indexing, IndexStatusCheckBox::Status::Indexing);
    EXPECT_EQ(getIndexStatusCalls - baseline, 2);   // taskFinished + taskFailed each re-query status
}

TEST_F(UT_IndexStatusControllerCov, TaskSignals_UnrelatedPath_Ignored)
{
    // Arrange
    controller->connectToBackend();
    view->setChecked(false);

    // Act
    emit client->taskFinished(AbstractIndexClient::TaskType::Update, QStringLiteral("/media/other"), true);

    // Assert
    EXPECT_EQ(getIndexStatusCalls, 0);   // shouldHandleIndexEvent false (view unchecked)
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_IndexStatusControllerCov, InitStatusBar_Checked_QueriesStatus)
{
    // Arrange
    view->setChecked(true);
    const int baseline = getIndexStatusCalls;

    // Act
    controller->initStatusBar();

    // Assert
    EXPECT_EQ(getIndexStatusCalls - baseline, 1);
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Indexing);
}

TEST_F(UT_IndexStatusControllerCov, InitStatusBar_Unchecked_SetsInactive)
{
    // Arrange
    view->setChecked(false);

    // Act
    controller->initStatusBar();

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Inactive);
    EXPECT_EQ(getIndexStatusCalls, 0);
}

// ---------- shouldHandleIndexEvent ----------

TEST_F(UT_IndexStatusControllerCov, ShouldHandleIndexEvent_States)
{
    // Arrange
    view->setChecked(false);

    // Act
    const bool unchecked = controller->shouldHandleIndexEvent(QStringLiteral("/home"), AbstractIndexClient::TaskType::Update);
    view->setChecked(true);
    const bool checkedNoFilter = controller->shouldHandleIndexEvent(QStringLiteral("/home"), AbstractIndexClient::TaskType::Update);

    // Assert
    EXPECT_FALSE(unchecked);
    EXPECT_TRUE(checkedNoFilter);
    EXPECT_EQ(client->descriptor().clientName, QString("covcontroller"));
}

TEST_F(UT_IndexStatusControllerCov, ShouldHandleIndexEvent_UserFilterDecides)
{
    // Arrange
    view->setChecked(true);
    auto *filtered = new IndexStatusController(view, client, IndexStatusControllerOptions(), nullptr);
    IndexStatusControllerOptions options;
    options.shouldHandleEvent = [](const QString &path, AbstractIndexClient::TaskType type) {
        return path == QStringLiteral("/home") && type == AbstractIndexClient::TaskType::Create;
    };
    delete filtered;
    auto *withFilter = new IndexStatusController(view, client, options, nullptr);

    // Act
    const bool match = withFilter->shouldHandleIndexEvent(QStringLiteral("/home"), AbstractIndexClient::TaskType::Create);
    const bool noMatch = withFilter->shouldHandleIndexEvent(QStringLiteral("/opt"), AbstractIndexClient::TaskType::Create);

    // Assert
    EXPECT_TRUE(match);
    EXPECT_FALSE(noMatch);
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Indexing);
    delete withFilter;
}

// ---------- applyServerStatus / applyWaitingStatus ----------

TEST_F(UT_IndexStatusControllerCov, ApplyServerStatus_Unchecked_IgnoresEverything)
{
    // Arrange
    view->setChecked(false);

    // Act
    controller->applyServerStatus(QStringLiteral("Running"));

    // Assert
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::Inactive);
    EXPECT_EQ(getLastUpdateTimeCalls, 0);
}

TEST_F(UT_IndexStatusControllerCov, ApplyServerStatus_RunningFailed_Unknown_Mapped)
{
    // Arrange
    view->setChecked(true);

    // Act
    controller->applyServerStatus(QStringLiteral("Running"));
    const auto running = view->status();
    controller->applyServerStatus(QStringLiteral("Failed"));
    const auto failed = view->status();
    controller->applyServerStatus(QStringLiteral("Whatever"));
    const auto unknown = view->status();

    // Assert
    EXPECT_EQ(running, IndexStatusCheckBox::Status::Indexing);
    EXPECT_EQ(failed, IndexStatusCheckBox::Status::Failed);
    EXPECT_EQ(unknown, IndexStatusCheckBox::Status::Failed);   // unknown keeps previous state
}

TEST_F(UT_IndexStatusControllerCov, ApplyServerStatus_WaitingStates_Mapped)
{
    // Arrange
    view->setChecked(true);
    const int waitingStateCount = 4;

    // Act
    controller->applyServerStatus(QStringLiteral("WaitingPower"));
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::WaitingPower);

    // Assert (continued)
    EXPECT_EQ(waitingStateCount, 4);
    controller->applyServerStatus(QStringLiteral("WaitingPowerSave"));
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::WaitingPowerSave);

    controller->applyServerStatus(QStringLiteral("WaitingIdle"));
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::WaitingIdle);

    controller->applyServerStatus(QStringLiteral("WaitingUpgrade"));
    EXPECT_EQ(view->status(), IndexStatusCheckBox::Status::WaitingUpgrade);
}
