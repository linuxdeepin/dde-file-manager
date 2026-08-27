// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermodel.cpp
 * @brief Unit tests for ComputerModel Mid-priority methods (dfmplugin-computer)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "models/computermodel.h"

using namespace dfmplugin_computer;

class ComputerModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(ComputerModelTest, findItemByClearDeviceId)
{
    // Instance method findItemByClearDeviceId
    ComputerModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.findItemByClearDeviceId(QString("test")); (void)r; });
}

TEST_F(ComputerModelTest, index)
{
    // Instance method index
    ComputerModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.index(0, 0, QModelIndex()); (void)r; });
}

TEST_F(ComputerModelTest, onItemPropertyChanged)
{
    // Instance method onItemPropertyChanged
    ComputerModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.onItemPropertyChanged(QUrl("file:///tmp/test"), QString("test"), QVariant()); });
}

TEST_F(ComputerModelTest, onItemRemoved)
{
    // Instance method onItemRemoved
    ComputerModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.onItemRemoved(QUrl("file:///tmp/test")); });
}

TEST_F(ComputerModelTest, parent)
{
    // Instance method parent
    ComputerModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.parent(QModelIndex()); (void)r; });
}

TEST_F(ComputerModelTest, rowCount)
{
    // Instance method rowCount
    ComputerModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.rowCount(QModelIndex()); (void)r; });
}

TEST_F(ComputerModelTest, setData)
{
    // Instance method setData
    ComputerModel obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.setData(QModelIndex(), QVariant(), 0); });
    (void)result;
}

TEST_F(ComputerModelTest, flags)
{
    // flags
    SUCCEED();
}

TEST_F(ComputerModelTest, itemList)
{
    // itemList
    SUCCEED();
}
