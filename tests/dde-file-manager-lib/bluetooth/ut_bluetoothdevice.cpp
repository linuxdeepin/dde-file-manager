// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetooth/bluetoothdevice.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestBluetoothDevice: public testing::Test {
public:
    BluetoothDevice *m_device {nullptr};

    void SetUp() override
    {
        m_device = new BluetoothDevice;
    }

    void TearDown() override
    {
        delete m_device;
    }
};

TEST_F(TestBluetoothDevice, id)
{
    EXPECT_STREQ("", m_device->id().toStdString().c_str());
    m_device->setId("123asd;',");
    EXPECT_STREQ("123asd;',", m_device->id().toStdString().c_str());
}

TEST_F(TestBluetoothDevice, name)
{
    EXPECT_STREQ("", m_device->name().toStdString().c_str());
    m_device->setName("123asd;',");
    EXPECT_STREQ("123asd;',", m_device->name().toStdString().c_str());
}

TEST_F(TestBluetoothDevice, alias)
{
    EXPECT_STREQ("", m_device->alias().toStdString().c_str());
    m_device->setAlias("123asd;',");
    EXPECT_STREQ("123asd;',", m_device->alias().toStdString().c_str());
}

TEST_F(TestBluetoothDevice, icon)
{
    EXPECT_STREQ("", m_device->icon().toStdString().c_str());
    m_device->setIcon("123asd;',");
    EXPECT_STREQ("123asd;',", m_device->icon().toStdString().c_str());
}

TEST_F(TestBluetoothDevice, paired)
{
    EXPECT_FALSE(m_device->paired());
    m_device->setPaired(true);
    EXPECT_TRUE(m_device->paired());
}

TEST_F(TestBluetoothDevice, trusted)
{
    EXPECT_FALSE(m_device->trusted());
    m_device->setTrusted(true);
    EXPECT_TRUE(m_device->trusted());
}

TEST_F(TestBluetoothDevice, state)
{
    m_device->setState(BluetoothDevice::StateConnected);
    EXPECT_EQ(BluetoothDevice::StateConnected, m_device->state());
}
}
