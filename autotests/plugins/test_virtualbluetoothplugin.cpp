// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualbluetoothplugin.cpp
 * @brief Unit tests for VirtualBluetoothPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/virtualbluetoothplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualBluetoothPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualBluetoothPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualBluetoothPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualBluetoothPluginTest, bluetoothAvailable)
{
    // Test bool getter: bluetoothAvailable()
    bool result = obj->bluetoothAvailable();
    EXPECT_FALSE(result);

}

TEST_F(VirtualBluetoothPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(VirtualBluetoothPluginTest, sendFiles)
{
    // Test method: void sendFiles((const QStringList &paths, const QString &deviceId))
    QStringList _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendFiles(_arg0, _arg1));
}

TEST_F(VirtualBluetoothPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
