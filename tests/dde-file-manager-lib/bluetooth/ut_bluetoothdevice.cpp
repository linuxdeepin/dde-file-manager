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
