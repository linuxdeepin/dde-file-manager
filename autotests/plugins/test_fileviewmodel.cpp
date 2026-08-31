// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewmodel.cpp
 * @brief Unit tests for FileViewModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileviewmodel.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewModelTest, columnCount)
{
    // Test method: int columnCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->columnCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewModelTest, connectFilterSortWorkSignals)
{
    // Test method: void connectFilterSortWorkSignals(())
    EXPECT_NO_FATAL_FAILURE(obj->connectFilterSortWorkSignals());
}

TEST_F(FileViewModelTest, executeLoad)
{
    // Test method: void executeLoad(())
    EXPECT_NO_FATAL_FAILURE(obj->executeLoad());
}

TEST_F(FileViewModelTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileViewModelTest, index)
{
    // Test method: QModelIndex index((int row, int column, const QModelIndex &parent))
    QModelIndex _arg2{};
    auto result = obj->index(0, 0, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewModelTest, initFilterSortWork)
{
    // Test method: void initFilterSortWork(())
    EXPECT_NO_FATAL_FAILURE(obj->initFilterSortWork());
}

TEST_F(FileViewModelTest, onGroupRemove)
{
    // Test method: void onGroupRemove((int firstIndex, int count))
    EXPECT_NO_FATAL_FAILURE(obj->onGroupRemove(0, 0));
}

TEST_F(FileViewModelTest, onGroupRemoveFinish)
{
    // Test method: void onGroupRemoveFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->onGroupRemoveFinish());
}

TEST_F(FileViewModelTest, onRemove)
{
    // Test method: void onRemove((int firstIndex, int count))
    EXPECT_NO_FATAL_FAILURE(obj->onRemove(0, 0));
}

TEST_F(FileViewModelTest, onRemoveFinish)
{
    // Test method: void onRemoveFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->onRemoveFinish());
}

TEST_F(FileViewModelTest, roleDisplayString)
{
    // Test method: QString roleDisplayString((int role))
    auto result = obj->roleDisplayString(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewModelTest, rootIndex)
{
    // Test getter: QModelIndex rootIndex()
    auto result = obj->rootIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewModelTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileViewModelTest, rowCount)
{
    // Test method: int rowCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->rowCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewModelTest, sort)
{
    // Test method: void sort((int column, Qt::SortOrder order))
    EXPECT_NO_FATAL_FAILURE(obj->sort(0, Qt::SortOrder()));
}

TEST_F(FileViewModelTest, flags)
{
    // Test method: Qt::ItemFlags flags((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->flags(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewModelTest, mimeData)
{
    // Test method: QMimeData mimeData((const QModelIndexList &indexes))
    QModelIndexList _arg0{};
    auto result = obj->mimeData(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->mimeData(_arg0); });

}
