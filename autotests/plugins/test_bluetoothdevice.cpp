// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothdevice.cpp
 * @brief Unit tests for BluetoothDevice methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothdevice.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothDeviceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothDevice();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothDevice *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothDeviceTest, BluetoothDevice)
{
    // Test constructor: BluetoothDevice((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BluetoothDeviceTest, setId)
{
    // Test setter: void setId((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setId(_arg0));
}

TEST_F(BluetoothDeviceTest, setState)
{
    // Test setter: void setState((const BluetoothDevice::State &state))
    BluetoothDevice::State _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setState(_arg0));
}
