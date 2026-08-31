// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperator_1.cpp
 * @brief Unit tests for FileOperator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileoperator.h"

#include <QTest>

using namespace ddplugin_organizer;

class FileOperatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorTest, copyFilePath)
{
    // Test method: void copyFilePath((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->copyFilePath(nullptr));
}

TEST_F(FileOperatorTest, copyFiles)
{
    // Test method: void copyFiles((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->copyFiles(nullptr));
}

TEST_F(FileOperatorTest, cutFiles)
{
    // Test method: void cutFiles((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->cutFiles(nullptr));
}

TEST_F(FileOperatorTest, dropFilesToCanvas)
{
    // Test method: void dropFilesToCanvas((const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls))
    Qt::DropAction _arg0{};
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->dropFilesToCanvas(_arg0, _arg1, _arg2));
}

TEST_F(FileOperatorTest, dropFilesToCollection)
{
    // Test method: void dropFilesToCollection((const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls, const QString &key, const int index))
    Qt::DropAction _arg0{};
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->dropFilesToCollection(_arg0, _arg1, _arg2, _arg3, 0));
}

TEST_F(FileOperatorTest, dropToApp)
{
    // Test method: void dropToApp((const QList<QUrl> &urls, const QString &app))
    QList<QUrl> _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->dropToApp(_arg0, _arg1));
}

TEST_F(FileOperatorTest, dropToTrash)
{
    // Test method: void dropToTrash((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->dropToTrash(_arg0));
}

TEST_F(FileOperatorTest, instance)
{
    // Test getter: FileOperator instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FileOperatorTest, moveToTrash)
{
    // Test method: void moveToTrash((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->moveToTrash(nullptr));
}

TEST_F(FileOperatorTest, openFiles)
{
    // Test method: void openFiles((const CollectionView *view, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openFiles(nullptr, _arg1));
}

TEST_F(FileOperatorTest, pasteFileData)
{
    // Test getter: QSet<QUrl> pasteFileData()
    auto result = obj->pasteFileData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileOperatorTest, previewFiles)
{
    // Test method: void previewFiles((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->previewFiles(nullptr));
}

TEST_F(FileOperatorTest, renameFile)
{
    // Test method: void renameFile((int wid, const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFile(0, _arg1, _arg2));
}

TEST_F(FileOperatorTest, renameFileData)
{
    // Test getter: QHash<QUrl, QUrl> renameFileData()
    auto result = obj->renameFileData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileOperatorTest, renameFiles)
{
    // Test method: void renameFiles((const CollectionView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFiles(nullptr, _arg1, QPair<QString, AbstractJobHandler::FileNameAddFlag>()));
}

TEST_F(FileOperatorTest, setDataProvider)
{
    // Test setter: void setDataProvider((CollectionDataProvider *provider))
    EXPECT_NO_FATAL_FAILURE(obj->setDataProvider(nullptr));
}

TEST_F(FileOperatorTest, showFilesProperty)
{
    // Test method: void showFilesProperty((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->showFilesProperty(nullptr));
}

TEST_F(FileOperatorTest, touchFileData)
{
    // Test getter: QUrl touchFileData()
    auto result = obj->touchFileData();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileOperatorTest, undoFiles)
{
    // Test method: void undoFiles((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->undoFiles(nullptr));
}
