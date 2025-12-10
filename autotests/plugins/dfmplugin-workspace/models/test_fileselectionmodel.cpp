// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "models/fileselectionmodel.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QModelIndex>
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
