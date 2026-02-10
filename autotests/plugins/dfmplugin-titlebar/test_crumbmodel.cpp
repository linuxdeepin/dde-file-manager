// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/crumbmodel.h"

#include <gtest/gtest.h>
#include <QStandardItem>
#include <QUrl>

using namespace dfmplugin_titlebar;

class CrumbModelTest : public testing::Test
{
protected:
    void SetUp() override
    {
        model = new CrumbModel();
    }

    void TearDown() override
    {
        delete model;
        model = nullptr;
    }

    CrumbModel *model { nullptr };
};

TEST_F(CrumbModelTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CrumbModelTest, RemoveAll_EmptyModel_NoError)
{
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CrumbModelTest, RemoveAll_WithItems_AllItemsRemoved)
{
    // Add some items
    for (int i = 0; i < 5; ++i) {
        QStandardItem *item = new QStandardItem(QString("Item %1").arg(i));
        model->appendRow(item);
    }
    EXPECT_EQ(model->rowCount(), 5);

    // Remove all
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CrumbModelTest, LastIndex_EmptyModel_InvalidIndex)
{
    QModelIndex idx = model->lastIndex();
    // With empty model (rowCount() = 0), lastIndex() returns index(-1, 0)
    EXPECT_EQ(idx.row(), -1);
    EXPECT_FALSE(idx.isValid());
}

TEST_F(CrumbModelTest, LastIndex_SingleItem_ReturnsFirstIndex)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);

    QModelIndex idx = model->lastIndex();
    EXPECT_EQ(idx.row(), 0);
    EXPECT_TRUE(idx.isValid());
}

TEST_F(CrumbModelTest, LastIndex_MultipleItems_ReturnsLastIndex)
{
    // Add 3 items
    for (int i = 0; i < 3; ++i) {
        QStandardItem *item = new QStandardItem(QString("Item %1").arg(i));
        model->appendRow(item);
    }

    QModelIndex idx = model->lastIndex();
    EXPECT_EQ(idx.row(), 2);
    EXPECT_TRUE(idx.isValid());
}

TEST_F(CrumbModelTest, CustomRoles_FileUrlRole_CanSetAndGet)
{
    QStandardItem *item = new QStandardItem("Test");
    QUrl testUrl("file:///home/test");
    item->setData(testUrl, CrumbModel::FileUrlRole);
    model->appendRow(item);

    QModelIndex idx = model->index(0, 0);
    QUrl retrievedUrl = model->data(idx, CrumbModel::FileUrlRole).toUrl();
    EXPECT_EQ(retrievedUrl, testUrl);
}

TEST_F(CrumbModelTest, CustomRoles_FullUrlRole_CanSetAndGet)
{
    QStandardItem *item = new QStandardItem("Test");
    QUrl testUrl("file:///home/test/full/path");
    item->setData(testUrl, CrumbModel::FullUrlRole);
    model->appendRow(item);

    QModelIndex idx = model->index(0, 0);
    QUrl retrievedUrl = model->data(idx, CrumbModel::FullUrlRole).toUrl();
    EXPECT_EQ(retrievedUrl, testUrl);
}

TEST_F(CrumbModelTest, RemoveAll_MultipleOperations_ConsistentBehavior)
{
    // First batch
    for (int i = 0; i < 3; ++i) {
        model->appendRow(new QStandardItem(QString("Item %1").arg(i)));
    }
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);

    // Second batch
    for (int i = 0; i < 5; ++i) {
        model->appendRow(new QStandardItem(QString("Item %1").arg(i)));
    }
    EXPECT_EQ(model->rowCount(), 5);
    model->removeAll();
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(CrumbModelTest, LastIndex_AfterRemoveAll_InvalidIndex)
{
    // Add and then remove
    for (int i = 0; i < 3; ++i) {
        model->appendRow(new QStandardItem(QString("Item %1").arg(i)));
    }
    model->removeAll();

    QModelIndex idx = model->lastIndex();
    EXPECT_FALSE(idx.isValid());
}
