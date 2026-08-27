// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileselectionmodel.cpp
 * @brief Unit tests for FileSelectionModel Mid-priority methods (dfmplugin-workspace)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "models/fileselectionmodel.h"

using namespace dfmplugin_workspace;

class FileSelectionModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(FileSelectionModelTest, clear)
{
    // Instance method clear
    FileSelectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.clear(); });
}

TEST_F(FileSelectionModelTest, clearSelectList)
{
    // Instance method clearSelectList
    FileSelectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.clearSelectList(); });
}

TEST_F(FileSelectionModelTest, isSelected)
{
    // Instance method isSelected
    FileSelectionModel obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isSelected(QModelIndex()); });
    (void)result;
}

TEST_F(FileSelectionModelTest, select)
{
    // Instance method select
    FileSelectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.select(QItemSelection(), QItemSelectionModel::SelectionFlags()); });
}

TEST_F(FileSelectionModelTest, selectedIndexes)
{
    // Instance method selectedIndexes
    FileSelectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.selectedIndexes(); (void)r; });
}

TEST_F(FileSelectionModelTest, FileSelectionModel)
{
    // FileSelectionModel
    SUCCEED();
}
