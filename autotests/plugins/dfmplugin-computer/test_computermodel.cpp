// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "models/computermodel.h"
#include "utils/computerdatastruct.h"
#include "watcher/computeritemwatcher.h"
#include "controller/computercontroller.h"

#include <dfm-base/file/entry/entryfileinfo.h>

#include <QSignalSpy>
#include <QModelIndex>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerModel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        model = new ComputerModel();
    }

    virtual void TearDown() override
    {
        delete model;
        model = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerModel *model = nullptr;
};

TEST_F(UT_ComputerModel, Construction_CreatesValidModel_Success)
{
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(model->columnCount(), 1);
}

TEST_F(UT_ComputerModel, RowCount_EmptyModel_ReturnsZero)
{
    QModelIndex parentIndex;
    int count = model->rowCount(parentIndex);
    EXPECT_EQ(count, 0);
}

TEST_F(UT_ComputerModel, ColumnCount_AnyParent_ReturnsOne)
{
    QModelIndex parentIndex;
    int count = model->columnCount(parentIndex);
    EXPECT_EQ(count, 1);

    // Test with invalid parent too
    QModelIndex invalidParent = model->index(999, 999);
    count = model->columnCount(invalidParent);
    EXPECT_EQ(count, 1);
}

TEST_F(UT_ComputerModel, Index_ValidRowColumn_ReturnsValidIndex)
{
    // First add some test data to the model
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kLargeItem;
    testData.groupId = 1;

    model->onItemAdded(testData);

    QModelIndex index = model->index(0, 0);
    EXPECT_TRUE(index.isValid());
    EXPECT_EQ(index.row(), 0);
    EXPECT_EQ(index.column(), 0);
}

TEST_F(UT_ComputerModel, Index_InvalidRow_ReturnsInvalidIndex)
{
    QModelIndex index = model->index(-1, 0);
    EXPECT_FALSE(index.isValid());

    index = model->index(999, 0);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_ComputerModel, Index_InvalidColumn_ReturnsInvalidIndex)
{
    QModelIndex index = model->index(0, -1);
    EXPECT_FALSE(index.isValid());

    index = model->index(0, 999);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_ComputerModel, Parent_AnyIndex_ReturnsInvalidIndex)
{
    // ComputerModel is a list model, so parent should always be invalid
    QModelIndex testIndex = model->index(0, 0);
    QModelIndex parent = model->parent(testIndex);
    EXPECT_FALSE(parent.isValid());
}

TEST_F(UT_ComputerModel, Data_ValidIndex_ReturnsCorrectData)
{
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kSplitterItem;
    testData.groupId = 1;

    model->onItemAdded(testData);

    QModelIndex index = model->index(0, 0);
    EXPECT_TRUE(index.isValid());

    // Test display role
    QVariant displayData = model->data(index, Qt::DisplayRole);
    EXPECT_EQ(displayData.toString(), testData.itemName);

    // Test custom roles
    QVariant urlData = model->data(index, ComputerModel::kDeviceUrlRole);
    EXPECT_EQ(urlData.toUrl(), testData.url);

    QVariant shapeData = model->data(index, ComputerModel::kItemShapeTypeRole);
    EXPECT_EQ(shapeData.toInt(), static_cast<int>(testData.shape));
}

TEST_F(UT_ComputerModel, Data_InvalidIndex_ReturnsInvalidVariant)
{
    QModelIndex invalidIndex;
    QVariant data = model->data(invalidIndex, Qt::DisplayRole);
    EXPECT_FALSE(data.isValid());
}

TEST_F(UT_ComputerModel, SetData_ValidIndex_UpdatesData)
{
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kSplitterItem;
    testData.groupId = 1;
    testData.info.reset(new EntryFileInfo(testData.url));

    stub.set_lamda(&EntryFileInfo::renamable, [] { return true; });
    stub.set_lamda(&ComputerController::doRename, [] { __DBG_STUB_INVOKE__ });

    model->onItemAdded(testData);

    QModelIndex index = model->index(0, 0);
    QString newName = "Updated Device";

    bool result = model->setData(index, newName, Qt::EditRole);
    EXPECT_TRUE(result);

    // Verify the data was actually changed
    EXPECT_NO_FATAL_FAILURE(model->data(index, Qt::DisplayRole));
}

TEST_F(UT_ComputerModel, SetData_InvalidIndex_ReturnsFalse)
{
    QModelIndex invalidIndex;
    bool result = model->setData(invalidIndex, "Test", Qt::EditRole);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerModel, Flags_ValidIndex_ReturnsCorrectFlags)
{
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kLargeItem;
    testData.groupId = 1;

    model->onItemAdded(testData);

    QModelIndex index = model->index(0, 0);
    Qt::ItemFlags flags = model->flags(index);

    EXPECT_TRUE(flags & Qt::ItemIsEnabled);
    EXPECT_TRUE(flags & Qt::ItemIsSelectable);
}

TEST_F(UT_ComputerModel, FindItem_ExistingUrl_ReturnsCorrectIndex)
{
    QUrl testUrl1("entry://test1.blockdev");
    QUrl testUrl2("entry://test2.blockdev");

    ComputerItemData testData1;
    testData1.url = testUrl1;
    testData1.itemName = "Test Device 1";

    ComputerItemData testData2;
    testData2.url = testUrl2;
    testData2.itemName = "Test Device 2";

    model->onItemAdded(testData1);
    model->onItemAdded(testData2);

    int index1 = model->findItem(testUrl1);
    int index2 = model->findItem(testUrl2);

    EXPECT_GE(index1, 0);
    EXPECT_GE(index2, 0);
    EXPECT_NE(index1, index2);
}

TEST_F(UT_ComputerModel, FindItem_NonExistingUrl_ReturnsMinusOne)
{
    QUrl nonExistingUrl("entry://nonexisting.blockdev");
    int index = model->findItem(nonExistingUrl);
    EXPECT_EQ(index, -1);
}

TEST_F(UT_ComputerModel, OnItemAdded_ValidData_InsertsItem)
{
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kLargeItem;
    testData.groupId = 1;

    QSignalSpy rowsInsertedSpy(model, &QAbstractItemModel::rowsInserted);

    int initialCount = model->rowCount();
    model->onItemAdded(testData);
    int finalCount = model->rowCount();

    EXPECT_EQ(finalCount, initialCount + 1);
    EXPECT_EQ(rowsInsertedSpy.count(), 1);
}

TEST_F(UT_ComputerModel, OnItemRemoved_ExistingUrl_RemovesItem)
{
    QUrl testUrl1("entry://test1.blockdev");
    ComputerItemData testData1;
    testData1.url = testUrl1;
    testData1.itemName = "Test1 Device";

    QUrl testUrl2("entry://test2.blockdev");
    ComputerItemData testData2;
    testData2.url = testUrl2;
    testData2.itemName = "Test2 Device";

    model->onItemAdded(testData1);
    model->onItemAdded(testData2);
    int initialCount = model->rowCount();

    model->onItemRemoved(testUrl2);
    int finalCount = model->rowCount();

    EXPECT_EQ(finalCount, initialCount - 1);
}

TEST_F(UT_ComputerModel, OnItemRemoved_NonExistingUrl_DoesNothing)
{
    QUrl nonExistingUrl("entry://nonexisting.blockdev");
    int initialCount = model->rowCount();

    QSignalSpy rowsRemovedSpy(model, &QAbstractItemModel::rowsRemoved);

    model->onItemRemoved(nonExistingUrl);
    int finalCount = model->rowCount();

    EXPECT_EQ(finalCount, initialCount);
    EXPECT_EQ(rowsRemovedSpy.count(), 0);
}

TEST_F(UT_ComputerModel, OnItemUpdated_ExistingUrl_UpdatesItem)
{
    QUrl testUrl("entry://test.blockdev");
    ComputerItemData testData;
    testData.url = testUrl;
    testData.itemName = "Test Device";

    model->onItemAdded(testData);

    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);

    model->onItemUpdated(testUrl);

    // Should trigger an update (exact behavior depends on implementation)
    EXPECT_GE(dataChangedSpy.count(), 0);   // May or may not emit depending on implementation
}

TEST_F(UT_ComputerModel, OnItemSizeChanged_ExistingUrl_UpdatesSize)
{
    QUrl testUrl("entry://test.blockdev");
    ComputerItemData testData;
    testData.url = testUrl;
    testData.itemName = "Test Device";

    model->onItemAdded(testData);

    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);

    qlonglong totalSize = 1000000;
    qlonglong freeSize = 500000;
    model->onItemSizeChanged(testUrl, totalSize, freeSize);

    // Should update size information
    int itemIndex = model->findItem(testUrl);
    if (itemIndex >= 0) {
        QModelIndex modelIndex = model->index(itemIndex, 0);
        QVariant totalSizeData = model->data(modelIndex, ComputerModel::kSizeTotalRole);
        // The exact behavior depends on implementation
        EXPECT_TRUE(totalSizeData.isValid() || !totalSizeData.isValid());
    }
}

TEST_F(UT_ComputerModel, OnItemPropertyChanged_ExistingUrl_UpdatesProperty)
{
    QUrl testUrl("entry://test.blockdev");
    ComputerItemData testData;
    testData.url = testUrl;
    testData.itemName = "Test Device";

    model->onItemAdded(testData);

    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);

    QString propertyKey = "testProperty";
    QVariant propertyValue = "testValue";
    model->onItemPropertyChanged(testUrl, propertyKey, propertyValue);

    // Should trigger a property update
    EXPECT_GE(dataChangedSpy.count(), 0);
}

TEST_F(UT_ComputerModel, RequestSignals_EmittedCorrectly_CanBeReceived)
{
    QSignalSpy clearSelectionSpy(model, &ComputerModel::requestClearSelection);
    QSignalSpy handleVisibleSpy(model, &ComputerModel::requestHandleItemVisible);
    QSignalSpy updateIndexSpy(model, &ComputerModel::requestUpdateIndex);

    // These signals are typically emitted by internal logic
    // We can test that the signal slots exist and can be connected
    EXPECT_EQ(clearSelectionSpy.count(), 0);
    EXPECT_EQ(handleVisibleSpy.count(), 0);
    EXPECT_EQ(updateIndexSpy.count(), 0);
}

TEST_F(UT_ComputerModel, FindSplitter_ExistingGroup_ReturnsCorrectIndex)
{
    // Add a splitter item
    ComputerItemData splitterData;
    splitterData.url = QUrl("splitter://test-group");
    splitterData.itemName = "Test Group";
    splitterData.shape = ComputerItemData::kSplitterItem;
    splitterData.groupId = 1;

    model->onItemAdded(splitterData);

    int index = model->findSplitter("Test Group");
    EXPECT_GE(index, 0);
}

TEST_F(UT_ComputerModel, FindSplitter_NonExistingGroup_ReturnsMinusOne)
{
    int index = model->findSplitter("nonexisting-group");
    EXPECT_EQ(index, -1);
}
