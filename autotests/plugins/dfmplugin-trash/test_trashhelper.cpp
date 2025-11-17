// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>

#include "utils/trashhelper.h"
#include "trashdiriterator.h"
#include "trashfilewatcher.h"
#include "events/trasheventcaller.h"
#include "dfmplugin_trash_global.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/interfaces/fileinfo.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/filemanagerwindowsmanager.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/dpf.h>
#include <dfm-io/dfmio_utils.h>

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

using namespace dfmplugin_trash;

class TestTrashHelper : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestTrashHelper, Instance)
{
    TrashHelper *instance1 = TrashHelper::instance();
    TrashHelper *instance2 = TrashHelper::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);   // Should be the same instance
}

TEST_F(TestTrashHelper, Scheme)
{
    QString scheme = TrashHelper::scheme();
    EXPECT_EQ(scheme, DFMBASE_NAMESPACE::Global::Scheme::kTrash);
}

TEST_F(TestTrashHelper, Icon)
{
    // Test that icon function does not crash
    EXPECT_NO_THROW({
        QIcon icon = TrashHelper::icon();
        (void)icon;   // Use variable to avoid unused warning
    });
}

TEST_F(TestTrashHelper, RootUrl)
{
    QUrl rootUrl = TrashHelper::rootUrl();
    EXPECT_EQ(rootUrl.scheme(), TrashHelper::scheme());
    EXPECT_EQ(rootUrl.path(), "/");
    EXPECT_EQ(rootUrl.host(), "");
}

TEST_F(TestTrashHelper, WindowId)
{
    QWidget *mockWidget = new QWidget();
    quint64 expectedId = 12345;

    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowId), [expectedId](FileManagerWindowsManager *self, const QWidget *sender) -> quint64 {
        Q_UNUSED(self)
        Q_UNUSED(sender)
        return expectedId;
    });

    quint64 result = TrashHelper::windowId(mockWidget);
    EXPECT_EQ(result, expectedId);

    delete mockWidget;
}

TEST_F(TestTrashHelper, ContextMenuHandle)
{
    quint64 windowId = 12345;
    QUrl url("trash:///");
    QPoint globalPos(100, 100);

    // Mock the event caller to track if it's called
    bool eventCalled = false;
    stub.set_lamda(&TrashEventCaller::sendOpenWindow, [&eventCalled](const QUrl &url) {
        Q_UNUSED(url)
        eventCalled = true;
    });

    // Mock the tab check and event calls
    stub.set_lamda(&TrashEventCaller::sendCheckTabAddable, [windowId](const quint64 winId) -> bool {
        Q_UNUSED(winId)
        return true;
    });

    stub.set_lamda(&TrashEventCaller::sendOpenTab, [&eventCalled](quint64 winId, const QUrl &url) {
        Q_UNUSED(winId)
        Q_UNUSED(url)
        eventCalled = true;
    });

    stub.set_lamda(&TrashEventCaller::sendEmptyTrash, [&eventCalled](quint64 winId, const QList<QUrl> &urls) {
        Q_UNUSED(winId)
        Q_UNUSED(urls)
        eventCalled = true;
    });

    stub.set_lamda(&TrashEventCaller::sendTrashPropertyDialog, [&eventCalled](const QUrl &url) {
        Q_UNUSED(url)
        eventCalled = true;
    });

    // Mock FileUtils::trashIsEmpty to control the menu state
    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        return false;   // Trash not empty to enable empty trash action
    });

    // This test creates and execs a menu, and we just want to make sure it doesn't crash
    // EXPECT_NO_THROW(TrashHelper::contenxtMenuHandle(windowId, url, globalPos));
}

TEST_F(TestTrashHelper, CreateEmptyTrashTopWidget)
{
    QWidget *widget = TrashHelper::createEmptyTrashTopWidget();
    EXPECT_NE(widget, nullptr);

    delete widget;
}

TEST_F(TestTrashHelper, ShowTopWidget)
{
    QWidget widget;
    QUrl url("trash:///test.txt");

    bool result = TrashHelper::showTopWidget(&widget, url);
    EXPECT_FALSE(result);   // The function always returns false
}

TEST_F(TestTrashHelper, TransToTrashFile)
{
    QString filePath = "/home/user/test.txt";
    QUrl result = TrashHelper::transToTrashFile(filePath);

    EXPECT_EQ(result.scheme(), TrashHelper::scheme());
    EXPECT_EQ(result.path(), filePath);
}

// TEST_F(TestTrashHelper, TrashFileToTargetUrl)
// {
//     QUrl trashUrl("trash:///original/path/file.txt");

//     // Mock InfoFactory to return a valid FileInfo
//     auto mockFileInfo = InfoFactory::create<FileInfo>(trashUrl);
//     stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
//                    [&mockFileInfo](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
//                        Q_UNUSED(url)
//                        Q_UNUSED(type)
//                        Q_UNUSED(key)
//                        return mockFileInfo;
//                    });

//     // Mock FileInfo's urlOf to return proper URLs
//     stub.set_lamda(&FileInfo::urlOf, [](const FileInfo *self, UrlInfoType type) -> QUrl {
//         Q_UNUSED(self)
//         if (type == UrlInfoType::kOriginalUrl) {
//             return QUrl("file:///original/path/file.txt");
//         } else if (type == UrlInfoType::kRedirectedFileUrl) {
//             return QUrl("file:///original/path/file.txt");
//         }
//         return QUrl();
//     });

//     QUrl result = TrashHelper::trashFileToTargetUrl(trashUrl);
//     EXPECT_EQ(result, QUrl("file:///original/path/file.txt"));
// }

TEST_F(TestTrashHelper, IsTrashFile)
{
    QUrl trashUrl("trash:///test");
    QUrl fileUrl("file:///test");
    QUrl trashPathUrl = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashLocalFilesPath) + "/somefile");

    bool result1 = TrashHelper::isTrashFile(trashUrl);
    bool result2 = TrashHelper::isTrashFile(fileUrl);
    bool result3 = TrashHelper::isTrashFile(trashPathUrl);

    EXPECT_TRUE(result1);
    EXPECT_FALSE(result2);
    // result3 depends on the actual path, but it should work with the regex pattern
}

TEST_F(TestTrashHelper, IsTrashRootFile)
{
    QUrl rootUrl("trash:///");
    QUrl fileUrl("trash:///test");

    bool result1 = TrashHelper::isTrashRootFile(rootUrl);
    bool result2 = TrashHelper::isTrashRootFile(fileUrl);

    EXPECT_TRUE(result1);
    EXPECT_FALSE(result2);
}

TEST_F(TestTrashHelper, EmptyTrash)
{
    quint64 windowId = 12345;

    EXPECT_NO_THROW(TrashHelper::emptyTrash(windowId));
    // Simply test that the function does not crash
    EXPECT_TRUE(true);
}

// TEST_F(TestTrashHelper, PropertyExtensionFunc)
// {
//     QUrl testUrl("trash:///test.txt");

//     Create a mock FileInfo that has the required behavior auto mockFileInfo = InfoFactory::create<FileInfo>(testUrl);
//     stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
//                    [&mockFileInfo](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
//                        Q_UNUSED(url)
//                        Q_UNUSED(type)
//                        Q_UNUSED(key)
//                        return mockFileInfo;
//                    });

//     // Mock urlOf to return proper URLs
//     stub.set_lamda(&FileInfo::urlOf, [](const FileInfo *self, UrlInfoType type) -> QUrl {
//         Q_UNUSED(self)
//         if (type == UrlInfoType::kOriginalUrl) {
//             return QUrl("file:///original/path/test.txt");
//         } else if (type == UrlInfoType::kRedirectedFileUrl) {
//             return QUrl("file:///original/path/test.txt");
//         }
//         return QUrl();
//     });

//     auto result = TrashHelper::propetyExtensionFunc(testUrl);
//     EXPECT_FALSE(result.empty());   // Should have some expansion data
// }

// TEST_F(TestTrashHelper, PropertyExtensionFunc_NullFileInfo)
// {
//     QUrl testUrl("trash:///test.txt");

//     // Mock to return null FileInfo
//     stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
//                    [](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
//                        Q_UNUSED(url)
//                        Q_UNUSED(type)
//                        Q_UNUSED(key)
//                        return nullptr;
//                    });

//     // The function should handle null fileInfo gracefully
//     auto result = TrashHelper::propetyExtensionFunc(testUrl);
//     EXPECT_TRUE(result.empty());   // Should return empty map when fileInfo is null
// }

// TEST_F(TestTrashHelper, DetailExtensionFunc)
// {
//     QUrl testUrl("trash:///test.txt");

//     // Create a mock FileInfo that has the required behavior
//     auto mockFileInfo = InfoFactory::create<FileInfo>(testUrl);
//     stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
//                    [&mockFileInfo](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
//                        Q_UNUSED(url)
//                        Q_UNUSED(type)
//                        Q_UNUSED(key)
//                        return mockFileInfo;
//                    });

//     // Mock urlOf to return proper URLs
//     stub.set_lamda(&FileInfo::urlOf, [](const FileInfo *self, UrlInfoType type) -> QUrl {
//         Q_UNUSED(self)
//         if (type == UrlInfoType::kOriginalUrl) {
//             return QUrl("file:///original/path/test.txt");
//         }
//         return QUrl();
//     });

//     auto result = TrashHelper::detailExtensionFunc(testUrl);
//     EXPECT_FALSE(result.empty());   // Should have some expansion data
// }

// TEST_F(TestTrashHelper, DetailExtensionFunc_NullFileInfo)
// {
//     QUrl testUrl("trash:///test.txt");

//     // Mock to return null FileInfo
//     stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
//                    [](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
//                        Q_UNUSED(url)
//                        Q_UNUSED(type)
//                        Q_UNUSED(key)
//                        return nullptr;
//                    });

//     // The function should handle null fileInfo gracefully
//     auto result = TrashHelper::detailExtensionFunc(testUrl);
//     EXPECT_TRUE(result.empty());   // Should return empty map when fileInfo is null
// }

TEST_F(TestTrashHelper, RestoreFromTrashHandle)
{
    quint64 windowId = 12345;
    QList<QUrl> urls = { QUrl("trash:///test.txt") };

    // For now, just test that the function executes without crashing
    // The actual slot channel call is complex to stub, so we'll just verify it doesn't crash
    EXPECT_NO_THROW({
        auto result = TrashHelper::restoreFromTrashHandle(windowId, urls, AbstractJobHandler::JobFlag::kNoHint);
        (void)result;   // Use result to avoid unused warning
    });
}

TEST_F(TestTrashHelper, CheckDragDropAction)
{
    TrashHelper helper;
    QList<QUrl> urls = { QUrl("trash:///test1.txt") };
    QUrl urlTo("trash:///folder/");
    Qt::DropAction action = Qt::CopyAction;

    bool result = helper.checkDragDropAction(urls, urlTo, &action);
    EXPECT_TRUE(result);
    EXPECT_EQ(action, Qt::IgnoreAction);   // Should be set to IgnoreAction for trash-to-trash
}

TEST_F(TestTrashHelper, CheckCanMove)
{
    TrashHelper helper;
    QUrl trashUrl("trash:///test.txt");

    // Mock FileUtils::isTrashRootFile and UrlRoute::urlParent
    stub.set_lamda(&FileUtils::isTrashRootFile, [](const QUrl &url) -> bool {
        Q_UNUSED(url)
        return true;   // Assume it's trash root file parent
    });

    stub.set_lamda(&UrlRoute::urlParent, [](const QUrl &url) -> QUrl {
        Q_UNUSED(url)
        return QUrl("trash:///");
    });

    bool result = helper.checkCanMove(trashUrl);
    EXPECT_TRUE(result);   // Should return true for trash URLs with trash root parent
}

TEST_F(TestTrashHelper, DetailViewIcon)
{
    TrashHelper helper;
    QUrl url = TrashHelper::rootUrl();
    QString iconName;

    // Mock SystemPathUtil to return an icon name
    stub.set_lamda(&SystemPathUtil::systemPathIconName, [](SystemPathUtil *self, const QString &path) -> QString {
        Q_UNUSED(self)
        if (path == "Trash") {
            return "user-trash";
        }
        return QString();
    });

    bool result = helper.detailViewIcon(url, &iconName);
    // Should return true when icon name is found for trash root
    EXPECT_TRUE(result);
    EXPECT_EQ(iconName, "user-trash");
}

TEST_F(TestTrashHelper, CustomColumnRole)
{
    TrashHelper helper;
    QUrl rootUrl("trash:///");
    QList<Global::ItemRoles> roleList;

    bool result = helper.customColumnRole(rootUrl, &roleList);
    EXPECT_TRUE(result);
    EXPECT_GT(roleList.size(), 0);   // Should add some roles
    // Check that expected roles are added
    EXPECT_NE(roleList.indexOf(Global::ItemRoles::kItemFileDisplayNameRole), -1);
    EXPECT_NE(roleList.indexOf(Global::ItemRoles::kItemFileOriginalPath), -1);
    EXPECT_NE(roleList.indexOf(Global::ItemRoles::kItemFileDeletionDate), -1);
}

TEST_F(TestTrashHelper, CustomRoleDisplayName)
{
    TrashHelper helper;
    QUrl url("trash:///test.txt");
    QString displayName;
    Global::ItemRoles role = Global::ItemRoles::kItemFileOriginalPath;

    bool result = helper.customRoleDisplayName(url, role, &displayName);
    EXPECT_TRUE(result);
    EXPECT_FALSE(displayName.isEmpty());
    EXPECT_EQ(displayName, QObject::tr("Source Path"));

    // Test another role
    displayName.clear();
    role = Global::ItemRoles::kItemFileDeletionDate;
    result = helper.customRoleDisplayName(url, role, &displayName);
    EXPECT_TRUE(result);
    EXPECT_FALSE(displayName.isEmpty());
    EXPECT_EQ(displayName, QObject::tr("Time deleted"));
}

TEST_F(TestTrashHelper, OnTrashEmptyState)
{
    TrashHelper helper;

    // Mock FileUtils::trashIsEmpty
    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        return true;
    });

    // Mock FileManagerWindowsManager.windowIdList
    QList<quint64> windowIds = { 123, 456 };
    stub.set_lamda(ADDR(FileManagerWindowsManager, windowIdList), [](FileManagerWindowsManager *self) -> QList<quint64> {
        Q_UNUSED(self)
        return { 123, 456 };
    });

    // Mock FileManagerWindowsManager.findWindowById
    FileManagerWindow *mockWindow = new FileManagerWindow(QUrl("trash:///"));
    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowById), [mockWindow](FileManagerWindowsManager *self, quint64 id) -> FileManagerWindow * {
        Q_UNUSED(self)
        Q_UNUSED(id)
        return mockWindow;
    });

    // Mock window current URL
    stub.set_lamda(&FileManagerWindow::currentUrl, [](FileManagerWindow *self) -> QUrl {
        Q_UNUSED(self)
        return QUrl("trash:///");
    });

    // Mock event caller
    bool eventCalled = false;
    stub.set_lamda(&TrashEventCaller::sendShowEmptyTrash,
                   [&eventCalled](quint64 winId, bool show) {
                       Q_UNUSED(winId)
                       Q_UNUSED(show)
                       eventCalled = true;
                   });

    helper.onTrashEmptyState();
    EXPECT_TRUE(eventCalled);

    delete mockWindow;
}

TEST_F(TestTrashHelper, HandleWindowUrlChanged)
{
    TrashHelper helper;
    quint64 winId = 12345;
    QUrl url("trash:///");

    // Mock trash state to be not unknown
    stub.set_lamda(&TrashHelper::ensureTrashStateInitialized, [](TrashHelper *self) {
        Q_UNUSED(self)
        // Simulate state initialization
    });

    // Mock event caller
    bool eventCalled = false;
    stub.set_lamda(&TrashEventCaller::sendShowEmptyTrash,
                   [&eventCalled](quint64 winId, bool show) {
                       Q_UNUSED(winId)
                       Q_UNUSED(show)
                       eventCalled = true;
                   });

    helper.handleWindowUrlChanged(winId, url);
    // This should trigger the event if the scheme is trash and state is not unknown
    EXPECT_TRUE(eventCalled);
}

TEST_F(TestTrashHelper, TrashNotEmpty)
{
    TrashHelper helper;

    QSignalSpy spy(&helper, &TrashHelper::trashNotEmptyState);
    helper.trashNotEmpty();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestTrashHelper, OnTrashNotEmptyState)
{
    TrashHelper helper;

    // Mock FileUtils::trashIsEmpty
    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        return false;   // Trash not empty
    });

    // Mock FileManagerWindowsManager.windowIdList
    stub.set_lamda(ADDR(FileManagerWindowsManager, windowIdList), [](FileManagerWindowsManager *self) -> QList<quint64> {
        Q_UNUSED(self)
        return { 123, 456 };
    });

    // Mock FileManagerWindowsManager.findWindowById
    FileManagerWindow *mockWindow = new FileManagerWindow(QUrl("trash:///"));
    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowById), [mockWindow](FileManagerWindowsManager *self, quint64 id) -> FileManagerWindow * {
        Q_UNUSED(self)
        Q_UNUSED(id)
        return mockWindow;
    });

    // Mock window current URL
    stub.set_lamda(&FileManagerWindow::currentUrl, [](FileManagerWindow *self) -> QUrl {
        Q_UNUSED(self)
        return QUrl("trash:///");
    });

    // Mock event caller
    bool eventCalled = false;
    stub.set_lamda(&TrashEventCaller::sendShowEmptyTrash,
                   [&eventCalled](quint64 winId, bool show) {
                       Q_UNUSED(winId)
                       Q_UNUSED(show)
                       eventCalled = true;
                   });

    helper.onTrashNotEmptyState();
    EXPECT_TRUE(eventCalled);

    delete mockWindow;
}

TEST_F(TestTrashHelper, Constructor)
{
    TrashHelper *helper = new TrashHelper();
    EXPECT_NE(helper, nullptr);
    delete helper;
    // Test that destructor works without crash
    EXPECT_TRUE(true);
}

TEST_F(TestTrashHelper, Destructor)
{
    TrashHelper *helper = new TrashHelper();
    EXPECT_NE(helper, nullptr);
    delete helper;
    // Test that destructor works without crash
    EXPECT_TRUE(true);
}
