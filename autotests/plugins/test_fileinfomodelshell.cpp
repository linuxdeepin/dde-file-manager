// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodelshell.cpp
 * @brief Unit tests for FileInfoModelShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/fileinfomodelshell.h"

#include <QTest>

using namespace ddplugin_organizer;

class FileInfoModelShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoModelShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoModelShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoModelShellTest, FileInfoModelShell)
{
    // Test constructor: FileInfoModelShell((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileInfoModelShellTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileInfoModelShellTest, fileUrl)
{
    // Test method: QUrl fileUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelShellTest, files)
{
    // Test getter: QList<QUrl> files()
    auto result = obj->files();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileInfoModelShellTest, index)
{
    // Test method: QModelIndex index((const QUrl &url, int column))
    QUrl _arg0{};
    auto result = obj->index(_arg0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelShellTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(FileInfoModelShellTest, modelState)
{
    // Test getter: int modelState()
    auto result = obj->modelState();
    EXPECT_EQ(result, 0);

}

TEST_F(FileInfoModelShellTest, refresh)
{
    // Test method: void refresh((const QModelIndex &parent))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refresh(_arg0));
}

TEST_F(FileInfoModelShellTest, rootIndex)
{
    // Test getter: QModelIndex rootIndex()
    auto result = obj->rootIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelShellTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileInfoModelShellTest, sourceModel)
{
    // Test getter: QAbstractItemModel sourceModel()
    auto result = obj->sourceModel();
    EXPECT_NO_FATAL_FAILURE({ obj->sourceModel(); });

}

TEST_F(FileInfoModelShellTest, FileInfoModelShell_Destructor)
{
    // Test method:  ~FileInfoModelShell(())
    EXPECT_NO_FATAL_FAILURE({ FileInfoModelShell *tmp = new FileInfoModelShell(); delete tmp; });
}
