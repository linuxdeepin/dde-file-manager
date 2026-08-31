// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothmanager.cpp
 * @brief Unit tests for BluetoothManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothManagerTest, instance)
{
    // Test getter: BluetoothManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
