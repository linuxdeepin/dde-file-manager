// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_diskencryptsetup.cpp
 * @brief Unit tests for DiskEncryptSetup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/dbus/diskencryptsetup.h"

#include <QTest>

using namespace src;

class DiskEncryptSetupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptSetup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptSetup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptSetupTest, DeviceStatus)
{
    // Test method: int DeviceStatus((const QString &dev))
    QString _arg0{};
    auto result = obj->DeviceStatus(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(DiskEncryptSetupTest, HolderDevice)
{
    // Test method: QString HolderDevice((const QString &dev))
    QString _arg0{};
    auto result = obj->HolderDevice(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptSetupTest, IgnoreAuthSetup)
{
    // Test method: void IgnoreAuthSetup(())
    EXPECT_NO_FATAL_FAILURE(obj->IgnoreAuthSetup());
}

TEST_F(DiskEncryptSetupTest, InitEncryption)
{
    // Test method: bool InitEncryption((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->InitEncryption(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupTest, IsTaskEmpty)
{
    // Test bool getter: IsTaskEmpty()
    bool result = obj->IsTaskEmpty();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupTest, IsTaskRunning)
{
    // Test bool getter: IsTaskRunning()
    bool result = obj->IsTaskRunning();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupTest, PendingDecryptionDevice)
{
    // Test getter: QString PendingDecryptionDevice()
    auto result = obj->PendingDecryptionDevice();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DiskEncryptSetupTest, ResumeEncryption)
{
    // Test method: bool ResumeEncryption((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->ResumeEncryption(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupTest, SetupAuthArgs)
{
    // Test method: bool SetupAuthArgs((const QDBusUnixFileDescriptor &credentialsFd))
    QDBusUnixFileDescriptor _arg0{};
    auto result = obj->SetupAuthArgs(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupTest, TpmToken)
{
    // Test method: QString TpmToken((const QString &dev))
    QString _arg0{};
    auto result = obj->TpmToken(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
