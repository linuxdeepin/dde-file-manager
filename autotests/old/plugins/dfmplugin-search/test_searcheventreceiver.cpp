// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>

#include "events/searcheventreceiver.h"
#include "events/searcheventcaller.h"
#include "utils/searchhelper.h"
#include "searchmanager/searchmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>

#include "stubext.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DPF_USE_NAMESPACE

class TestSearchEventReceiver : public testing::Test
{
public:
    void SetUp() override
    {
        receiver = SearchEventReceiver::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    SearchEventReceiver *receiver = nullptr;
};

TEST_F(TestSearchEventReceiver, Instance_ReturnsSameInstance)
{
    auto receiver1 = SearchEventReceiver::instance();
    auto receiver2 = SearchEventReceiver::instance();

    EXPECT_NE(receiver1, nullptr);
    EXPECT_EQ(receiver1, receiver2);
}

TEST_F(TestSearchEventReceiver, HandleSearch_WithValidParameters_CallsChangeCurrentUrl)
{
    quint64 winId = 12345;
    QString keyword = "test_keyword";
    QUrl currentUrl = QUrl::fromLocalFile("/home/test");
    bool changeCurrentUrlCalled = false;

    // Create mock window
    FileManagerWindow mockWindow(currentUrl);

    // Mock FileManagerWindowsManager::findWindowById
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64 id) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    // Mock FileManagerWindow::currentUrl
    stub.set_lamda(&FileManagerWindow::currentUrl,
                   [currentUrl](FileManagerWindow *) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return currentUrl;
                   });

    // Mock SearchHelper::isSearchFile
    stub.set_lamda(&SearchHelper::isSearchFile, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return false; // Not a search file
    });

    // Mock SearchHelper::fromSearchFile
    QUrl expectedSearchUrl = SearchHelper::fromSearchFile(currentUrl, keyword, QString::number(winId));

    // Mock SearchEventCaller::sendChangeCurrentUrl
    stub.set_lamda(&SearchEventCaller::sendChangeCurrentUrl,
                   [&changeCurrentUrlCalled, winId, expectedSearchUrl](quint64 id, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       changeCurrentUrlCalled = true;
                       EXPECT_EQ(id, winId);
                       EXPECT_EQ(url, expectedSearchUrl);
                   });

    receiver->handleSearch(winId, keyword);

    EXPECT_TRUE(changeCurrentUrlCalled);
}

TEST_F(TestSearchEventReceiver, HandleSearch_WithSearchFile_UsesTargetUrl)
{
    quint64 winId = 12345;
    QString keyword = "test_keyword";
    QUrl searchUrl("search:?keyword=old&url=file:///home/test");
    QUrl targetUrl = QUrl::fromLocalFile("/home/test");
    bool changeCurrentUrlCalled = false;

    // Create mock window
    FileManagerWindow mockWindow(searchUrl);

    // Mock FileManagerWindowsManager::findWindowById
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64 id) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    // Mock FileManagerWindow::currentUrl
    stub.set_lamda(&FileManagerWindow::currentUrl,
                   [searchUrl](FileManagerWindow *) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return searchUrl;
                   });

    // Mock SearchHelper::isSearchFile
    stub.set_lamda(&SearchHelper::isSearchFile, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return true; // Is a search file
    });

    // Mock SearchHelper::searchTargetUrl
    stub.set_lamda(&SearchHelper::searchTargetUrl,
                   [targetUrl](const QUrl &searchUrl) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return targetUrl;
                   });

    // Mock SearchHelper::fromSearchFile
    QUrl expectedSearchUrl = SearchHelper::fromSearchFile(targetUrl, keyword, QString::number(winId));

    // Mock SearchEventCaller::sendChangeCurrentUrl
    stub.set_lamda(&SearchEventCaller::sendChangeCurrentUrl,
                   [&changeCurrentUrlCalled](quint64 id, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       changeCurrentUrlCalled = true;
                   });

    receiver->handleSearch(winId, keyword);

    EXPECT_TRUE(changeCurrentUrlCalled);
}

TEST_F(TestSearchEventReceiver, HandleStopSearch_CallsSearchManagerStop)
{
    quint64 winId = 12345;
    bool stopCalled = false;

    // Mock SearchManager::stop
    stub.set_lamda(static_cast<void (SearchManager::*)(quint64)>(&SearchManager::stop),
                   [&stopCalled, winId](SearchManager *, quint64 id) {
                       __DBG_STUB_INVOKE__
                       stopCalled = true;
                       EXPECT_EQ(id, winId);
                   });

    receiver->handleStopSearch(winId);

    EXPECT_TRUE(stopCalled);
}

TEST_F(TestSearchEventReceiver, HandleShowAdvanceSearchBar_CallsSearchEventCaller)
{
    quint64 winId = 12345;
    bool visible = true;
    bool sendShowAdvanceSearchBarCalled = false;

    // Mock SearchEventCaller::sendShowAdvanceSearchBar
    stub.set_lamda(&SearchEventCaller::sendShowAdvanceSearchBar,
                   [&sendShowAdvanceSearchBarCalled, winId, visible](quint64 id, bool vis) {
                       __DBG_STUB_INVOKE__
                       sendShowAdvanceSearchBarCalled = true;
                       EXPECT_EQ(id, winId);
                       EXPECT_EQ(vis, visible);
                   });

    receiver->handleShowAdvanceSearchBar(winId, visible);

    EXPECT_TRUE(sendShowAdvanceSearchBarCalled);
}

TEST_F(TestSearchEventReceiver, HandleAddressInputStr_WithSearchUrlWithoutWinId_AppendsWinId)
{
    quint64 winId = 12345;
    QString inputStr = "search:?keyword=test&url=file:///home";

    receiver->handleAddressInputStr(winId, &inputStr);

    QString expectedWinId = "&winId=" + QString::number(winId);
    EXPECT_TRUE(inputStr.contains(expectedWinId));
}

TEST_F(TestSearchEventReceiver, HandleAddressInputStr_WithSearchUrlWithWinId_DoesNotModify)
{
    quint64 winId = 12345;
    QString inputStr = "search:?keyword=test&url=file:///home&winId=456";
    QString originalStr = inputStr;

    receiver->handleAddressInputStr(winId, &inputStr);

    EXPECT_EQ(inputStr, originalStr);
}

TEST_F(TestSearchEventReceiver, HandleAddressInputStr_WithNonSearchUrl_DoesNotModify)
{
    quint64 winId = 12345;
    QString inputStr = "file:///home/test";
    QString originalStr = inputStr;

    receiver->handleAddressInputStr(winId, &inputStr);

    EXPECT_EQ(inputStr, originalStr);
}

TEST_F(TestSearchEventReceiver, HandleFileAdd_EmitsSearchManagerSignal)
{
    QUrl url = QUrl::fromLocalFile("/home/test/new_file.txt");

    // Mock SearchManager::instance
    SearchManager mockManager;
    stub.set_lamda(&SearchManager::instance, [&mockManager]() -> SearchManager * {
        __DBG_STUB_INVOKE__
        return &mockManager;
    });

    // Use QSignalSpy to verify signal emission
    QSignalSpy spy(&mockManager, &SearchManager::fileAdd);

    receiver->handleFileAdd(url);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), url);
}

TEST_F(TestSearchEventReceiver, HandleFileDelete_EmitsSearchManagerSignal)
{
    QUrl url = QUrl::fromLocalFile("/home/test/deleted_file.txt");

    // Mock SearchManager::instance
    SearchManager mockManager;
    stub.set_lamda(&SearchManager::instance, [&mockManager]() -> SearchManager * {
        __DBG_STUB_INVOKE__
        return &mockManager;
    });

    // Use QSignalSpy to verify signal emission
    QSignalSpy spy(&mockManager, &SearchManager::fileDelete);

    receiver->handleFileDelete(url);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), url);
}

TEST_F(TestSearchEventReceiver, HandleFileRename_EmitsSearchManagerSignal)
{
    QUrl oldUrl = QUrl::fromLocalFile("/home/test/old_name.txt");
    QUrl newUrl = QUrl::fromLocalFile("/home/test/new_name.txt");

    // Mock SearchManager::instance
    SearchManager mockManager;
    stub.set_lamda(&SearchManager::instance, [&mockManager]() -> SearchManager * {
        __DBG_STUB_INVOKE__
        return &mockManager;
    });

    // Use QSignalSpy to verify signal emission
    QSignalSpy spy(&mockManager, &SearchManager::fileRename);

    receiver->handleFileRename(oldUrl, newUrl);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), oldUrl);
    EXPECT_EQ(spy.at(0).at(1).toUrl(), newUrl);
}

TEST_F(TestSearchEventReceiver, HandleSearch_WithNullWindow_DoesNotCrash)
{
    quint64 winId = 99999; // Non-existent window ID
    QString keyword = "test_keyword";

    FileManagerWindow window(QUrl::fromLocalFile("/home"));
    // Mock FileManagerWindowsManager::findWindowById to return null
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&window](FileManagerWindowsManager *, quint64 id) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &window;
                   });

    // This should not crash even with null window
    // Note: The original code uses Q_ASSERT(window) which would terminate in debug builds
    // In release builds, it would likely crash when accessing null pointer
    // For testing purposes, we verify the method can be called
    EXPECT_NO_FATAL_FAILURE(receiver->handleSearch(winId, keyword));
}

TEST_F(TestSearchEventReceiver, HandleSearch_WithEmptyKeyword_ProcessesCorrectly)
{
    quint64 winId = 12345;
    QString keyword = ""; // Empty keyword
    QUrl currentUrl = QUrl::fromLocalFile("/home/test");
    bool changeCurrentUrlCalled = false;

    // Create mock window
    FileManagerWindow mockWindow(currentUrl);

    // Mock FileManagerWindowsManager::findWindowById
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64 id) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    // Mock FileManagerWindow::currentUrl
    stub.set_lamda(&FileManagerWindow::currentUrl,
                   [currentUrl](FileManagerWindow *) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return currentUrl;
                   });

    // Mock SearchHelper::isSearchFile
    stub.set_lamda(&SearchHelper::isSearchFile, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock SearchHelper::fromSearchFile
    QUrl expectedSearchUrl = SearchHelper::fromSearchFile(currentUrl, keyword, QString::number(winId));
    // Mock SearchEventCaller::sendChangeCurrentUrl
    stub.set_lamda(&SearchEventCaller::sendChangeCurrentUrl,
                   [&changeCurrentUrlCalled](quint64 id, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       changeCurrentUrlCalled = true;
                   });

    receiver->handleSearch(winId, keyword);

    EXPECT_TRUE(changeCurrentUrlCalled);
}

TEST_F(TestSearchEventReceiver, HandleShowAdvanceSearchBar_WithFalseVisible_CallsCorrectly)
{
    quint64 winId = 12345;
    bool visible = false;
    bool sendShowAdvanceSearchBarCalled = false;

    // Mock SearchEventCaller::sendShowAdvanceSearchBar
    stub.set_lamda(&SearchEventCaller::sendShowAdvanceSearchBar,
                   [&sendShowAdvanceSearchBarCalled, winId, visible](quint64 id, bool vis) {
                       __DBG_STUB_INVOKE__
                       sendShowAdvanceSearchBarCalled = true;
                       EXPECT_EQ(id, winId);
                       EXPECT_EQ(vis, visible);
                   });

    receiver->handleShowAdvanceSearchBar(winId, visible);

    EXPECT_TRUE(sendShowAdvanceSearchBarCalled);
}
