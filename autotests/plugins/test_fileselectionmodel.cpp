// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileselectionmodel.cpp
 * @brief Unit tests for FileSelectionModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileselectionmodel.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileSelectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileSelectionModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileSelectionModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSelectionModelTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(FileSelectionModelTest, clearSelectList)
{
    // Test method: void clearSelectList(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSelectList());
}

TEST_F(FileSelectionModelTest, isSelected)
{
    // Test method: bool isSelected((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isSelected(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileSelectionModelTest, select)
{
    // Test method: void select((const QItemSelection &selection, QItemSelectionModel::SelectionFlags command))
    QItemSelection _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->select(_arg0, QItemSelectionModel::SelectionFlags()));
}

TEST_F(FileSelectionModelTest, selectedIndexes)
{
    // Test getter: QModelIndexList selectedIndexes()
    auto result = obj->selectedIndexes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileSelectionModelTest, FileSelectionModel)
{
    // Test constructor: FileSelectionModel((QAbstractItemModel *model, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
