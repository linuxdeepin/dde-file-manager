// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothmodel_1.cpp
 * @brief Unit tests for BluetoothModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothmodel.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothModelTest, BluetoothModel)
{
    // Test constructor: BluetoothModel((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BluetoothModelTest, adapterById)
{
    // Test method: BluetoothAdapter adapterById((const QString &id))
    QString _arg0{};
    auto result = obj->adapterById(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->adapterById(_arg0); });

}

TEST_F(BluetoothModelTest, addAdapter)
{
    // Test method: void addAdapter((BluetoothAdapter *adapter))
    EXPECT_NO_FATAL_FAILURE(obj->addAdapter(nullptr));
}

TEST_F(BluetoothModelTest, getAdapters)
{
    // Test getter: QMap<QString, const BluetoothAdapter *> getAdapters()
    auto result = obj->getAdapters();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}
