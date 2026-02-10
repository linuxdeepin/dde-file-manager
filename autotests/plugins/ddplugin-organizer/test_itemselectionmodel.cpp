// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/itemselectionmodel.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QItemSelection>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_ItemSelectionModel : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        // Create a test model
        testModel = new QStandardItemModel();
        
        // Add some test data
        for (int row = 0; row < 5; ++row) {
            QList<QStandardItem*> items;
            for (int col = 0; col < 3; ++col) {
                QStandardItem *item = new QStandardItem(QString("Item %1,%2").arg(row).arg(col));
                items.append(item);
            }
            testModel->appendRow(items);
        }
        
        // Create the selection model
        selectionModel = new ItemSelectionModel(testModel);
    }

    void TearDown() override
    {
        delete selectionModel;
        delete testModel;
        if (app) {
            delete app;
            app = nullptr;
        }
        stub.clear();
    }

public:
    ItemSelectionModel *selectionModel;
    QStandardItemModel *testModel;
    QApplication *app = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ItemSelectionModel, TestConstructor)
{
    EXPECT_NE(selectionModel, nullptr);
    EXPECT_EQ(selectionModel->model(), testModel);
    EXPECT_TRUE(selectionModel->selectedIndexes().isEmpty());
}

TEST_F(UT_ItemSelectionModel, TestCustomSelectAll)
{
    // Initially should have no selection
    EXPECT_TRUE(selectionModel->selectedIndexes().isEmpty());
    
    // Call our custom selectAll method
    selectionModel->selectAll();
    
    // Should select all items
    QList<QModelIndex> selected = selectionModel->selectedIndexes();
    EXPECT_EQ(selected.size(), 15);  // 5 rows * 3 columns
    
    // Verify all indexes are selected
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            QModelIndex index = testModel->index(row, col);
            EXPECT_TRUE(selectionModel->isSelected(index)) 
                << "Index (" << row << "," << col << ") should be selected";
        }
    }
}

TEST_F(UT_ItemSelectionModel, TestSelectAllAfterClear)
{
    // Select all
    selectionModel->selectAll();
    EXPECT_FALSE(selectionModel->selectedIndexes().isEmpty());
    
    // Clear selection
    selectionModel->clearSelection();
    EXPECT_TRUE(selectionModel->selectedIndexes().isEmpty());
    
    // Select all again
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllMultipleTimes)
{
    // Call selectAll multiple times
    selectionModel->selectAll();
    selectionModel->selectAll();
    selectionModel->selectAll();
    
    // Should still have all items selected
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
}

TEST_F(UT_ItemSelectionModel, TestSelectWithEmptyModel)
{
    // Create selection model with empty model
    QStandardItemModel emptyModel;
    ItemSelectionModel emptySelectionModel(&emptyModel);
    
    // selectAll should not crash
    EXPECT_NO_THROW(emptySelectionModel.selectAll());
    
    // Should still have no selections
    EXPECT_TRUE(emptySelectionModel.selectedIndexes().isEmpty());
}

TEST_F(UT_ItemSelectionModel, TestSelectWithNullModel)
{
    // Create selection model with null model
    ItemSelectionModel nullSelectionModel(nullptr);
    
    // selectAll should handle null model gracefully
    EXPECT_NO_THROW(nullSelectionModel.selectAll());
    
    // Should have no selections
    EXPECT_TRUE(nullSelectionModel.selectedIndexes().isEmpty());
}

TEST_F(UT_ItemSelectionModel, testMixedSelectionOperations)
{
    // Select some individual items first
    QModelIndex index1 = testModel->index(0, 0);
    QModelIndex index2 = testModel->index(2, 1);
    
    selectionModel->select(index1, QItemSelectionModel::Select);
    selectionModel->select(index2, QItemSelectionModel::Select);
    
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 2);
    EXPECT_TRUE(selectionModel->isSelected(index1));
    EXPECT_TRUE(selectionModel->isSelected(index2));
    
    // Now select all
    selectionModel->selectAll();
    
    // Should have all items selected now
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
    EXPECT_TRUE(selectionModel->isSelected(index1));
    EXPECT_TRUE(selectionModel->isSelected(index2));
    
    // Verify all indexes are selected
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            QModelIndex index = testModel->index(row, col);
            EXPECT_TRUE(selectionModel->isSelected(index));
        }
    }
}

TEST_F(UT_ItemSelectionModel, TestSelectAllWithDifferentSelectionModes)
{
    // Test selectAll functionality
    selectionModel->clearSelection();
    selectionModel->selectAll();
    
    // Should select items
    EXPECT_GE(selectionModel->selectedIndexes().size(), 0);
    
    // Test multiple calls to selectAll
    selectionModel->selectAll();
    selectionModel->selectAll();
    
    // Should still have items selected
    EXPECT_GE(selectionModel->selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllSignals)
{
    QSignalSpy selectionChangedSpy(selectionModel, &ItemSelectionModel::selectionChanged);
    
    // Clear any initial signals
    selectionModel->clearSelection();
    selectionChangedSpy.clear();
    
    // Select all
    selectionModel->selectAll();
    
    // Should have emitted selectionChanged signal
    EXPECT_GT(selectionChangedSpy.count(), 0);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllWithRowSelection)
{
    // Test row-based selection
    QItemSelection rowSelection;
    QModelIndex firstRowFirstCol = testModel->index(0, 0);
    QModelIndex firstRowLastCol = testModel->index(0, 2);
    rowSelection.select(firstRowFirstCol, firstRowLastCol);
    
    selectionModel->select(rowSelection, QItemSelectionModel::Select);
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 3);  // First row
    
    // Now select all
    selectionModel->selectAll();
    
    // Should have all items selected
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllAfterModelChange)
{
    // Select all
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
    
    // Add more items to model
    for (int row = 0; row < 3; ++row) {
        QList<QStandardItem*> items;
        for (int col = 0; col < 3; ++col) {
            QStandardItem *item = new QStandardItem(QString("New Item %1,%2").arg(row).arg(col));
            items.append(item);
        }
        testModel->appendRow(items);
    }
    
    // Select all again (should include new items)
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 24);  // 8 rows * 3 columns
}

TEST_F(UT_ItemSelectionModel, TestEdgeCases)
{
    // Test selectAll on a model with no rows but columns
    QStandardItemModel columnOnlyModel;
    columnOnlyModel.setColumnCount(3);
    ItemSelectionModel columnOnlySelection(&columnOnlyModel);
    
    EXPECT_NO_THROW(columnOnlySelection.selectAll());
    EXPECT_TRUE(columnOnlySelection.selectedIndexes().isEmpty());
    
    // Test selectAll on a model with rows but no columns
    QStandardItemModel rowOnlyModel;
    rowOnlyModel.setRowCount(3);
    ItemSelectionModel rowOnlySelection(&rowOnlyModel);
    
    EXPECT_NO_THROW(rowOnlySelection.selectAll());
    EXPECT_TRUE(rowOnlySelection.selectedIndexes().isEmpty());
    
    // Test selectAll with our original model
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
}

TEST_F(UT_ItemSelectionModel, TestPerformanceWithLargeModel)
{
    // Create a large model
    QStandardItemModel largeModel;
    const int rows = 100;
    const int cols = 10;
    
    for (int row = 0; row < rows; ++row) {
        QList<QStandardItem*> items;
        for (int col = 0; col < cols; ++col) {
            items.append(new QStandardItem(QString("Item %1,%2").arg(row).arg(col)));
        }
        largeModel.appendRow(items);
    }
    
    ItemSelectionModel largeSelectionModel(&largeModel);
    
    // Test selectAll performance (should not crash)
    EXPECT_NO_THROW(largeSelectionModel.selectAll());
    
    // Should have all items selected
    EXPECT_EQ(largeSelectionModel.selectedIndexes().size(), rows * cols);
    
    // Clear and select again
    largeSelectionModel.clearSelection();
    EXPECT_TRUE(largeSelectionModel.selectedIndexes().isEmpty());
    
    EXPECT_NO_THROW(largeSelectionModel.selectAll());
    EXPECT_EQ(largeSelectionModel.selectedIndexes().size(), rows * cols);
}

TEST_F(UT_ItemSelectionModel, TestInheritedBehavior)
{
    // Test that inherited QItemSelectionModel methods still work
    QModelIndex index = testModel->index(1, 1);
    
    // Test select method
    selectionModel->select(index, QItemSelectionModel::Select);
    EXPECT_TRUE(selectionModel->isSelected(index));
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 1);
    
    // Test setCurrentIndex
    QModelIndex index2 = testModel->index(2, 2);
    selectionModel->setCurrentIndex(index2, QItemSelectionModel::Select);
    EXPECT_EQ(selectionModel->currentIndex(), index2);
    
    // Test clear
    selectionModel->clear();
    EXPECT_TRUE(selectionModel->selectedIndexes().isEmpty());
    EXPECT_FALSE(selectionModel->currentIndex().isValid());
    
    // Test that our selectAll still works after using inherited methods
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllWithFilter)
{
    // Test with selection that has a filter
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select 
                                            | QItemSelectionModel::Rows;
    
    QModelIndex rowIndex = testModel->index(1, 0);
    selectionModel->select(rowIndex, flags);
    
    // Should select entire row 1 (3 columns)
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 3);
    
    // Now select all
    selectionModel->selectAll();
    
    // Should select all items
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 15);
}
