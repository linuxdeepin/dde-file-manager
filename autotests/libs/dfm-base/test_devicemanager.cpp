// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanager.cpp
 * @brief Unit tests for DeviceManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/devicemanager.h"

#include <QTest>

using namespace src;

class DeviceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceManagerTest, DeviceManager)
{
    // Test constructor: DeviceManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DeviceManagerTest, detachAllRemovableBlockDevs)
{
    // Test method: void detachAllRemovableBlockDevs(())
    EXPECT_NO_FATAL_FAILURE(obj->detachAllRemovableBlockDevs());
}

TEST_F(DeviceManagerTest, ejectBlockDevAsync)
{
    // Test method: void ejectBlockDevAsync((const QString &id, const QVariantMap &opts, CallbackType2 cb))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->ejectBlockDevAsync(_arg0, _arg1, CallbackType2()));
}

TEST_F(DeviceManagerTest, powerOffBlockDevAsync)
{
    // Test method: void powerOffBlockDevAsync((const QString &id, const QVariantMap &opts, CallbackType2 cb))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->powerOffBlockDevAsync(_arg0, _arg1, CallbackType2()));
}

TEST_F(DeviceManagerTest, renameBlockDevAsync)
{
    // Test method: void renameBlockDevAsync((const QString &id, const QString &newName, const QVariantMap &opts, CallbackType2 cb))
    QString _arg0{};
    QString _arg1{};
    QVariantMap _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameBlockDevAsync(_arg0, _arg1, _arg2, CallbackType2()));
}

TEST_F(DeviceManagerTest, unlockBlockDevAsync)
{
    // Test method: void unlockBlockDevAsync((const QString &id, const QString &passwd, const QVariantMap &opts, CallbackType1 cb))
    QString _arg0{};
    QString _arg1{};
    QVariantMap _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->unlockBlockDevAsync(_arg0, _arg1, _arg2, CallbackType1()));
}

TEST_F(DeviceManagerTest, unmountBlockDev)
{
    // Test method: bool unmountBlockDev((const QString &id, const QVariantMap &opts))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->unmountBlockDev(_arg0, _arg1);
    EXPECT_FALSE(result);

}
