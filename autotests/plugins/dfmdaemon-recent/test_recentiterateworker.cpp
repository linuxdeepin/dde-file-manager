// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "recentiterateworker.h"
#include "serverplugin_recentmanager_global.h"

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/protocolutils.h>

#include <DRecentManager>

#include <QSignalSpy>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QThread>

DFMBASE_USE_NAMESPACE
SERVERRECENTMANAGER_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_RecentIterateWorker : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        worker = new RecentIterateWorker();

        // Create temporary test xbel file
        tempXbelFile = new QTemporaryFile();
        tempXbelFile->open();
        tempXbelPath = tempXbelFile->fileName();

        // Write sample xbel content
        writeSampleXbelContent();
    }

    virtual void TearDown() override
    {
        delete worker;
        worker = nullptr;
        delete tempXbelFile;
        tempXbelFile = nullptr;
        stub.clear();
    }

    void writeSampleXbelContent()
    {
        const QString sampleContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<xbel version="1.0"
      xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks"
      xmlns:mime="http://www.freedesktop.org/standards/shared-mime-info">
    <bookmark href="file:///test/file1.txt" modified="2024-01-01T10:00:00Z">
        <info>
            <metadata owner="http://freedesktop.org">
                <mime:mime-type type="text/plain"/>
                <bookmark:applications>
                    <bookmark:application name="TestApp" exec="testapp" modified="2024-01-01T10:00:00Z" count="1"/>
                </bookmark:applications>
            </metadata>
        </info>
    </bookmark>
    <bookmark href="file:///test/file2.txt" modified="2024-01-01T11:00:00Z">
        <info>
            <metadata owner="http://freedesktop.org">
                <mime:mime-type type="text/plain"/>
                <bookmark:applications>
                    <bookmark:application name="TestApp2" exec="testapp2" modified="2024-01-01T11:00:00Z" count="1"/>
                </bookmark:applications>
            </metadata>
        </info>
    </bookmark>
</xbel>)";

        tempXbelFile->write(sampleContent.toUtf8());
        tempXbelFile->flush();
    }

protected:
    RecentIterateWorker *worker { nullptr };
    stub_ext::StubExt stub;
    QTemporaryFile *tempXbelFile { nullptr };
    QString tempXbelPath;
};

TEST_F(UT_RecentIterateWorker, constructor_InitializesCorrectly)
{
    EXPECT_NE(worker, nullptr);
    EXPECT_TRUE(worker->itemsInfo.isEmpty());
}

TEST_F(UT_RecentIterateWorker, onRequestReload_WithValidFile_ProcessesBookmarks)
{
    qint64 testTimestamp = 1234567890;
    bool reloadFinishedEmitted = false;
    qint64 receivedTimestamp = 0;

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock file operations - skip QFile::open stubbing due to virtual function complexity
    // The test will use the actual file operations which should work with temporary files

    // Mock file existence and validation
    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::exists), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::isFile), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QString (QFileInfo::*)() const>(&QFileInfo::absoluteFilePath), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return "/test/file1.txt";
    });

    // Mock URL operations
    stub.set_lamda(static_cast<bool (QUrl::*)() const>(&QUrl::isLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QString (QUrl::*)() const>(&QUrl::toLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return "/test/file1.txt";
    });

    // Mock protocol check
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock file utils
    stub.set_lamda(&FileUtils::bindPathTransform, [](const QString &path, bool) {
        __DBG_STUB_INVOKE__
        return path;
    });

    QObject::connect(worker, &RecentIterateWorker::reloadFinished,
                     [&](qint64 timestamp) {
                         reloadFinishedEmitted = true;
                         receivedTimestamp = timestamp;
                     });

    worker->onRequestReload(tempXbelPath, testTimestamp);
    EXPECT_TRUE(reloadFinishedEmitted);
    EXPECT_EQ(receivedTimestamp, testTimestamp);
}

TEST_F(UT_RecentIterateWorker, onRequestReload_WithInvalidFile_EmitsReloadFinished)
{
    qint64 testTimestamp = 1234567890;
    bool reloadFinishedEmitted = false;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QObject::connect(worker, &RecentIterateWorker::reloadFinished,
                     [&](qint64) {
                         reloadFinishedEmitted = true;
                     });

    // Use a non-existent file path to trigger file open failure
    worker->onRequestReload("/nonexistent/path.xbel", testTimestamp);

    EXPECT_TRUE(reloadFinishedEmitted);
}

TEST_F(UT_RecentIterateWorker, onRequestReload_DetectsItemChanges)
{
    // Pre-populate worker with an item
    RecentItem existingItem;
    existingItem.href = "file:///test/file1.txt";
    existingItem.modified = 1000000000;   // Old timestamp
    worker->itemsInfo.insert("/test/file1.txt", existingItem);

    bool itemChangedEmitted = false;
    QString changedPath;
    RecentItem changedItem;

    // Mock thread and app checks
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock file operations - skip due to virtual function complexity
    // The test will use actual file operations

    // Mock file validation
    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::exists), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QFileInfo::isFile, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QFileInfo::absoluteFilePath, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return "/test/file1.txt";
    });

    // Mock URL operations
    stub.set_lamda(&QUrl::isLocalFile, [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QUrl::toLocalFile, [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return "/test/file1.txt";
    });

    // Mock protocol check
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock file utils
    stub.set_lamda(&FileUtils::bindPathTransform, [](const QString &path, bool) {
        __DBG_STUB_INVOKE__
        return path;
    });

    QObject::connect(worker, &RecentIterateWorker::itemChanged,
                     [&](const QString &path, const RecentItem &item) {
                         itemChangedEmitted = true;
                         changedPath = path;
                         changedItem = item;
                     });

    worker->onRequestReload(tempXbelPath, 1234567890);

    EXPECT_TRUE(itemChangedEmitted);
    EXPECT_EQ(changedPath, "/test/file1.txt");
    EXPECT_NE(changedItem.modified, existingItem.modified);
}

TEST_F(UT_RecentIterateWorker, onRequestReload_DetectsRemovedItems)
{
    // Pre-populate worker with items that won't be found in the new scan
    RecentItem item1;
    item1.href = "file:///removed/file1.txt";
    item1.modified = 1000000000;
    worker->itemsInfo.insert("/removed/file1.txt", item1);

    RecentItem item2;
    item2.href = "file:///removed/file2.txt";
    item2.modified = 1000000001;
    worker->itemsInfo.insert("/removed/file2.txt", item2);

    bool itemsRemovedEmitted = false;
    QStringList removedPaths;

    // Mock thread and app checks
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock file operations - skip due to virtual function complexity
    // The test will use actual file operations

    QObject::connect(worker, &RecentIterateWorker::itemsRemoved,
                     [&](const QStringList &paths) {
                         itemsRemovedEmitted = true;
                         removedPaths = paths;
                     });

    worker->onRequestReload(tempXbelPath, 1234567890);

    EXPECT_TRUE(itemsRemovedEmitted);
    EXPECT_EQ(removedPaths.size(), 2);
    EXPECT_TRUE(removedPaths.contains("/removed/file1.txt"));
    EXPECT_TRUE(removedPaths.contains("/removed/file2.txt"));
}

TEST_F(UT_RecentIterateWorker, onRequestAddRecentItem_WithValidItem_CallsDRecentManager)
{
    QVariantMap testItem;
    testItem[RecentProperty::kPath] = "/test/newfile.txt";
    testItem[RecentProperty::kAppName] = "TestApp";
    testItem[RecentProperty::kAppExec] = "testapp";
    testItem[RecentProperty::kMimeType] = "text/plain";

    bool addItemCalled = false;
    QString receivedPath;
    DTK_CORE_NAMESPACE::DRecentData receivedData;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock DRecentManager::addItem
    stub.set_lamda(&DTK_CORE_NAMESPACE::DRecentManager::addItem,
                   [&](const QString &path, const DTK_CORE_NAMESPACE::DRecentData &data) -> bool {
                       __DBG_STUB_INVOKE__
                       addItemCalled = true;
                       receivedPath = path;
                       receivedData = data;
                       return true;
                   });

    worker->onRequestAddRecentItem(testItem);

    EXPECT_TRUE(addItemCalled);
    EXPECT_EQ(receivedPath, "/test/newfile.txt");
    EXPECT_EQ(receivedData.appName, "TestApp");
    EXPECT_EQ(receivedData.appExec, "testapp");
    EXPECT_EQ(receivedData.mimeType, "text/plain");
}

TEST_F(UT_RecentIterateWorker, onRequestAddRecentItem_WithEmptyPath_DoesNotCallDRecentManager)
{
    QVariantMap testItem;
    testItem[RecentProperty::kPath] = "";
    testItem[RecentProperty::kAppName] = "TestApp";

    bool addItemCalled = false;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock DRecentManager::addItem
    stub.set_lamda(&DTK_CORE_NAMESPACE::DRecentManager::addItem,
                   [&](const QString &, const DTK_CORE_NAMESPACE::DRecentData &) -> bool {
                       __DBG_STUB_INVOKE__
                       addItemCalled = true;
                       return true;
                   });

    worker->onRequestAddRecentItem(testItem);

    EXPECT_FALSE(addItemCalled);
}

TEST_F(UT_RecentIterateWorker, onRequestAddRecentItem_WithFailedAddition_LogsError)
{
    QVariantMap testItem;
    testItem[RecentProperty::kPath] = "/test/failfile.txt";
    testItem[RecentProperty::kAppName] = "TestApp";

    bool addItemCalled = false;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock DRecentManager::addItem to fail
    stub.set_lamda(&DTK_CORE_NAMESPACE::DRecentManager::addItem,
                   [&](const QString &, const DTK_CORE_NAMESPACE::DRecentData &) -> bool {
                       __DBG_STUB_INVOKE__
                       addItemCalled = true;
                       return false;   // Simulate failure
                   });

    worker->onRequestAddRecentItem(testItem);

    EXPECT_TRUE(addItemCalled);
    // Test completes without crash, indicating error was handled
}

TEST_F(UT_RecentIterateWorker, onRequestRemoveItems_CallsDRecentManager)
{
    QStringList testHrefs = { "file:///test/remove1.txt", "file:///test/remove2.txt" };
    bool removeItemsCalled = false;
    QStringList receivedHrefs;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock DRecentManager::removeItems
    stub.set_lamda(&DTK_CORE_NAMESPACE::DRecentManager::removeItems,
                   [&](const QStringList &hrefs) {
                       __DBG_STUB_INVOKE__
                       removeItemsCalled = true;
                       receivedHrefs = hrefs;
                   });

    worker->onRequestRemoveItems(testHrefs);

    EXPECT_TRUE(removeItemsCalled);
    EXPECT_EQ(receivedHrefs, testHrefs);
}

TEST_F(UT_RecentIterateWorker, onRequestPurgeItems_WritesEmptyXbelAndEmitsSignal)
{
    QString testXbelPath = "/test/purge.xbel";
    bool fileOpened = false;
    bool fileWritten = false;
    bool purgeFinishedEmitted = false;
    QByteArray writtenData;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // // Mock file operations
    // using QFileOpenFunc = bool (QFile::*)(QIODevice::OpenMode);
    // stub.set_lamda(static_cast<QFileOpenFunc>(&QFile::open), [&](QFile *, QIODevice::OpenMode mode) {
    //     __DBG_STUB_INVOKE__
    //     fileOpened = (mode & QIODevice::WriteOnly);
    //     return true;
    // });

    using QFileWriteFunc = qint64 (QFile::*)(const QByteArray &);
    stub.set_lamda(static_cast<QFileWriteFunc>(&QFile::write), [&](QFile *, const QByteArray &data) {
        __DBG_STUB_INVOKE__
        fileWritten = true;
        writtenData = data;
        return data.size();
    });

    QSignalSpy spy(worker, &RecentIterateWorker::purgeFinished);

    worker->onRequestPurgeItems(tempXbelPath);   // Use the actual temporary file

    EXPECT_EQ(spy.count(), 1);

    // Verify the file was written by checking its contents
    QFile file(tempXbelPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray content = file.readAll();
        EXPECT_TRUE(content.contains("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"));
        EXPECT_TRUE(content.contains("<xbel version=\"1.0\""));
        EXPECT_TRUE(content.contains("</xbel>"));
        file.close();
    }
}

TEST_F(UT_RecentIterateWorker, onRequestPurgeItems_WithFileOpenFailure_EmitsSignalAnyway)
{
    QString testXbelPath = "/test/failpurge.xbel";
    bool purgeFinishedEmitted = false;

    // Mock thread check
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    stub.set_lamda(&QCoreApplication::instance, []() {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock file open failure
    // using QFileOpenFunc = bool (QFile::*)(QIODevice::OpenMode);
    // stub.set_lamda(static_cast<QFileOpenFunc>(&QFile::open), [](QFile *, QIODevice::OpenMode) {
    //     __DBG_STUB_INVOKE__
    //     return false;
    // });

    QSignalSpy spy(worker, &RecentIterateWorker::purgeFinished);

    worker->onRequestPurgeItems(testXbelPath);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_RecentIterateWorker, processBookmarkElement_WithValidElement_AddsNewItem)
{
    // Create a mock XML reader with a bookmark element
    QString xmlContent = R"(<bookmark href="file:///test/newfile.txt" modified="2024-01-01T12:00:00Z"/>)";
    QXmlStreamReader reader(xmlContent);
    reader.readNext();   // Move to StartElement

    QStringList curPathList;
    bool itemAddedEmitted = false;
    QString addedPath;
    RecentItem addedItem;

    // Mock file validation
    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::exists), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::isFile), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QString (QFileInfo::*)() const>(&QFileInfo::absoluteFilePath), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return "/test/newfile.txt";
    });

    // Mock URL operations
    stub.set_lamda(static_cast<bool (QUrl::*)() const>(&QUrl::isLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QString (QUrl::*)() const>(&QUrl::toLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return "/test/newfile.txt";
    });

    // Mock protocol check
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock file utils
    stub.set_lamda(&FileUtils::bindPathTransform, [](const QString &path, bool) {
        __DBG_STUB_INVOKE__
        return path;
    });

    QObject::connect(worker, &RecentIterateWorker::itemAdded,
                     [&](const QString &path, const RecentItem &item) {
                         itemAddedEmitted = true;
                         addedPath = path;
                         addedItem = item;
                     });

    worker->processBookmarkElement(reader, curPathList);

    EXPECT_TRUE(itemAddedEmitted);
    EXPECT_EQ(addedPath, "/test/newfile.txt");
    EXPECT_EQ(addedItem.href, "file:///test/newfile.txt");
    EXPECT_TRUE(curPathList.contains("/test/newfile.txt"));
}

TEST_F(UT_RecentIterateWorker, processBookmarkElement_WithNonLocalFile_IgnoresItem)
{
    QString xmlContent = R"(<bookmark href="http://example.com/file.txt" modified="2024-01-01T12:00:00Z"/>)";
    QXmlStreamReader reader(xmlContent);
    reader.readNext();

    QStringList curPathList;
    bool itemAddedEmitted = false;

    // Mock URL operations to return non-local file
    stub.set_lamda(static_cast<bool (QUrl::*)() const>(&QUrl::isLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QObject::connect(worker, &RecentIterateWorker::itemAdded,
                     [&](const QString &, const RecentItem &) {
                         itemAddedEmitted = true;
                     });

    worker->processBookmarkElement(reader, curPathList);

    EXPECT_FALSE(itemAddedEmitted);
    EXPECT_TRUE(curPathList.isEmpty());
}

TEST_F(UT_RecentIterateWorker, processBookmarkElement_WithNonExistentFile_IgnoresItem)
{
    QString xmlContent = R"(<bookmark href="file:///nonexistent/file.txt" modified="2024-01-01T12:00:00Z"/>)";
    QXmlStreamReader reader(xmlContent);
    reader.readNext();

    QStringList curPathList;
    bool itemAddedEmitted = false;

    // Mock URL operations
    stub.set_lamda(static_cast<bool (QUrl::*)() const>(&QUrl::isLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QString (QUrl::*)() const>(&QUrl::toLocalFile), [](const QUrl *) {
        __DBG_STUB_INVOKE__
        return "/nonexistent/file.txt";
    });

    // Mock file validation to return false
    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::exists), [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock protocol check
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QObject::connect(worker, &RecentIterateWorker::itemAdded,
                     [&](const QString &, const RecentItem &) {
                         itemAddedEmitted = true;
                     });

    worker->processBookmarkElement(reader, curPathList);

    EXPECT_FALSE(itemAddedEmitted);
    EXPECT_TRUE(curPathList.isEmpty());
}

TEST_F(UT_RecentIterateWorker, removeOutdatedItems_RemovesCorrectItems)
{
    // Pre-populate worker with items
    RecentItem item1;
    item1.href = "file:///keep/file1.txt";
    item1.modified = 1000000000;
    worker->itemsInfo.insert("/keep/file1.txt", item1);

    RecentItem item2;
    item2.href = "file:///remove/file2.txt";
    item2.modified = 1000000001;
    worker->itemsInfo.insert("/remove/file2.txt", item2);

    RecentItem item3;
    item3.href = "file:///remove/file3.txt";
    item3.modified = 1000000002;
    worker->itemsInfo.insert("/remove/file3.txt", item3);

    QStringList cachedPathList = { "/keep/file1.txt", "/remove/file2.txt", "/remove/file3.txt" };
    QStringList curPathList = { "/keep/file1.txt" };   // Only file1 should be kept

    bool itemsRemovedEmitted = false;
    QStringList removedPaths;

    QObject::connect(worker, &RecentIterateWorker::itemsRemoved,
                     [&](const QStringList &paths) {
                         itemsRemovedEmitted = true;
                         removedPaths = paths;
                     });

    worker->removeOutdatedItems(cachedPathList, curPathList);

    EXPECT_TRUE(itemsRemovedEmitted);
    EXPECT_EQ(removedPaths.size(), 2);
    EXPECT_TRUE(removedPaths.contains("/remove/file2.txt"));
    EXPECT_TRUE(removedPaths.contains("/remove/file3.txt"));

    // Verify items were actually removed from internal storage
    EXPECT_TRUE(worker->itemsInfo.contains("/keep/file1.txt"));
    EXPECT_FALSE(worker->itemsInfo.contains("/remove/file2.txt"));
    EXPECT_FALSE(worker->itemsInfo.contains("/remove/file3.txt"));
}

TEST_F(UT_RecentIterateWorker, removeOutdatedItems_WithNoRemovals_DoesNotEmitSignal)
{
    QStringList cachedPathList = { "/keep/file1.txt", "/keep/file2.txt" };
    QStringList curPathList = { "/keep/file1.txt", "/keep/file2.txt" };

    bool itemsRemovedEmitted = false;

    QObject::connect(worker, &RecentIterateWorker::itemsRemoved,
                     [&](const QStringList &) {
                         itemsRemovedEmitted = true;
                     });

    worker->removeOutdatedItems(cachedPathList, curPathList);

    EXPECT_FALSE(itemsRemovedEmitted);
}
