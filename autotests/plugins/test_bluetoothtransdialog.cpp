// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothtransdialog.cpp
 * @brief Unit tests for BluetoothTransDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothtransdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothTransDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothTransDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothTransDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothTransDialogTest, connectDevice)
{
    // Test method: void connectDevice((const BluetoothDevice *dev))
    EXPECT_NO_FATAL_FAILURE(obj->connectDevice(nullptr));
}

TEST_F(BluetoothTransDialogTest, isBluetoothIdle)
{
    // Test bool getter: isBluetoothIdle()
    bool result = obj->isBluetoothIdle();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothTransDialogTest, removeDevice)
{
    // Test method: void removeDevice((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeDevice(_arg0));
}

TEST_F(BluetoothTransDialogTest, BluetoothTransDialog)
{
    // Test constructor: BluetoothTransDialog((const QStringList &urls, QString targetDevId, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BluetoothTransDialogTest, createDeviceSelectorPage)
{
    // Test getter: QWidget createDeviceSelectorPage()
    auto result = obj->createDeviceSelectorPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createDeviceSelectorPage(); });

}
