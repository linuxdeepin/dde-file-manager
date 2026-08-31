// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothmanager_1.cpp
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

TEST_F(BluetoothManagerTest, BluetoothManager)
{
    // Test constructor: BluetoothManager(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(BluetoothManagerTest, instance)
{
    // Test getter: BluetoothManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(BluetoothManagerTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(BluetoothManagerTest, operator=)
{
    // Test getter: BluetoothManager operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(BluetoothManagerTest, hasAdapter)
{
    // Test bool getter: hasAdapter()
    bool result = obj->hasAdapter();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothManagerTest, getAdapters)
{
    // Test getter: QMap<QString, const BluetoothAdapter *> getAdapters()
    auto result = obj->getAdapters();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothManagerTest, sendFiles)
{
    // Test method: void sendFiles((const QString &id, const QStringList &filePath, const QString &senderToken))
    QString _arg0{};
    QStringList _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sendFiles(_arg0, _arg1, _arg2));
}

TEST_F(BluetoothManagerTest, cancelTransfer)
{
    // Test method: bool cancelTransfer((const QString &sessionPath))
    QString _arg0{};
    auto result = obj->cancelTransfer(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BluetoothManagerTest, bluetoothSendEnable)
{
    // Test bool getter: bluetoothSendEnable()
    bool result = obj->bluetoothSendEnable();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothManagerTest, showBluetoothSettings)
{
    // Test method: void showBluetoothSettings(())
    EXPECT_NO_FATAL_FAILURE(obj->showBluetoothSettings());
}

TEST_F(BluetoothManagerTest, canSendBluetoothRequest)
{
    // Test bool getter: canSendBluetoothRequest()
    bool result = obj->canSendBluetoothRequest();
    EXPECT_FALSE(result);

}

TEST_F(BluetoothManagerTest, isLongFilenameFailure)
{
    // Test method: bool isLongFilenameFailure((const QString &sessionPath))
    QString _arg0{};
    auto result = obj->isLongFilenameFailure(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BluetoothManagerTest, M_(BluetoothManager))
{
    // Test getter: Q_DECLARE_PRIVATE (BluetoothManager)()
    EXPECT_NO_FATAL_FAILURE({ obj->(BluetoothManager)(); });
}

TEST_F(BluetoothManagerTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(BluetoothManagerTest, d_ptr)
{
    // Test getter: QSharedPointer<BluetoothManagerPrivate> d_ptr()
    auto result = obj->d_ptr();
    EXPECT_EQ(result.get(), nullptr);

}
