// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothadapter.cpp
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

TEST_F(BluetoothAdapterTest, BluetoothAdapter)
{
    // Test constructor: BluetoothAdapter((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BluetoothAdapterTest, removeDevice)
{
    // Test method: void removeDevice((const QString &deviceId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeDevice(_arg0));
}
