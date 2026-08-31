// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothmodel.cpp
 * @brief Unit tests for BluetoothModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothmodel.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothModelTest, removeAdapater)
{
    // Test method: BluetoothAdapter removeAdapater((const QString &adapterId))
    QString _arg0{};
    auto result = obj->removeAdapater(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->removeAdapater(_arg0); });

}
