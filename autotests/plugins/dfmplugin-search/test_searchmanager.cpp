// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>

// Include search manager from the plugin
#include "searchmanager/searchmanager.h"
#include "searchmanager/maincontroller/maincontroller.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/dpf.h>

using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SearchManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        manager = SearchManager::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    SearchManager *manager = nullptr;
};

TEST_F(UT_SearchManager, Instance_ReturnsSameInstance)
{
    auto manager1 = SearchManager::instance();
    auto manager2 = SearchManager::instance();

    EXPECT_NE(manager1, nullptr);
    EXPECT_EQ(manager1, manager2);
}

TEST_F(UT_SearchManager, Search_WithValidParameters_ReturnsTrue)
{
    quint64 winId = 12345;
    QString taskId = "test_task_001";
    QUrl url = QUrl::fromLocalFile("/home/test");
    QString keyword = "test_keyword";

    bool doSearchTaskCalled = false;

    // Mock MainController::doSearchTask
    stub.set_lamda(ADDR(MainController, doSearchTask),
                   [&doSearchTaskCalled, taskId, url, keyword](MainController *, const QString &id, const QUrl &searchUrl, const QString &word) -> bool {
                       __DBG_STUB_INVOKE__
                       doSearchTaskCalled = true;
                       EXPECT_EQ(id, taskId);
                       EXPECT_EQ(searchUrl, url);
                       EXPECT_EQ(word, keyword);
                       return true;
                   });

    bool result = manager->search(winId, taskId, url, keyword);

    EXPECT_TRUE(result);
    EXPECT_TRUE(doSearchTaskCalled);
}

TEST_F(UT_SearchManager, Search_WithNullMainController_ReturnsFalse)
{
    quint64 winId = 12345;
    QString taskId = "test_task_002";
    QUrl url = QUrl::fromLocalFile("/home/test");
    QString keyword = "test_keyword";

    // Make mainController return null by stubbing the initialization
    stub.set_lamda(ADDR(MainController, doSearchTask),
                   [](MainController *, const QString &, const QUrl &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = manager->search(winId, taskId, url, keyword);

    EXPECT_FALSE(result);
}

TEST_F(UT_SearchManager, MatchedResults_WithValidTaskId_ReturnsResults)
{
    QString taskId = "test_task_003";
    DFMSearchResultMap expectedResults;
    // Add some test data to expectedResults

    // Mock MainController::getResults
    stub.set_lamda(ADDR(MainController, getResults),
                   [expectedResults](MainController *, const QString &id) -> DFMSearchResultMap {
                       __DBG_STUB_INVOKE__
                       return expectedResults;
                   });

    auto results = manager->matchedResults(taskId);

    EXPECT_TRUE(results.isEmpty());
}

TEST_F(UT_SearchManager, MatchedResultUrls_WithValidTaskId_ReturnsUrls)
{
    QString taskId = "test_task_004";
    QList<QUrl> expectedUrls = {
        QUrl::fromLocalFile("/home/test1.txt"),
        QUrl::fromLocalFile("/home/test2.txt")
    };

    // Mock MainController::getResultUrls
    stub.set_lamda(ADDR(MainController, getResultUrls),
                   [expectedUrls](MainController *, const QString &id) -> QList<QUrl> {
                       __DBG_STUB_INVOKE__
                       return expectedUrls;
                   });

    auto urls = manager->matchedResultUrls(taskId);

    EXPECT_EQ(urls, expectedUrls);
}

TEST_F(UT_SearchManager, Stop_WithTaskId_CallsMainControllerStop)
{
    QString taskId = "test_task_005";
    bool stopCalled = false;

    // Mock MainController::stop
    stub.set_lamda(ADDR(MainController, stop),
                   [&stopCalled, taskId](MainController *, const QString &id) {
                       __DBG_STUB_INVOKE__
                       stopCalled = true;
                       EXPECT_EQ(id, taskId);
                   });

    QSignalSpy spy(manager, &SearchManager::searchStoped);

    manager->stop(taskId);

    EXPECT_TRUE(stopCalled);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), taskId);
}

TEST_F(UT_SearchManager, Stop_WithWindowId_StopsAssociatedTask)
{
    quint64 winId = 12345;
    QString taskId = "test_task_006";
    QUrl url = QUrl::fromLocalFile("/home/test");
    QString keyword = "test_keyword";

    bool stopCalled = false;

    // First start a search to establish the mapping
    stub.set_lamda(ADDR(MainController, doSearchTask),
                   [](MainController *, const QString &, const QUrl &, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    manager->search(winId, taskId, url, keyword);

    // Mock MainController::stop
    stub.set_lamda(ADDR(MainController, stop),
                   [&stopCalled, taskId](MainController *, const QString &id) {
                       __DBG_STUB_INVOKE__
                       stopCalled = true;
                       EXPECT_EQ(id, taskId);
                   });

    QSignalSpy spy(manager, &SearchManager::searchStoped);

    manager->stop(winId);

    EXPECT_TRUE(stopCalled);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_SearchManager, OnDConfigValueChanged_WithSearchConfig_EmitsSignal)
{
    QString config = DConfig::kSearchCfgPath;
    QString key = DConfig::kEnableFullTextSearch;

    // Mock DConfigManager::value
    stub.set_lamda(&DConfigManager::value, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock dpfSignalDispatcher->publish
    typedef bool (EventDispatcherManager::*PublishFunc)(const QString &, const QString &, QString, QVariantMap &);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    bool publishCalled = false;

    stub.set_lamda(publish, [&publishCalled](EventDispatcherManager *, const QString &space, const QString &topic, QString data, QVariantMap map) -> bool {
        __DBG_STUB_INVOKE__
        publishCalled = true;
        EXPECT_EQ(space, QString("dfmplugin_search"));
        EXPECT_EQ(topic, QString("signal_ReportLog_Commit"));
        EXPECT_EQ(data, QString("Search"));
        return true;
    });

    QSignalSpy spy(manager, &SearchManager::enableFullTextSearchChanged);

    manager->onDConfigValueChanged(config, key);

    EXPECT_TRUE(publishCalled);
}

TEST_F(UT_SearchManager, OnDConfigValueChanged_WithDifferentConfig_DoesNotEmitSignal)
{
    stub.set_lamda(&DConfigManager::value, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QString config = "other.config.path";
    QString key = DConfig::kEnableFullTextSearch;

    QSignalSpy spy(manager, &SearchManager::enableFullTextSearchChanged);

    manager->onDConfigValueChanged(config, key);

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_SearchManager, OnDConfigValueChanged_WithDifferentKey_DoesNotEmitSignal)
{
    stub.set_lamda(&DConfigManager::value, [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    QString config = DConfig::kSearchCfgPath;
    QString key = "other.key";

    QSignalSpy spy(manager, &SearchManager::enableFullTextSearchChanged);

    manager->onDConfigValueChanged(config, key);

    EXPECT_EQ(spy.count(), 0);
}
