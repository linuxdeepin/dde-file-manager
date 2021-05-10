/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
