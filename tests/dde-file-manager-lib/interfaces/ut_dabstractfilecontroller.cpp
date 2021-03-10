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

#include <gtest/gtest.h>

#include "dabstractfilecontroller.h"
#include "dfmevent.h"
#include "stub.h"

#include <DDesktopServices>
#include <QProcess>

DWIDGET_USE_NAMESPACE

namespace  {
class TestDAbstractFileController : public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestDAbstractFileController" << std::endl;
        controller = new DAbstractFileController();
        QProcess::execute("touch /tmp/1.txt");
        fileUrl = DUrl("file:///tmp/1.txt");
    }

    virtual void TearDown() override
    {
        std::cout << "end TestDAbstractFileController" << std::endl;
        if (controller) {
            delete controller;
            controller = nullptr;
        }
        QProcess::execute("rm -f /tmp/1.txt");
    }
public:
    DAbstractFileController *controller;
    DUrl fileUrl;
};
}

TEST_F(TestDAbstractFileController, openFile)
{
    auto res = controller->openFile(dMakeEventPointer<DFMOpenFileEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, openFiles)
{
    auto res = controller->openFiles(dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, openFileByApp)
{
    auto res = controller->openFileByApp(dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "", fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, openFilesByApp)
{
    auto res = controller->openFilesByApp(dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "", DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, compressFiles)
{
    auto res = controller->compressFiles(dMakeEventPointer<DFMCompressEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, decompressFile)
{
    auto res = controller->decompressFile(dMakeEventPointer<DFMDecompressEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, decompressFileHere)
{
    auto res = controller->decompressFileHere(dMakeEventPointer<DFMDecompressEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, writeFilesToClipboard)
{
    auto res = controller->writeFilesToClipboard(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, renameFile)
{
    auto res = controller->renameFile(dMakeEventPointer<DFMRenameEvent>(nullptr, fileUrl, DUrl("file:///2.txt")));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, deleteFiles)
{
    auto res = controller->deleteFiles(dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, moveToTrash)
{
    auto res = controller->moveToTrash(dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_TRUE(res.isEmpty());
}

TEST_F(TestDAbstractFileController, pasteFile)
{
    auto res = controller->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction, DUrl("file:///tmp"), DUrlList() << fileUrl));
    EXPECT_TRUE(res.isEmpty());
}

TEST_F(TestDAbstractFileController, restoreFile)
{
    auto res = controller->restoreFile(dMakeEventPointer<DFMRestoreFromTrashEvent>(nullptr, DUrlList() << fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, mkdir)
{
    auto res = controller->mkdir(dMakeEventPointer<DFMMkdirEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, touch)
{
    auto res = controller->touch(dMakeEventPointer<DFMTouchFileEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, setPermissions)
{
    auto res = controller->setPermissions(dMakeEventPointer<DFMSetPermissionEvent>(nullptr, fileUrl, QFileDevice::ReadUser));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, openFileLocation)
{
    // not root user
    bool (*stub_showFileItem)(QUrl, const QString &) = [](QUrl, const QString &) {
        return true;
    };

    Stub stub;
    auto addr = (bool(*)(QUrl, const QString &))ADDR(DDesktopServices, showFileItem);
    stub.set(addr, stub_showFileItem);
    auto res = controller->openFileLocation(dMakeEventPointer<DFMOpenFileLocation>(nullptr, fileUrl));
    EXPECT_TRUE(res);

    // root user
    bool (*stub_isRootUser)() = []() {
        return true;
    };
    stub.set(ADDR(DFMGlobal, isRootUser), stub_isRootUser);

    bool (*stub_startDetached)(const QString &, const QStringList &) = [](const QString &, const QStringList &) {
        return true;
    };
    stub.set((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), stub_startDetached);
    res = controller->openFileLocation(dMakeEventPointer<DFMOpenFileLocation>(nullptr, fileUrl));
    EXPECT_TRUE(res);
}

TEST_F(TestDAbstractFileController, getChildren)
{
    auto flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    auto res = controller->getChildren(dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("file:///home"), QStringList(), QDir::AllEntries, flags));
    EXPECT_TRUE(res.isEmpty());
}

TEST_F(TestDAbstractFileController, createFileInfo)
{
    auto res = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, createDirIterator)
{
    auto flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    auto res = controller->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, DUrl::fromTrashFile("/"), QStringList(), QDir::AllEntries, flags));
    EXPECT_TRUE(res != nullptr);
}

TEST_F(TestDAbstractFileController, addToBookmark)
{
    auto res = controller->addToBookmark(dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, removeBookmark)
{
    auto res = controller->removeBookmark(dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, createSymlink)
{
    auto res = controller->createSymlink(dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, fileUrl, DUrl("file:///tmp/2.txt")));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, shareFolder)
{
    auto res = controller->shareFolder(dMakeEventPointer<DFMFileShareEvent>(nullptr, fileUrl, "share"));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, unShareFolder)
{
    auto res = controller->unShareFolder(dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, openInTerminal)
{
    auto res = controller->openInTerminal(dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, fileUrl));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, setFileTags)
{
    auto res = controller->setFileTags(dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, fileUrl, QList<QString>() << "red"));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, removeTagsOfFile)
{
    auto res = controller->removeTagsOfFile(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, fileUrl, QList<QString>() << "red"));
    EXPECT_FALSE(res);
}

TEST_F(TestDAbstractFileController, getTagsThroughFiles)
{
    auto res = controller->getTagsThroughFiles(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>() << fileUrl));
    EXPECT_TRUE(!res.isEmpty());
}

TEST_F(TestDAbstractFileController, createFileWatcher)
{
    auto res = controller->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, fileUrl));
    EXPECT_TRUE(res == nullptr);
}

TEST_F(TestDAbstractFileController, createFileDevice)
{
    auto res = controller->createFileDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, fileUrl));
    EXPECT_TRUE(res == nullptr);
}

TEST_F(TestDAbstractFileController, createFileHandler)
{
    auto res = controller->createFileHandler(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, fileUrl));
    EXPECT_TRUE(res == nullptr);
}

TEST_F(TestDAbstractFileController, createStorageInfo)
{
    auto res = controller->createStorageInfo(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, fileUrl));
    EXPECT_TRUE(res == nullptr);
}

TEST_F(TestDAbstractFileController, setExtraProperties)
{
    QVariantHash ep{{"tag_name_list", QStringList() << "red"}};
    auto res = controller->setExtraProperties(dMakeEventPointer<DFMSetFileExtraProperties>(nullptr, fileUrl, ep));
    EXPECT_FALSE(res);
}
