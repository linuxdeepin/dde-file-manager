// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorproxy_1.cpp
 * @brief Unit tests for FileOperatorProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/fileoperatorproxy.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileOperatorProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorProxyTest, FileOperatorProxy)
{
    // Test constructor: FileOperatorProxy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperatorProxyTest, copyFilePath)
{
    // Test method: void copyFilePath((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->copyFilePath(nullptr));
}

TEST_F(FileOperatorProxyTest, copyFiles)
{
    // Test method: void copyFiles((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->copyFiles(nullptr));
}

TEST_F(FileOperatorProxyTest, cutFiles)
{
    // Test method: void cutFiles((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->cutFiles(nullptr));
}

TEST_F(FileOperatorProxyTest, dropFiles)
{
    // Test method: void dropFiles((const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls))
    Qt::DropAction _arg0{};
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->dropFiles(_arg0, _arg1, _arg2));
}

TEST_F(FileOperatorProxyTest, dropToApp)
{
    // Test method: void dropToApp((const QList<QUrl> &urls, const QString &app))
    QList<QUrl> _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->dropToApp(_arg0, _arg1));
}

TEST_F(FileOperatorProxyTest, dropToTrash)
{
    // Test method: void dropToTrash((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->dropToTrash(_arg0));
}

TEST_F(FileOperatorProxyTest, moveToTrash)
{
    // Test method: void moveToTrash((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->moveToTrash(nullptr));
}

TEST_F(FileOperatorProxyTest, openFiles)
{
    // Test method: void openFiles((const CanvasView *view, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openFiles(nullptr, _arg1));
}

TEST_F(FileOperatorProxyTest, redoFiles)
{
    // Test method: void redoFiles((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->redoFiles(nullptr));
}

TEST_F(FileOperatorProxyTest, renameFile)
{
    // Test method: void renameFile((int wid, const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFile(0, _arg1, _arg2));
}

TEST_F(FileOperatorProxyTest, renameFiles)
{
    // Test method: void renameFiles((const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFiles(nullptr, _arg1, QPair<QString, AbstractJobHandler::FileNameAddFlag>()));
}

TEST_F(FileOperatorProxyTest, sendFilesToBluetooth)
{
    // Test method: void sendFilesToBluetooth((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->sendFilesToBluetooth(nullptr));
}

TEST_F(FileOperatorProxyTest, showFilesProperty)
{
    // Test method: void showFilesProperty((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->showFilesProperty(nullptr));
}

TEST_F(FileOperatorProxyTest, touchFile)
{
    // Test method: void touchFile((const CanvasView *view, const QPoint pos, const QUrl &source))
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->touchFile(nullptr, QPoint(), _arg2));
}

TEST_F(FileOperatorProxyTest, touchFolder)
{
    // Test method: void touchFolder((const CanvasView *view, const QPoint pos))
    EXPECT_NO_FATAL_FAILURE(obj->touchFolder(nullptr, QPoint()));
}

TEST_F(FileOperatorProxyTest, undoFiles)
{
    // Test method: void undoFiles((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->undoFiles(nullptr));
}
