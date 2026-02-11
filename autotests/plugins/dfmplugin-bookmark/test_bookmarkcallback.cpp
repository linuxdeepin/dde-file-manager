// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "bookmarkcallback.h"
#include "controller/bookmarkmanager.h"
#include "events/bookmarkeventcaller.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-framework/event/event.h>

#include <dfm-io/dfile.h>

#include <DMenu>
#include <DDialog>

#include <QApplication>
#include <QUrl>
#include <QPoint>
#include <QFileInfo>
#include <QTimer>
#include <QAction>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace BookmarkCallBack;
using namespace dfmplugin_bookmark;
using namespace dpf;

class UT_BookmarkCallBack : public testing::Test
{
protected:
    virtual void SetUp() override { }

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;

    const QUrl testUrl = QUrl::fromLocalFile("/home/test");
    const QUrl networkUrl = QUrl("smb://192.168.1.100/share");
    const QUrl ftpUrl = QUrl("ftp://192.168.1.100/share");
    const quint64 testWindowId = 12345;
    const QPoint testGlobalPos = QPoint(100, 200);
    const QString testName = "Test Bookmark Name";
};

TEST_F(UT_BookmarkCallBack, ContextMenuHandle_ValidFile)
{
    bool menuCreated = false;

    // Mock file exists
    stub.set_lamda(qOverload<>(&QFileInfo::exists), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock menu creation and execution
    stub.set_lamda(qOverload<const QPoint &, QAction *>(&DMenu::exec), [&] {
        __DBG_STUB_INVOKE__
        menuCreated = true;
        return nullptr;
    });

    // Mock event caller methods
    stub.set_lamda(BookMarkEventCaller::sendCheckTabAddable, [](quint64) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(contextMenuHandle(testWindowId, testUrl, testGlobalPos));
    EXPECT_TRUE(menuCreated);
}

TEST_F(UT_BookmarkCallBack, ContextMenuHandle_InvalidFile)
{
    bool menuCreated = false;

    // Mock file does not exist
    stub.set_lamda(qOverload<>(&QFileInfo::exists), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock menu creation and execution
    stub.set_lamda(qOverload<const QPoint &, QAction *>(&DMenu::exec), [&] {
        __DBG_STUB_INVOKE__
        menuCreated = true;
        return nullptr;
    });

    EXPECT_NO_FATAL_FAILURE(contextMenuHandle(testWindowId, testUrl, testGlobalPos));
    EXPECT_TRUE(menuCreated);
}

TEST_F(UT_BookmarkCallBack, RenameCallBack)
{
    bool isCall { false };
    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QUrl, QVariantMap &);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return true;
    });
    stub.set_lamda(&BookMarkManager::bookMarkRename, [] { return true; });
    BookmarkCallBack::renameCallBack(1, QUrl(), "test");

    EXPECT_TRUE(isCall);
}

TEST_F(UT_BookmarkCallBack, CdBookMarkUrlCallBack_FileExists)
{
    bool openInWindowCalled = false;

    // Mock bookmark exists in manager
    BookmarkData mockData;
    mockData.url = testUrl;
    mockData.name = "Test";

    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, BookmarkData> map;
        map[testUrl] = mockData;
        return map;
    });

    // Mock file exists
    stub.set_lamda(&dfmio::DFile::exists, [](const dfmio::DFile *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock network check
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusy, [](NetworkUtils *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock event caller
    stub.set_lamda(&BookMarkEventCaller::sendOpenBookMarkInWindow, [&](quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        openInWindowCalled = (winId == testWindowId && url == testUrl);
    });

    // Mock cursor restore
    stub.set_lamda(&QApplication::restoreOverrideCursor, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(cdBookMarkUrlCallBack(testWindowId, testUrl));
    EXPECT_TRUE(openInWindowCalled);
}

TEST_F(UT_BookmarkCallBack, CdBookMarkUrlCallBack_BookmarkNotFound)
{
    // Mock empty bookmark map
    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        return QMap<QUrl, BookmarkData>();
    });

    // Mock cursor restore
    stub.set_lamda(&QApplication::restoreOverrideCursor, []() {
        __DBG_STUB_INVOKE__
    });

    // Should return early without any further actions
    EXPECT_NO_FATAL_FAILURE(cdBookMarkUrlCallBack(testWindowId, testUrl));
}

TEST_F(UT_BookmarkCallBack, CdBookMarkUrlCallBack_NetworkBusy)
{
    bool dialogShown = false;

    // Mock bookmark exists
    BookmarkData mockData;
    mockData.url = networkUrl;
    mockData.name = "Network Share";

    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, BookmarkData> map;
        map[networkUrl] = mockData;
        return map;
    });

    // Mock network busy
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusy, [](NetworkUtils *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock dialog
    stub.set_lamda(&DialogManager::showUnableToVistDir, [&](DialogManager *, const QString &) {
        __DBG_STUB_INVOKE__
        dialogShown = true;
        return 0;
    });

    // Mock cursor restore
    stub.set_lamda(&QApplication::restoreOverrideCursor, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(cdBookMarkUrlCallBack(testWindowId, networkUrl));
    EXPECT_TRUE(dialogShown);
}

TEST_F(UT_BookmarkCallBack, CdBookMarkUrlCallBack_SMBFileNotExists)
{
    bool openInWindowCalled = false;
    bool removeDialogShown = false;

    // Mock bookmark exists
    BookmarkData mockData;
    mockData.url = networkUrl;
    mockData.name = "SMB Share";

    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, BookmarkData> map;
        map[networkUrl] = mockData;
        return map;
    });

    // Mock file does not exist
    stub.set_lamda(&dfmio::DFile::exists, [](const dfmio::DFile *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock network not busy
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusy, [](NetworkUtils *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock SMB protocol detection
    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&ProtocolUtils::isFTPFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock device utils
    QUrl sourceUrl = QUrl("smb://192.168.1.100/");
    stub.set_lamda(&DeviceUtils::parseNetSourceUrl, [&](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return sourceUrl;
    });

    // Mock event caller
    stub.set_lamda(&BookMarkEventCaller::sendOpenBookMarkInWindow, [&](quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        openInWindowCalled = (winId == testWindowId && url == sourceUrl);
    });

    // Mock cursor restore
    stub.set_lamda(&QApplication::restoreOverrideCursor, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(cdBookMarkUrlCallBack(testWindowId, networkUrl));
    EXPECT_TRUE(openInWindowCalled);
}

TEST_F(UT_BookmarkCallBack, CdBookMarkUrlCallBack_ShowRemoveDialog)
{
    bool removeDialogShown = false;
    bool bookmarkRemoved = false;

    // Mock bookmark exists
    BookmarkData mockData;
    mockData.url = testUrl;
    mockData.name = "Test";

    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, BookmarkData> map;
        map[testUrl] = mockData;
        return map;
    });

    // Mock file does not exist
    stub.set_lamda(&dfmio::DFile::exists, [](const dfmio::DFile *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock network not busy
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusy, [](NetworkUtils *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock not SMB/FTP
    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&ProtocolUtils::isFTPFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock remove dialog - user accepts
    stub.set_lamda(&BookMarkManager::showRemoveBookMarkDialog, [&](BookMarkManager *, quint64) -> int {
        __DBG_STUB_INVOKE__
        removeDialogShown = true;
        return DDialog::Accepted;
    });

    // Mock remove bookmark
    stub.set_lamda(&BookMarkManager::removeBookMark, [&](BookMarkManager *, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        bookmarkRemoved = (url == testUrl);
        return true;
    });

    // Mock cursor restore
    stub.set_lamda(&QApplication::restoreOverrideCursor, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_FATAL_FAILURE(cdBookMarkUrlCallBack(testWindowId, testUrl));
    EXPECT_TRUE(removeDialogShown);
    EXPECT_TRUE(bookmarkRemoved);
}

TEST_F(UT_BookmarkCallBack, CdDefaultItemUrlCallBack)
{
    bool openInWindowCalled = false;

    stub.set_lamda(&BookMarkEventCaller::sendOpenBookMarkInWindow, [&](quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        openInWindowCalled = (winId == testWindowId && url == testUrl);
    });

    EXPECT_NO_FATAL_FAILURE(cdDefaultItemUrlCallBack(testWindowId, testUrl));
    EXPECT_TRUE(openInWindowCalled);
}
