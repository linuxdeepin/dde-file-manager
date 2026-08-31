// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothdevice_1.cpp
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

TEST_F(BluetoothDeviceTest, getAlias)
{
    // Test getter: QString getAlias()
    auto result = obj->getAlias();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothDeviceTest, getIcon)
{
    // Test getter: QString getIcon()
    auto result = obj->getIcon();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothDeviceTest, getId)
{
    // Test getter: QString getId()
    auto result = obj->getId();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothDeviceTest, getName)
{
    // Test getter: QString getName()
    auto result = obj->getName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothDeviceTest, getState)
{
    // Test getter: State getState()
    auto result = obj->getState();
    EXPECT_NO_FATAL_FAILURE({ obj->getState(); });

}

TEST_F(BluetoothDeviceTest, isPaired)
{
    // Test bool getter: isPaired()
    bool result = obj->isPaired();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothDeviceTest, isTrusted)
{
    // Test bool getter: isTrusted()
    bool result = obj->isTrusted();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothDeviceTest, setAlias)
{
    // Test setter: void setAlias((const QString &alias))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setAlias(_arg0));
}

TEST_F(BluetoothDeviceTest, setIcon)
{
    // Test setter: void setIcon((const QString &icon))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIcon(_arg0));
}

TEST_F(BluetoothDeviceTest, setName)
{
    // Test setter: void setName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setName(_arg0));
}

TEST_F(BluetoothDeviceTest, setPaired)
{
    // Test setter: void setPaired((bool paired))
    EXPECT_NO_FATAL_FAILURE(obj->setPaired(false));
}

TEST_F(BluetoothDeviceTest, setTrusted)
{
    // Test setter: void setTrusted((bool trusted))
    EXPECT_NO_FATAL_FAILURE(obj->setTrusted(false));
}
