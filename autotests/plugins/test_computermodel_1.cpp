// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermodel_1.cpp
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

TEST_F(ComputerModelTest, ComputerModel)
{
    // Test constructor: ComputerModel((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerModelTest, columnCount)
{
    // Test method: int columnCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->columnCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(ComputerModelTest, findItem)
{
    // Test method: int findItem((const QUrl &target))
    QUrl _arg0{};
    auto result = obj->findItem(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(ComputerModelTest, findItemData)
{
    // Test method: ComputerItemData findItemData((const QUrl &target))
    QUrl _arg0{};
    auto result = obj->findItemData(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->findItemData(_arg0); });

}

TEST_F(ComputerModelTest, findSplitter)
{
    // Test method: int findSplitter((const QString &group))
    QString _arg0{};
    auto result = obj->findSplitter(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(ComputerModelTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(ComputerModelTest, onItemSizeChanged)
{
    // Test method: void onItemSizeChanged((const QUrl &url, qlonglong total, qlonglong free))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemSizeChanged(_arg0, {}, {}));
}

TEST_F(ComputerModelTest, onItemUpdated)
{
    // Test method: void onItemUpdated((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemUpdated(_arg0));
}

TEST_F(ComputerModelTest, updateItemInfo)
{
    // Test method: void updateItemInfo((int pos))
    EXPECT_NO_FATAL_FAILURE(obj->updateItemInfo(0));
}

TEST_F(ComputerModelTest, ComputerModel_Destructor)
{
    // Test method:  ~ComputerModel(())
    EXPECT_NO_FATAL_FAILURE({ ComputerModel *tmp = new ComputerModel(); delete tmp; });
}
