/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dfmevent.h"
#include "stub.h"
#include "dfileservices.h"

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "controllers/searchcontroller.cpp"
#undef private

namespace  {
class TestSearchController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestSearchController" << std::endl;
        controller = new SearchController;
        filePath_1 =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/1.txt";
        filePath_2 =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/2.txt";
        dirPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/hello";
        QProcess::execute("touch " + filePath_1 + " " + filePath_2);
        QProcess::execute("mkdir " + dirPath);
        fileUrl_1.setScheme(SEARCH_SCHEME);
        fileUrl_1.setQuery("url=file:///tmp&keyword=hello");
        fileUrl_1.setSearchedFileUrl(DUrl::fromLocalFile(filePath_1));

        fileUrl_2.setScheme(SEARCH_SCHEME);
        fileUrl_2.setQuery("url=file:///tmp&keyword=hello");
        fileUrl_2.setSearchedFileUrl(DUrl::fromLocalFile(filePath_2));

        dirUrl.setScheme(SEARCH_SCHEME);
        dirUrl.setQuery("url=file:///tmp&keyword=hello");
        dirUrl.setSearchedFileUrl(DUrl::fromLocalFile(dirPath));
    }

    void TearDown() override
    {
        std::cout << "end TestSearchController" << std::endl;
        QProcess::execute("rm -rf " + filePath_1 + " " + filePath_2 + " " + dirPath);
        delete controller;
        controller = nullptr;
    }

public:
    SearchController *controller;
    QString filePath_1;
    QString filePath_2;
    QString dirPath;
    DUrl fileUrl_1;
    DUrl fileUrl_2;
    DUrl dirUrl;
};
}

TEST_F(TestSearchController, createFileInfo)
{
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl_1);
    EXPECT_TRUE(controller->createFileInfo(event) != nullptr);
}

TEST_F(TestSearchController, openFileLocation)
{
    auto event = dMakeEventPointer<DFMOpenFileLocation>(nullptr, fileUrl_1);

    bool (*stub_openFileLocation)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, openFileLocation), stub_openFileLocation);

    EXPECT_TRUE(controller->openFileLocation(event));
}

TEST_F(TestSearchController, openFile)
{
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, fileUrl_1);
    bool (*stub_openFile)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, openFile), stub_openFile);

    EXPECT_TRUE(controller->openFile(event));
}

TEST_F(TestSearchController, openFileByApp)
{
    auto event = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", fileUrl_1);
    bool (*stub_openFileByApp)(const QObject *, const QString &, const DUrl &) = [](const QObject *, const QString &, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, openFileByApp), stub_openFileByApp);

    EXPECT_TRUE(controller->openFileByApp(event));
}

TEST_F(TestSearchController, openFilesByApp)
{
    auto event = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "/usr/share/applications/deepin-editor.desktop", DUrlList() << fileUrl_1 << fileUrl_2);
    bool (*stub_openFilesByApp)(const QObject *, const QString &, const QList<DUrl> &, const bool)
    = [](const QObject *, const QString &, const QList<DUrl> &, const bool) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, openFilesByApp), stub_openFilesByApp);

    EXPECT_TRUE(controller->openFilesByApp(event));
}

TEST_F(TestSearchController, writeFilesToClipboard)
{
    auto event = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << fileUrl_1 << fileUrl_2);
    bool (*stub_writeFilesToClipboard)(const QObject *, DFMGlobal::ClipboardAction, const DUrlList &)
    = [](const QObject *, DFMGlobal::ClipboardAction, const DUrlList &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, writeFilesToClipboard), stub_writeFilesToClipboard);

    EXPECT_TRUE(controller->writeFilesToClipboard(event));
}

TEST_F(TestSearchController, moveToTrash)
{
    auto event = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << fileUrl_1);
    DUrlList(*stub_moveToTrash)(const QObject *, const DUrlList &) = [](const QObject *, const DUrlList &) {
        return DUrlList();
    };
    Stub stub;
    stub.set(ADDR(DFileService, moveToTrash), stub_moveToTrash);

    DUrlList resList = controller->moveToTrash(event);
    EXPECT_TRUE(resList.isEmpty());
}

TEST_F(TestSearchController, deleteFiles)
{
    auto event = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << fileUrl_1);
    bool (*stub_deleteFiles)(const QObject *, const DUrlList &, bool, bool, bool)
    = [](const QObject *, const DUrlList &, bool, bool, bool) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, deleteFiles), stub_deleteFiles);

    EXPECT_TRUE(controller->deleteFiles(event));
}

TEST_F(TestSearchController, renameFile)
{
    QString temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
    DUrl to;
    to.setScheme(SEARCH_SCHEME);
    to.setSearchedFileUrl(DUrl::fromLocalFile(temp));
    auto event = dMakeEventPointer<DFMRenameEvent>(nullptr, fileUrl_1, to);
    bool (*stub_renameFile)(const QObject *, const DUrl &, const DUrl &, const bool)
    = [](const QObject *, const DUrl &, const DUrl &, const bool) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, renameFile), stub_renameFile);

    EXPECT_TRUE(controller->renameFile(event));
}

TEST_F(TestSearchController, setPermissions)
{
    auto event = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, fileUrl_1, (QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ReadOwner));
    bool (*stub_setPermissions)(const QObject *, const DUrl &, const QFileDevice::Permissions)
    = [](const QObject *, const DUrl &, const QFileDevice::Permissions) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, setPermissions), stub_setPermissions);

    EXPECT_TRUE(controller->setPermissions(event));
}

TEST_F(TestSearchController, addToBookmark)
{
    auto event = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, dirUrl);
    bool (*stub_addToBookmark)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, addToBookmark), stub_addToBookmark);

    EXPECT_TRUE(controller->addToBookmark(event));
}

TEST_F(TestSearchController, removeBookmark)
{
    auto event = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, dirUrl);
    bool (*stub_removeBookmark)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, removeBookmark), stub_removeBookmark);

    EXPECT_TRUE(controller->removeBookmark(event));
}

TEST_F(TestSearchController, createSymlink)
{
    QString symlinkPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/3.txt";
    auto event = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, fileUrl_1, DUrl::fromLocalFile(symlinkPath));
    bool (*stub_createSymlink)(const QObject *, const DUrl &, const DUrl &, bool)
    = [](const QObject *, const DUrl &, const DUrl &, bool) {
        return true;
    };
    Stub stub;
    stub.set((bool(DFileService::*)(const QObject *, const DUrl &, const DUrl &, bool) const)ADDR(DFileService, createSymlink), stub_createSymlink);

    EXPECT_TRUE(controller->createSymlink(event));
}

TEST_F(TestSearchController, shareFolder)
{
    auto event = dMakeEventPointer<DFMFileShareEvent>(nullptr, dirUrl, "hello");
    bool (*stub_shareFolder)(const QObject *, const DUrl &, const QString &, bool, bool)
    = [](const QObject *, const DUrl &, const QString &, bool, bool) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, shareFolder), stub_shareFolder);

    EXPECT_TRUE(controller->shareFolder(event));
}

TEST_F(TestSearchController, unShareFolder)
{
    auto unShareFolderEvent = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, dirUrl);
    bool (*stub_unShareFolder)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, unShareFolder), stub_unShareFolder);

    EXPECT_TRUE(controller->unShareFolder(unShareFolderEvent));
}

TEST_F(TestSearchController, openInTerminal)
{
    auto event = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, dirUrl);
    bool (*stub_openInTerminal)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, openInTerminal), stub_openInTerminal);

    EXPECT_TRUE(controller->openInTerminal(event));
}

TEST_F(TestSearchController, createDirIterator)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, dirUrl, QStringList(), QDir::AllEntries);
    // auto iter = controller->createDirIterator(event);
    // if (iter) {
    //     iter->hasNext();
    //     iter->next();
    //     iter->fileUrl();
    //     iter->fileInfo();
    //     iter->fileName();
    //     iter->url();
    //     iter->close();
    // }
    // EXPECT_TRUE(iter != nullptr);

    // 处理消息队列，否则其余位置直[间]接调用会导致事件处理顺序问题而 crash
    // qApp->processEvents();
}

TEST_F(TestSearchController, createFileWatcher)
{
    DUrl url(SEARCH_SCHEME);
    url.setQuery("url=file:///tmp&keyword=hello");
    auto event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url);
    SearchFileWatcher *watcher = static_cast<SearchFileWatcher *>(controller->createFileWatcher(event));
    if (watcher) {
        watcher->addWatcher(fileUrl_1);
        watcher->removeWatcher(fileUrl_1);
        watcher->onFileModified(fileUrl_1);
        watcher->onFileDeleted(fileUrl_1);
        watcher->onFileMoved(fileUrl_1, fileUrl_2);
        watcher->onFileAttributeChanged(fileUrl_1);
    }
    EXPECT_TRUE(watcher != nullptr);
    if (watcher) {
        delete watcher;
        watcher = nullptr;
    }
}

TEST_F(TestSearchController, setFileTags)
{
    auto event = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, fileUrl_1, QList<QString>({"红色"}));
    bool (*stub_setFileTags)(const QObject *, const DUrl &, const QList<QString> &) = [](const QObject *, const DUrl &, const QList<QString> &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, setFileTags), stub_setFileTags);

    EXPECT_TRUE(controller->setFileTags(event));
}

TEST_F(TestSearchController, removeTagsOfFile)
{
    auto event = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, fileUrl_1, QList<QString>({"红色"}));
    bool (*stub_removeTagsOfFile)(const QObject *, const DUrl &, const QList<QString> &) = [](const QObject *, const DUrl &, const QList<QString> &) {
        return true;
    };
    Stub stub;
    stub.set(ADDR(DFileService, removeTagsOfFile), stub_removeTagsOfFile);

    EXPECT_TRUE(controller->removeTagsOfFile(event));
}

TEST_F(TestSearchController, getTagsThroughFiles)
{
    auto event = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>({fileUrl_1}));
    QList<QString> (*stub_getTagsThroughFiles)(const QObject *, const QList<DUrl> &, const bool) = [](const QObject *, const QList<DUrl> &, const bool) {
        return QList<QString>();
    };
    Stub stub;
    stub.set(ADDR(DFileService, getTagsThroughFiles), stub_getTagsThroughFiles);

    EXPECT_TRUE(controller->getTagsThroughFiles(event).isEmpty());
}
