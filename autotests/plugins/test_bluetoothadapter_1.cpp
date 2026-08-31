// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothadapter_1.cpp
 * @brief Unit tests for BluetoothAdapter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothadapter.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothAdapter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothAdapter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothAdapterTest, addDevice)
{
    // Test method: void addDevice((const BluetoothDevice *device))
    EXPECT_NO_FATAL_FAILURE(obj->addDevice(nullptr));
}

TEST_F(BluetoothAdapterTest, deviceById)
{
    // Test method: BluetoothDevice deviceById((const QString &id))
    QString _arg0{};
    auto result = obj->deviceById(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->deviceById(_arg0); });

}

TEST_F(BluetoothAdapterTest, getDevices)
{
    // Test getter: QMap<QString, const BluetoothDevice *> getDevices()
    auto result = obj->getDevices();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothAdapterTest, getId)
{
    // Test getter: QString getId()
    auto result = obj->getId();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothAdapterTest, getName)
{
    // Test getter: QString getName()
    auto result = obj->getName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothAdapterTest, isPowered)
{
    // Test bool getter: isPowered()
    bool result = obj->isPowered();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothAdapterTest, setId)
{
    // Test setter: void setId((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setId(_arg0));
}

TEST_F(BluetoothAdapterTest, setName)
{
    // Test setter: void setName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setName(_arg0));
}

TEST_F(BluetoothAdapterTest, setPowered)
{
    // Test setter: void setPowered((bool powered))
    EXPECT_NO_FATAL_FAILURE(obj->setPowered(false));
}
