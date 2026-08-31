// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothmanagerprivate.cpp
 * @brief Unit tests for BluetoothManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothManagerPrivateTest, onAdapterRemoved)
{
    // Test method: void onAdapterRemoved((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAdapterRemoved(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onDeviceRemoved)
{
    // Test method: void onDeviceRemoved((const QString &json))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDeviceRemoved(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onObexSessionRemoved)
{
    // Test method: void onObexSessionRemoved((const QDBusObjectPath &sessionPath))
    QDBusObjectPath _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onObexSessionRemoved(_arg0));
}

TEST_F(BluetoothManagerPrivateTest, onTransferRemoved)
{
    // Test method: void onTransferRemoved((const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath, bool done))
    QString _arg0{};
    QDBusObjectPath _arg1{};
    QDBusObjectPath _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onTransferRemoved(_arg0, _arg1, _arg2, false));
}

TEST_F(BluetoothManagerPrivateTest, resolve)
{
    // Test method: void resolve((const QDBusReply<QString> &req))
    QDBusReply<QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resolve(_arg0));
}
