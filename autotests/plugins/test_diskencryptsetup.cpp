// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptsetup.cpp
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

TEST_F(DiskEncryptSetupTest, ChangePassphrase)
{
    // Test method: bool ChangePassphrase((const QDBusUnixFileDescriptor &credentialsFd))
    QDBusUnixFileDescriptor _arg0{};
    auto result = obj->ChangePassphrase(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupTest, Decryption)
{
    // Test method: bool Decryption((const QDBusUnixFileDescriptor &credentialsFd))
    QDBusUnixFileDescriptor _arg0{};
    auto result = obj->Decryption(_arg0);
    EXPECT_FALSE(result);

}
