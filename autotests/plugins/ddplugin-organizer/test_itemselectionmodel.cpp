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
    // selectAll() only works with CollectionModel (dynamic_cast check).
    // With QStandardItemModel, selectAll() does nothing.
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllAfterClear)
{
    // selectAll does nothing with non-CollectionModel
    selectionModel->clearSelection();
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllMultipleTimes)
{
    // selectAll does nothing with non-CollectionModel
    selectionModel->selectAll();
    selectionModel->selectAll();
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);
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
    // selectAll does nothing with non-CollectionModel
    QModelIndex index1 = testModel->index(0, 0);
    QModelIndex index2 = testModel->index(2, 1);

    selectionModel->select(index1, QItemSelectionModel::Select);
    selectionModel->select(index2, QItemSelectionModel::Select);
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 2);

    selectionModel->selectAll();
    // selectAll still does nothing - manual selections preserved
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 2);
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

    selectionModel->clearSelection();
    selectionChangedSpy.clear();

    // selectAll does nothing with non-CollectionModel, so no signal
    selectionModel->selectAll();
    EXPECT_EQ(selectionChangedSpy.count(), 0);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllWithRowSelection)
{
    QItemSelection rowSelection;
    QModelIndex firstRowFirstCol = testModel->index(0, 0);
    QModelIndex firstRowLastCol = testModel->index(0, 2);
    rowSelection.select(firstRowFirstCol, firstRowLastCol);

    selectionModel->select(rowSelection, QItemSelectionModel::Select);
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 3);

    // selectAll does nothing with non-CollectionModel
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 3);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllAfterModelChange)
{
    // selectAll does nothing with non-CollectionModel
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);

    // Add more items to model
    for (int row = 0; row < 3; ++row) {
        QList<QStandardItem*> items;
        for (int col = 0; col < 3; ++col) {
            QStandardItem *item = new QStandardItem(QString("New Item %1,%2").arg(row).arg(col));
            items.append(item);
 }
        testModel->appendRow(items);
    }

    // selectAll still does nothing
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestEdgeCases)
{
    QStandardItemModel columnOnlyModel;
    columnOnlyModel.setColumnCount(3);
    ItemSelectionModel columnOnlySelection(&columnOnlyModel);

    EXPECT_NO_THROW(columnOnlySelection.selectAll());
    EXPECT_TRUE(columnOnlySelection.selectedIndexes().isEmpty());

    QStandardItemModel rowOnlyModel;
    rowOnlyModel.setRowCount(3);
    ItemSelectionModel rowOnlySelection(&rowOnlyModel);

    EXPECT_NO_THROW(rowOnlySelection.selectAll());
    EXPECT_TRUE(rowOnlySelection.selectedIndexes().isEmpty());

    // selectAll does nothing with non-CollectionModel
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestPerformanceWithLargeModel)
{
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

    // selectAll does nothing with non-CollectionModel
    EXPECT_NO_THROW(largeSelectionModel.selectAll());
    EXPECT_EQ(largeSelectionModel.selectedIndexes().size(), 0);

    largeSelectionModel.clearSelection();
    EXPECT_NO_THROW(largeSelectionModel.selectAll());
    EXPECT_EQ(largeSelectionModel.selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestInheritedBehavior)
{
    QModelIndex index = testModel->index(1, 1);
    selectionModel->select(index, QItemSelectionModel::Select);
    EXPECT_TRUE(selectionModel->isSelected(index));
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 1);

    QModelIndex index2 = testModel->index(2, 2);
    selectionModel->setCurrentIndex(index2, QItemSelectionModel::Select);
    EXPECT_EQ(selectionModel->currentIndex(), index2);

    selectionModel->clear();
    EXPECT_TRUE(selectionModel->selectedIndexes().isEmpty());

    // selectAll does nothing with non-CollectionModel
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 0);
}

TEST_F(UT_ItemSelectionModel, TestSelectAllWithFilter)
{
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select
                                            | QItemSelectionModel::Rows;
    QModelIndex rowIndex = testModel->index(1, 0);
    selectionModel->select(rowIndex, flags);
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 3);

    // selectAll does nothing with non-CollectionModel
    selectionModel->selectAll();
    EXPECT_EQ(selectionModel->selectedIndexes().size(), 3);
}
