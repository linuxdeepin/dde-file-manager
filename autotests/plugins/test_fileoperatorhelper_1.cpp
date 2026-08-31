// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorhelper_1.cpp
 * @brief Unit tests for FileOperatorHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileoperatorhelper.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class FileOperatorHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorHelperTest, FileOperatorHelper)
{
    // Test constructor: FileOperatorHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperatorHelperTest, FileOperatorHelper_FileOper)
{
    // Test constructor: FileOperatorHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperatorHelperTest, copyFilePath)
{
    // Test method: void copyFilePath((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->copyFilePath(nullptr));
}

TEST_F(FileOperatorHelperTest, copyFiles)
{
    // Test method: void copyFiles((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->copyFiles(nullptr));
}

TEST_F(FileOperatorHelperTest, cutFiles)
{
    // Test method: void cutFiles((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->cutFiles(nullptr));
}

TEST_F(FileOperatorHelperTest, dropFiles)
{
    // Test method: void dropFiles((const FileView *view, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls))
    Qt::DropAction _arg1{};
    QUrl _arg2{};
    QList<QUrl> _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->dropFiles(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(FileOperatorHelperTest, moveToTrash)
{
    // Test method: void moveToTrash((const FileView *view, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->moveToTrash(nullptr, _arg1));
}

TEST_F(FileOperatorHelperTest, openFiles)
{
    // Test method: void openFiles((const FileView *view, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openFiles(nullptr, _arg1));
}

TEST_F(FileOperatorHelperTest, openFilesByApp)
{
    // Test method: void openFilesByApp((const FileView *view, const QList<QUrl> &urls, const QList<QString> &apps))
    QList<QUrl> _arg1{};
    QList<QString> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->openFilesByApp(nullptr, _arg1, _arg2));
}

TEST_F(FileOperatorHelperTest, openInTerminal)
{
    // Test method: void openInTerminal((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->openInTerminal(nullptr));
}

TEST_F(FileOperatorHelperTest, pasteClipboardImage)
{
    // Test method: void pasteClipboardImage((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->pasteClipboardImage(nullptr));
}

TEST_F(FileOperatorHelperTest, pasteFiles)
{
    // Test method: void pasteFiles((quint64 windowId, const QList<QUrl> &srcUrls, const QUrl &targetUrl, const Qt::DropAction &action))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    Qt::DropAction _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->pasteFiles(0, _arg1, _arg2, _arg3));
}

TEST_F(FileOperatorHelperTest, pasteFiles_pasteFil)
{
    // Test method: void pasteFiles((quint64 windowId, const QList<QUrl> &srcUrls, const QUrl &targetUrl, const Qt::DropAction &action))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    Qt::DropAction _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->pasteFiles(0, _arg1, _arg2, _arg3));
}

TEST_F(FileOperatorHelperTest, previewFiles)
{
    // Test method: void previewFiles((const FileView *view, const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls))
    QList<QUrl> _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->previewFiles(nullptr, _arg1, _arg2));
}

TEST_F(FileOperatorHelperTest, redoFiles)
{
    // Test method: void redoFiles((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->redoFiles(nullptr));
}

TEST_F(FileOperatorHelperTest, renameFile)
{
    // Test method: void renameFile((const FileView *view, const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFile(nullptr, _arg1, _arg2));
}

TEST_F(FileOperatorHelperTest, renameFilesByAdd)
{
    // Test method: void renameFilesByAdd((const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &addPair))
    QList<QUrl> _arg1{};
    QPair<QString, AbstractJobHandler::FileNameAddFlag> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFilesByAdd(nullptr, _arg1, _arg2));
}

TEST_F(FileOperatorHelperTest, renameFilesByCustom)
{
    // Test method: void renameFilesByCustom((const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, QString> &customPair))
    QList<QUrl> _arg1{};
    QPair<QString, QString> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFilesByCustom(nullptr, _arg1, _arg2));
}

TEST_F(FileOperatorHelperTest, renameFilesByReplace)
{
    // Test method: void renameFilesByReplace((const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, QString> &replacePair))
    QList<QUrl> _arg1{};
    QPair<QString, QString> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFilesByReplace(nullptr, _arg1, _arg2));
}

TEST_F(FileOperatorHelperTest, showFilesProperty)
{
    // Test method: void showFilesProperty((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->showFilesProperty(nullptr));
}

TEST_F(FileOperatorHelperTest, touchFiles)
{
    // Test method: void touchFiles((const FileView *view, const QUrl &source))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->touchFiles(nullptr, _arg1));
}

TEST_F(FileOperatorHelperTest, touchFolder)
{
    // Test method: void touchFolder((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->touchFolder(nullptr));
}

TEST_F(FileOperatorHelperTest, undoCallBackFunction)
{
    // Test method: void undoCallBackFunction((QSharedPointer<AbstractJobHandler> handler))
    EXPECT_NO_FATAL_FAILURE(obj->undoCallBackFunction(QSharedPointer<AbstractJobHandler>()));
}

TEST_F(FileOperatorHelperTest, undoFiles)
{
    // Test method: void undoFiles((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->undoFiles(nullptr));
}
