// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "models/fileselectionmodel.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QTimer>

using namespace dfmplugin_workspace;

class FileSelectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        selectionModel = new FileSelectionModel(nullptr);
    }

    void TearDown() override
    {
        delete selectionModel;
        stub.clear();
    }

    FileSelectionModel *selectionModel = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSelectionModelTest, Constructor_WithNullModel_CreatesValidObject)
{
    // Test constructor with null model
    FileSelectionModel model(nullptr);
    
    EXPECT_EQ(model.model(), nullptr);
}

TEST_F(FileSelectionModelTest, Constructor_WithModelAndParent_CreatesValidObject)
{
    // Test constructor with model and parent
    QObject parent;
    FileSelectionModel model(nullptr, &parent);
    
    EXPECT_EQ(model.model(), nullptr);
    EXPECT_EQ(model.parent(), &parent);
}

TEST_F(FileSelectionModelTest, IsSelected_WithInvalidIndex_ReturnsFalse)
{
    // Test checking if invalid index is selected
    QModelIndex invalidIndex;
    
    EXPECT_FALSE(selectionModel->isSelected(invalidIndex));
}

TEST_F(FileSelectionModelTest, SelectedCount_WithNoSelection_ReturnsZero)
{
    // Test selected count with no selection
    EXPECT_EQ(selectionModel->selectedCount(), 0);
}

TEST_F(FileSelectionModelTest, SelectedIndexes_WithNoSelection_ReturnsEmptyList)
{
    // Test selected indexes with no selection
    QModelIndexList indexes = selectionModel->selectedIndexes();
    
    EXPECT_TRUE(indexes.isEmpty());
}

TEST_F(FileSelectionModelTest, ClearSelectList_DoesNotCrash)
{
    // Test clearing selected list
    selectionModel->clearSelectList();
    
    // Should not crash
    EXPECT_NO_THROW(selectionModel->clearSelectList());
}

TEST_F(FileSelectionModelTest, Clear_ClearsAllSelections)
{
    // Test clear method
    selectionModel->clear();
    
    // Should have no selections after clear
    EXPECT_EQ(selectionModel->selectedCount(), 0);
}

TEST_F(FileSelectionModelTest, Destructor_DoesNotCrash)
{
    // Test destructor
    auto *model = new FileSelectionModel(nullptr);
    
    // Should not crash
    EXPECT_NO_THROW(delete model);
}

// ===== PMS sev-2 regression tests (appended) =====
// PMS:326753 延迟 select 需维护 first/lastSelectedIndex，selectedCount 按区间计数
TEST_F(FileSelectionModelTest, BUG326753_Select_ClearAndSelect_TracksRange)
{
    QStandardItemModel model;
    model.appendRow(new QStandardItem("a"));
    model.appendRow(new QStandardItem("b"));
    model.appendRow(new QStandardItem("c"));

    QItemSelection selection;
    selection.select(model.index(0, 0), model.index(2, 0));

    const auto flags = QItemSelectionModel::Current | QItemSelectionModel::Rows
            | QItemSelectionModel::ClearAndSelect;
    EXPECT_NO_THROW(selectionModel->select(selection, flags));
    EXPECT_EQ(selectionModel->selectedCount(), 3);

    // 空选区 + 相同命令：first/last 归位，不崩溃
    QItemSelection empty;
    EXPECT_NO_THROW(selectionModel->select(empty, flags));
    EXPECT_EQ(selectionModel->selectedCount(), 0);

    // 非 ClearAndSelect 命令：不维护区间，不崩溃
    QItemSelection sel2;
    sel2.select(model.index(1, 0), model.index(1, 0));
    EXPECT_NO_THROW(selectionModel->select(sel2, QItemSelectionModel::Select));
}
