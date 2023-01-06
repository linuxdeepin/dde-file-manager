// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetooth/bluetoothadapter.h"
#include "bluetooth/bluetoothdevice.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestBluetoothAdapter: public testing::Test {
public:
    BluetoothAdapter *m_adapter {nullptr};

    void SetUp() override
    {
        m_adapter = new BluetoothAdapter;
    }

    void TearDown() override
    {
        delete m_adapter;
    }
};

TEST_F(TestBluetoothAdapter, id)
{
    EXPECT_STREQ("", m_adapter->id().toStdString().c_str());
    m_adapter->setId("123asd;',");
    EXPECT_STREQ("123asd;',", m_adapter->id().toStdString().c_str());
}

TEST_F(TestBluetoothAdapter, name)
{
    EXPECT_STREQ("", m_adapter->name().toStdString().c_str());
    m_adapter->setName("123asd;',");
    EXPECT_STREQ("123asd;',", m_adapter->name().toStdString().c_str());
}

TEST_F(TestBluetoothAdapter, powered)
{
    EXPECT_FALSE(m_adapter->powered());
    m_adapter->setPowered(true);
    EXPECT_TRUE(m_adapter->powered());
}

TEST_F(TestBluetoothAdapter, addDevice)
{
    BluetoothDevice *device = new BluetoothDevice;
    device->setId("abc");
    device->setName("123");
    m_adapter->addDevice(device);
    QMap<QString, const BluetoothDevice *> devices = m_adapter->devices();
    EXPECT_STREQ(devices["abc"]->name().toStdString().c_str(), "123");
    EXPECT_STREQ(m_adapter->deviceById("abc")->name().toStdString().c_str(), "123");
    delete device;
}
#ifndef __arm__
TEST_F(TestBluetoothAdapter, removeDevice)
{
    BluetoothDevice *device = new BluetoothDevice;
    device->setId("abc");
    device->setName("123");
    m_adapter->addDevice(device);
    EXPECT_STREQ(m_adapter->deviceById("abc")->name().toStdString().c_str(), "123");
    m_adapter->removeDevice("abc");
    QMap<QString, const BluetoothDevice *> devices = m_adapter->devices();
    EXPECT_EQ(devices.size(), 0);
    delete device;
}
#endif

}
