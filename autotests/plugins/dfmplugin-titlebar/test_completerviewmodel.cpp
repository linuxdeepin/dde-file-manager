// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/completerviewmodel.h"

#include <gtest/gtest.h>
#include <QStandardItem>

using namespace dfmplugin_titlebar;

class CompleterViewModelTest : public testing::Test
{
protected:
    void SetUp() override
    {
        model = new CompleterViewModel();
    }

    void TearDown() override
    {
        delete model;
        model = nullptr;
    }

    CompleterViewModel *model { nullptr };
};

TEST_F(CompleterViewModelTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CompleterViewModelTest, SetStringList_EmptyList_NoItemsAdded)
{
    QStringList emptyList;
    model->setStringList(emptyList);
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CompleterViewModelTest, SetStringList_SingleItem_OneItemAdded)
{
    QStringList list { "item1" };
    model->setStringList(list);
    EXPECT_EQ(model->rowCount(), 1);

    QModelIndex idx = model->index(0, 0);
    EXPECT_EQ(model->data(idx, Qt::DisplayRole).toString(), QString("item1"));
}

TEST_F(CompleterViewModelTest, SetStringList_MultipleItems_AllItemsAdded)
{
    QStringList list { "item1", "item2", "item3" };
    model->setStringList(list);
    EXPECT_EQ(model->rowCount(), 3);

    for (int i = 0; i < 3; ++i) {
        QModelIndex idx = model->index(i, 0);
        EXPECT_EQ(model->data(idx, Qt::DisplayRole).toString(), list[i]);
    }
}

TEST_F(CompleterViewModelTest, SetStringList_WithEmptyStrings_EmptyStringsSkipped)
{
    QStringList list { "item1", "", "item2", "", "item3" };
    model->setStringList(list);
    // Empty strings should be skipped, so only 3 items
    EXPECT_EQ(model->rowCount(), 3);

    QModelIndex idx0 = model->index(0, 0);
    QModelIndex idx1 = model->index(1, 0);
    QModelIndex idx2 = model->index(2, 0);
    EXPECT_EQ(model->data(idx0, Qt::DisplayRole).toString(), QString("item1"));
    EXPECT_EQ(model->data(idx1, Qt::DisplayRole).toString(), QString("item2"));
    EXPECT_EQ(model->data(idx2, Qt::DisplayRole).toString(), QString("item3"));
}

TEST_F(CompleterViewModelTest, SetStringList_AllEmptyStrings_NoItemsAdded)
{
    QStringList list { "", "", "" };
    model->setStringList(list);
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CompleterViewModelTest, SetStringList_OverwritesPreviousData_OldDataCleared)
{
    // First set
    QStringList list1 { "old1", "old2" };
    model->setStringList(list1);
    EXPECT_EQ(model->rowCount(), 2);

    // Second set should overwrite
    QStringList list2 { "new1", "new2", "new3" };
    model->setStringList(list2);
    EXPECT_EQ(model->rowCount(), 3);

    QModelIndex idx0 = model->index(0, 0);
    EXPECT_EQ(model->data(idx0, Qt::DisplayRole).toString(), QString("new1"));
}

TEST_F(CompleterViewModelTest, RemoveAll_EmptyModel_NoError)
{
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CompleterViewModelTest, RemoveAll_WithItems_AllItemsRemoved)
{
    QStringList list { "item1", "item2", "item3" };
    model->setStringList(list);
    EXPECT_EQ(model->rowCount(), 3);

    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CompleterViewModelTest, RemoveAll_MultipleOperations_ConsistentBehavior)
{
    // First batch
    model->setStringList(QStringList { "a", "b", "c" });
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);

    // Second batch
    model->setStringList(QStringList { "x", "y" });
    EXPECT_EQ(model->rowCount(), 2);
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CompleterViewModelTest, SetStringList_LargeList_AllItemsAdded)
{
    QStringList largeList;
    for (int i = 0; i < 100; ++i) {
        largeList << QString("item%1").arg(i);
    }

    model->setStringList(largeList);
    EXPECT_EQ(model->rowCount(), 100);

    // Check first and last
    QModelIndex idx0 = model->index(0, 0);
    QModelIndex idx99 = model->index(99, 0);
    EXPECT_EQ(model->data(idx0, Qt::DisplayRole).toString(), QString("item0"));
    EXPECT_EQ(model->data(idx99, Qt::DisplayRole).toString(), QString("item99"));
}

TEST_F(CompleterViewModelTest, SetStringList_SpecialCharacters_HandledCorrectly)
{
    QStringList list { "/home/user", "~/Documents", "file:///path", "C:\\Windows" };
    model->setStringList(list);
    EXPECT_EQ(model->rowCount(), 4);

    for (int i = 0; i < 4; ++i) {
        QModelIndex idx = model->index(i, 0);
        EXPECT_EQ(model->data(idx, Qt::DisplayRole).toString(), list[i]);
    }
}
