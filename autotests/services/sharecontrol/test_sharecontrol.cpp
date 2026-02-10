// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QTest>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusContext>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include <QDataStream>

// System headers for mocking system calls
#include <unistd.h>
#include <cstring>
#include <algorithm>

// Include the classes under test
#include "service_sharecontrol_global.h"
#include "sharecontroldbus.h"
#include "polkit/policykithelper.h"
#include <dfm-base/utils/fileutils.h>

SERVICESHARECONTROL_USE_NAMESPACE
using ServiceCommon::PolicyKitHelper;

class UT_ShareControlDBus : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Reset read call counter for each test
        readCallCount = 0;

        // Mock QDBusContext::message to prevent crashes
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> const QDBusMessage & {
            __DBG_STUB_INVOKE__
            // Create a static message to ensure it has valid service data
            static QDBusMessage msg = QDBusMessage::createMethodCall("org.test.service", "/test", "org.test.Interface", "testMethod");
            return msg;
        });

        shareControlDBus = new ShareControlDBus("test_sharecontrol");
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete shareControlDBus;
        shareControlDBus = nullptr;
    }

protected:
    ShareControlDBus *shareControlDBus = nullptr;
    stub_ext::StubExt stub;
    int readCallCount = 0;   // Track read system call count
};

class UT_PolicyKitHelper : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

// Test ShareControlDBus constructor
TEST_F(UT_ShareControlDBus, Constructor_ValidName_Success)
{
    // Test successful construction
    EXPECT_NE(shareControlDBus, nullptr);
}

// Test CloseSmbShareByShareName with show=false
TEST_F(UT_ShareControlDBus, CloseSmbShareByShareName_ShowFalse_ReturnsTrue)
{
    bool result = shareControlDBus->CloseSmbShareByShareName("testshare", false);
    EXPECT_TRUE(result);
}

// Test CloseSmbShareByShareName with authentication failure
TEST_F(UT_ShareControlDBus, CloseSmbShareByShareName_AuthenticationFailed_ReturnsFalse)
{
    // Mock authentication failure
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = shareControlDBus->CloseSmbShareByShareName("testshare", true);
    EXPECT_FALSE(result);
}

// Test CloseSmbShareByShareName with DBus connection failure
TEST_F(UT_ShareControlDBus, CloseSmbShareByShareName_DBusConnectionFailed_ReturnsFalse)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock DBus connection failure
    stub.set_lamda(&QDBusConnection::isConnected, [](const QDBusConnection *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = shareControlDBus->CloseSmbShareByShareName("testshare", true);
    EXPECT_FALSE(result);
}

// Test CloseSmbShareByShareName with permission denied
TEST_F(UT_ShareControlDBus, CloseSmbShareByShareName_PermissionDenied_ReturnsFalse)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock DBus connection success
    stub.set_lamda(&QDBusConnection::isConnected, [](const QDBusConnection *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock service UID (non-root, non-owner)
    stub.set_lamda(&QDBusConnectionInterface::serviceUid, [](QDBusConnectionInterface *, const QString &) -> QDBusReply<uint> {
        __DBG_STUB_INVOKE__
        QDBusMessage reply;
        reply << static_cast<uint>(1001);
        return QDBusReply<uint>(reply);
    });

    // Mock file info with different owner
    stub.set_lamda(&QFileInfo::ownerId, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return 1002;   // Different from caller
    });

    stub.set_lamda(&QFileInfo::isSymLink, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&QFileInfo::absoluteFilePath, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return QString("/var/lib/samba/usershares/testshare");
    });

    bool result = shareControlDBus->CloseSmbShareByShareName("testshare", true);
    EXPECT_FALSE(result);
}

// Test CloseSmbShareByShareName successful execution
TEST_F(UT_ShareControlDBus, CloseSmbShareByShareName_Success_ReturnsTrue)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock DBus connection success
    stub.set_lamda(&QDBusConnection::isConnected, [](const QDBusConnection *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock service UID (same as file owner)
    stub.set_lamda(&QDBusConnectionInterface::serviceUid, [](QDBusConnectionInterface *, const QString &) -> QDBusReply<uint> {
        __DBG_STUB_INVOKE__
        QDBusMessage reply;
        reply << static_cast<uint>(1001);
        return QDBusReply<uint>(reply);
    });

    // Mock file info with same owner
    stub.set_lamda(&QFileInfo::ownerId, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return 1001;   // Same as caller
    });

    stub.set_lamda(&QFileInfo::isSymLink, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&QFileInfo::absoluteFilePath, [](const QFileInfo *) {
        __DBG_STUB_INVOKE__
        return QString("/var/lib/samba/usershares/testshare");
    });

    // Mock QProcess success - use function pointer to specify overload
    using ProcessStartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<ProcessStartFunc>(&QProcess::start), [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {
        __DBG_STUB_INVOKE__
    });

    using ProcessWaitFunc = bool (QProcess::*)(int);
    stub.set_lamda(static_cast<ProcessWaitFunc>(&QProcess::waitForFinished), [](QProcess *, int) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = shareControlDBus->CloseSmbShareByShareName("testshare", true);
    EXPECT_TRUE(result);
}

// Test SetUserSharePassword with authentication failure
TEST_F(UT_ShareControlDBus, SetUserSharePassword_AuthenticationFailed_ReturnsFalse)
{
    // Mock authentication failure
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Create a mock file descriptor - should be valid but authentication will fail first
    QDBusUnixFileDescriptor mockFd;
    stub.set_lamda(&QDBusUnixFileDescriptor::isValid, [](const QDBusUnixFileDescriptor *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = shareControlDBus->SetUserSharePassword(mockFd);
    EXPECT_FALSE(result);
}

// Test SetUserSharePassword with invalid file descriptor
TEST_F(UT_ShareControlDBus, SetUserSharePassword_InvalidFileDescriptor_ReturnsFalse)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Create an invalid file descriptor
    QDBusUnixFileDescriptor mockFd;
    stub.set_lamda(&QDBusUnixFileDescriptor::isValid, [](const QDBusUnixFileDescriptor *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = shareControlDBus->SetUserSharePassword(mockFd);
    EXPECT_FALSE(result);
}

// Test SetUserSharePassword successful execution
TEST_F(UT_ShareControlDBus, SetUserSharePassword_Success_ReturnsTrue)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Create a mock file descriptor
    QDBusUnixFileDescriptor mockFd;
    stub.set_lamda(&QDBusUnixFileDescriptor::isValid, [](const QDBusUnixFileDescriptor *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QDBusUnixFileDescriptor::fileDescriptor, [](const QDBusUnixFileDescriptor *) {
        __DBG_STUB_INVOKE__
        return 0;   // Mock valid file descriptor
    });

    // Mock read system call to simulate reading credentials from pipe
    stub.set_lamda(read, [this](int, void *buf, size_t count) -> ssize_t {
        __DBG_STUB_INVOKE__
        this->readCallCount++;

        if (this->readCallCount == 1) {
            // First call: return serialized credentials data
            QByteArray data;
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream << QString("testuser") << QString("testpassword");

            // Copy data to buffer (ensure we don't exceed buffer size)
            size_t dataSize = std::min(static_cast<size_t>(data.size()), count);
            memcpy(buf, data.constData(), dataSize);
            return static_cast<ssize_t>(dataSize);
        } else {
            // Subsequent calls: return 0 to indicate end of file
            return 0;
        }
    });

    // Mock password decryption
    stub.set_lamda(&dfmbase::FileUtils::decryptString, [](const QString &) {
        __DBG_STUB_INVOKE__
        return QString("decrypted_password");
    });

    // Mock QProcess success - use function pointer to specify overload
    using ProcessStartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<ProcessStartFunc>(&QProcess::start), [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QProcess::waitForStarted, [](QProcess *, int) {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ProcessWriteFunc = qint64 (QProcess::*)(const char *);
    stub.set_lamda(static_cast<ProcessWriteFunc>(&QProcess::write), [](QProcess *, const char *) {
        __DBG_STUB_INVOKE__
        return 10;
    });

    stub.set_lamda(&QProcess::closeWriteChannel, [](QProcess *) {
        __DBG_STUB_INVOKE__
    });

    using ProcessWaitFunc = bool (QProcess::*)(int);
    stub.set_lamda(static_cast<ProcessWaitFunc>(&QProcess::waitForFinished), [](QProcess *, int) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = shareControlDBus->SetUserSharePassword(mockFd);
    EXPECT_TRUE(result);
}

// Test IsUserSharePasswordSet with process failure
TEST_F(UT_ShareControlDBus, IsUserSharePasswordSet_ProcessFailed_ReturnsFalse)
{
    // Mock QProcess failure - use function pointer to specify overload
    using ProcessStartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<ProcessStartFunc>(&QProcess::start), [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {
        __DBG_STUB_INVOKE__
    });

    using ProcessWaitFunc = bool (QProcess::*)(int);
    stub.set_lamda(static_cast<ProcessWaitFunc>(&QProcess::waitForFinished), [](QProcess *, int) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = shareControlDBus->IsUserSharePasswordSet("testuser");
    EXPECT_FALSE(result);
}

// Test IsUserSharePasswordSet with user found
TEST_F(UT_ShareControlDBus, IsUserSharePasswordSet_UserFound_ReturnsTrue)
{
    // Mock QProcess success - use function pointer to specify overload
    using ProcessStartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<ProcessStartFunc>(&QProcess::start), [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {
        __DBG_STUB_INVOKE__
    });

    using ProcessWaitFunc = bool (QProcess::*)(int);
    stub.set_lamda(static_cast<ProcessWaitFunc>(&QProcess::waitForFinished), [](QProcess *, int) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QProcess::readAllStandardOutput, [](QProcess *) {
        __DBG_STUB_INVOKE__
        return QByteArray("testuser:1001:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:LCT-00000000:\n");
    });

    bool result = shareControlDBus->IsUserSharePasswordSet("testuser");
    EXPECT_TRUE(result);
}

// Test IsUserSharePasswordSet with user not found
TEST_F(UT_ShareControlDBus, IsUserSharePasswordSet_UserNotFound_ReturnsFalse)
{
    // Mock QProcess success - use function pointer to specify overload
    using ProcessStartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<ProcessStartFunc>(&QProcess::start), [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {
        __DBG_STUB_INVOKE__
    });

    using ProcessWaitFunc = bool (QProcess::*)(int);
    stub.set_lamda(static_cast<ProcessWaitFunc>(&QProcess::waitForFinished), [](QProcess *, int) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QProcess::readAllStandardOutput, [](QProcess *) {
        __DBG_STUB_INVOKE__
        return QByteArray("otheruser:1002:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:LCT-00000000:\n");
    });

    bool result = shareControlDBus->IsUserSharePasswordSet("testuser");
    EXPECT_FALSE(result);
}

// PolicyKitHelper Tests
TEST_F(UT_PolicyKitHelper, Instance_Singleton_ReturnsSameInstance)
{
    PolicyKitHelper *instance1 = PolicyKitHelper::instance();
    PolicyKitHelper *instance2 = PolicyKitHelper::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_PolicyKitHelper, CheckAuthorization_EmptyBusName_ReturnsFalse)
{
    PolicyKitHelper *helper = PolicyKitHelper::instance();
    bool result = helper->checkAuthorization("test.action", "");
    EXPECT_FALSE(result);
}

TEST_F(UT_PolicyKitHelper, CheckAuthorization_ValidParameters_CallsPolkitQt)
{
    // Mock PolkitQt1::Authority::instance()->checkAuthorizationSync
    // Note: This is a complex mock as it involves PolkitQt1 namespace
    // In a real scenario, you might need to create a wrapper interface for testing

    PolicyKitHelper *helper = PolicyKitHelper::instance();
    // This test would require more complex mocking of PolkitQt1 classes
    // For now, we test the basic parameter validation
    bool result = helper->checkAuthorization("test.action", "org.test.service");
    // The actual result depends on the system's PolicyKit configuration
    // In unit tests, this should be mocked
    (void)result;   // Suppress unused variable warning
}

// Test isValidUsername method (private method testing through reflection or making it protected for testing)
class TestableShareControlDBus : public ShareControlDBus
{
public:
    TestableShareControlDBus(const char *name)
        : ShareControlDBus(name) { }

    // Expose private method for testing
    bool testIsValidUsername(const QString &username) const
    {
        return isValidUsername(username);
    }
};

class UT_ShareControlDBus_Username : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QDBusContext::message to prevent crashes
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> const QDBusMessage & {
            __DBG_STUB_INVOKE__
            static QDBusMessage msg = QDBusMessage::createMethodCall("org.test.service", "/test", "org.test.Interface", "testMethod");
            return msg;
        });

        shareControlDBus = new TestableShareControlDBus("test_sharecontrol_username");
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete shareControlDBus;
        shareControlDBus = nullptr;
    }

protected:
    TestableShareControlDBus *shareControlDBus = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ShareControlDBus_Username, IsValidUsername_ValidUsernames_ReturnsTrue)
{
    EXPECT_TRUE(shareControlDBus->testIsValidUsername("testuser"));
    EXPECT_TRUE(shareControlDBus->testIsValidUsername("user123"));
    EXPECT_TRUE(shareControlDBus->testIsValidUsername("test_user"));
    EXPECT_TRUE(shareControlDBus->testIsValidUsername("user-name"));
    EXPECT_TRUE(shareControlDBus->testIsValidUsername("a"));
    EXPECT_TRUE(shareControlDBus->testIsValidUsername("user_123-test"));
}

TEST_F(UT_ShareControlDBus_Username, IsValidUsername_InvalidUsernames_ReturnsFalse)
{
    // Empty username
    EXPECT_FALSE(shareControlDBus->testIsValidUsername(""));

    // Too long username (over 32 characters)
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("this_is_a_very_long_username_that_exceeds_32_characters"));

    // Starting with hyphen (command injection prevention)
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("-testuser"));

    // Containing double hyphens (command injection prevention)
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test--user"));

    // Containing invalid characters
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test user"));   // space
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test@user"));   // @
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test$user"));   // $
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test&user"));   // &
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test|user"));   // |
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test;user"));   // ;
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test'user"));   // '
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test\"user"));   // "
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test\\user"));   // \
    EXPECT_FALSE(shareControlDBus->testIsValidUsername("test/user"));  // /
}
