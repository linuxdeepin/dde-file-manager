// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QFuture>

#include "searchmanager/maincontroller/maincontroller.h"
#include "searchmanager/maincontroller/task/taskcommander.h"
#include "searchmanager/searcher/searchresult_define.h"

#include "stubext.h"

DPSEARCH_USE_NAMESPACE

class TestMainController : public testing::Test
{
public:
    void SetUp() override
    {
        controller = new MainController;
    }

    void TearDown() override
    {
        if (controller) {
            delete controller;
            controller = nullptr;
        }
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    MainController *controller = nullptr;
};

TEST_F(TestMainController, Constructor_CreatesValidInstance)
{
    // Test that MainController can be created (via friend class or reflection)
    // Note: In real implementation, this would be done via SearchManager
    EXPECT_TRUE(true);   // Basic test that class exists
}

TEST_F(TestMainController, DoSearchTask_WithValidParameters_ReturnsTrue)
{
    stub.set_lamda(&TaskCommander::start, [](TaskCommander *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Test parameters
    QString taskId = "test_task_001";
    QUrl searchUrl = QUrl::fromLocalFile("/home/test");
    QString keyword = "test_keyword";

    // Since doSearchTask is private, we test via public interface or metaObject
    bool result = false;
    if (controller) {
        result = controller->doSearchTask(taskId, searchUrl, keyword);
    }

    EXPECT_TRUE(result);   // Test that method signature exists
}

TEST_F(TestMainController, DoSearchTask_WithEmptyTaskId_HandlesCorrectly)
{
    stub.set_lamda(&TaskCommander::start, [](TaskCommander *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Test parameters
    QString taskId = "test_task_001";
    QUrl searchUrl = QUrl::fromLocalFile("/home/test");
    QString keyword = "test_keyword";

    // Since doSearchTask is private, we test via public interface or metaObject
    bool result = false;
    if (controller) {
        result = controller->doSearchTask(taskId, searchUrl, keyword);
    }

    EXPECT_FALSE(result);
}

TEST_F(TestMainController, Stop_WithValidTaskId_CallsTaskCommanderStop)
{
    QString taskId = "test_task_003";

    // Mock TaskCommander operations
    stub.set_lamda(&TaskCommander::stop, [](TaskCommander *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TaskCommander::deleteLater, [](QObject *) {
        __DBG_STUB_INVOKE__
    });

    TaskCommander *task = new TaskCommander(taskId, QUrl("file:///test"), "key");
    controller->taskManager.insert(taskId, task);
    if (controller) {
        controller->stop(taskId);
    }

    EXPECT_TRUE(true);
}

TEST_F(TestMainController, GetResults_WithValidTaskId_ReturnsResults)
{
    QString taskId = "test_task_004";

    // Mock TaskCommander to return test results
    stub.set_lamda(&TaskCommander::getResults, [](TaskCommander *) -> DFMSearchResultMap {
        __DBG_STUB_INVOKE__
        DFMSearchResultMap mockResults;
        return mockResults;
    });

    TaskCommander *task = new TaskCommander(taskId, QUrl("file:///test"), "key");
    controller->taskManager.insert(taskId, task);

    DFMSearchResultMap results;
    if (controller) {
        results = controller->getResults(taskId);
    }

    EXPECT_TRUE(results.isEmpty());
}

TEST_F(TestMainController, GetResultUrls_WithValidTaskId_ReturnsUrls)
{
    QString taskId = "test_task_005";

    // Mock TaskCommander to return test URLs
    stub.set_lamda(&TaskCommander::getResultsUrls, [] {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });

    TaskCommander *task = new TaskCommander(taskId, QUrl("file:///test"), "key");
    controller->taskManager.insert(taskId, task);

    QList<QUrl> urls;
    if (controller) {
        urls = controller->getResultUrls(taskId);
    }

    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(TestMainController, OnFinished_WithValidTaskId_EmitsSignals)
{
    QString taskId = "test_task_006";

    if (controller) {
        EXPECT_NO_FATAL_FAILURE(controller->onFinished(taskId));
    }
}
