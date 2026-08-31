// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptsetupprivate.cpp
 * @brief Unit tests for DiskEncryptSetupPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/dbus/diskencryptsetup.h"

#include <QTest>

using namespace src;

class DiskEncryptSetupPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptSetupPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptSetupPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptSetupPrivateTest, parseCredentialsFromFd)
{
    // Test method: bool parseCredentialsFromFd((const QDBusUnixFileDescriptor &credentialsFd, QVariantMap *args))
    QDBusUnixFileDescriptor _arg0{};
    auto result = obj->parseCredentialsFromFd(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, removeMarkerFile)
{
    // Test method: bool removeMarkerFile((const QString &path))
    QString _arg0{};
    auto result = obj->removeMarkerFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, removeOverlayDMFlagFile)
{
    // Test bool getter: removeOverlayDMFlagFile()
    bool result = obj->removeOverlayDMFlagFile();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, removeOverlayDMPendingFile)
{
    // Test bool getter: removeOverlayDMPendingFile()
    bool result = obj->removeOverlayDMPendingFile();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, syncConfigWithFileSystem)
{
    // Test method: void syncConfigWithFileSystem(())
    EXPECT_NO_FATAL_FAILURE(obj->syncConfigWithFileSystem());
}

TEST_F(DiskEncryptSetupPrivateTest, updateInitramfs)
{
    // Test bool getter: updateInitramfs()
    bool result = obj->updateInitramfs();
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptSetupPrivateTest, validateDecryptArgs)
{
    // Test method: bool validateDecryptArgs((const QVariantMap &args))
    QVariantMap _arg0{};
    auto result = obj->validateDecryptArgs(_arg0);
    EXPECT_FALSE(result);

}
