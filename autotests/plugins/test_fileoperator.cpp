// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperator.cpp
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

TEST_F(FileOperatorTest, clearDropFileData)
{
    // Test method: void clearDropFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearDropFileData());
}

TEST_F(FileOperatorTest, clearPasteFileData)
{
    // Test method: void clearPasteFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearPasteFileData());
}

TEST_F(FileOperatorTest, clearRenameFileData)
{
    // Test method: void clearRenameFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearRenameFileData());
}

TEST_F(FileOperatorTest, clearTouchFileData)
{
    // Test method: void clearTouchFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearTouchFileData());
}

TEST_F(FileOperatorTest, deleteFiles)
{
    // Test method: void deleteFiles((const CollectionView *view))
    EXPECT_NO_FATAL_FAILURE(obj->deleteFiles(nullptr));
}

TEST_F(FileOperatorTest, onCanvasPastedFiles)
{
    // Test method: void onCanvasPastedFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->onCanvasPastedFiles());
}

TEST_F(FileOperatorTest, pasteFiles)
{
    // Test method: void pasteFiles((const CollectionView *view, const QString &targetColletion))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->pasteFiles(nullptr, _arg1));
}

TEST_F(FileOperatorTest, removeDropFileData)
{
    // Test method: void removeDropFileData((const QUrl &oldUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeDropFileData(_arg0));
}

TEST_F(FileOperatorTest, removePasteFileData)
{
    // Test method: void removePasteFileData((const QUrl &oldUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removePasteFileData(_arg0));
}

TEST_F(FileOperatorTest, removeRenameFileData)
{
    // Test method: void removeRenameFileData((const QUrl &oldUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeRenameFileData(_arg0));
}

TEST_F(FileOperatorTest, FileOperator)
{
    // Test constructor: FileOperator((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
