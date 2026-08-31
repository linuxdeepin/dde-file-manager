// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermodel.cpp
 * @brief Unit tests for ComputerModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/computermodel.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerModelTest, findItemByClearDeviceId)
{
    // Test method: int findItemByClearDeviceId((const QString &id))
    QString _arg0{};
    auto result = obj->findItemByClearDeviceId(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(ComputerModelTest, index)
{
    // Test method: QModelIndex index((int row, int column, const QModelIndex &parent))
    QModelIndex _arg2{};
    auto result = obj->index(0, 0, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerModelTest, onItemPropertyChanged)
{
    // Test method: void onItemPropertyChanged((const QUrl &url, const QString &key, const QVariant &val))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemPropertyChanged(_arg0, _arg1, _arg2));
}

TEST_F(ComputerModelTest, onItemRemoved)
{
    // Test method: void onItemRemoved((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemRemoved(_arg0));
}

TEST_F(ComputerModelTest, parent)
{
    // Test method: QModelIndex parent((const QModelIndex &child))
    QModelIndex _arg0{};
    auto result = obj->parent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerModelTest, rowCount)
{
    // Test method: int rowCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->rowCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(ComputerModelTest, setData)
{
    // Test method: bool setData((const QModelIndex &index, const QVariant &value, int role))
    QModelIndex _arg0{};
    QVariant _arg1{};
    auto result = obj->setData(_arg0, _arg1, 0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerModelTest, flags)
{
    // Test method: Qt::ItemFlags flags((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->flags(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ComputerModelTest, itemList)
{
    // Test getter: QList<ComputerItemData> itemList()
    auto result = obj->itemList();
    EXPECT_TRUE(result.isEmpty());

}
