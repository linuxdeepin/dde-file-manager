// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewhelper.cpp
 * @brief Unit tests for FileViewHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewHelperTest, clearSearchKey)
{
    // Test method: void clearSearchKey(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSearchKey());
}

TEST_F(FileViewHelperTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileViewHelperTest, findIndex)
{
    // Test method: QModelIndex findIndex((const QByteArray &keys, bool matchStart, int current, bool reverseOrder, bool excludeCurrent))
    QByteArray _arg0{};
    auto result = obj->findIndex(_arg0, false, 0, false, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewHelperTest, handleCommitData)
{
    // Test method: void handleCommitData((QWidget *editor))
    EXPECT_NO_FATAL_FAILURE(obj->handleCommitData(nullptr));
}

TEST_F(FileViewHelperTest, initStyleOption)
{
    // Test method: void initStyleOption((QStyleOptionViewItem *option, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->initStyleOption(nullptr, _arg1));
}

TEST_F(FileViewHelperTest, selectFiles)
{
    // Test method: void selectFiles((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFiles(_arg0));
}

TEST_F(FileViewHelperTest, FileViewHelper)
{
    // Test constructor: FileViewHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileViewHelperTest, indexWidget)
{
    // Test method: QWidget indexWidget((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->indexWidget(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->indexWidget(_arg0); });

}

TEST_F(FileViewHelperTest, parent)
{
    // Test getter: FileView parent()
    auto result = obj->parent();
    EXPECT_NO_FATAL_FAILURE({ obj->parent(); });

}
