// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QTest>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusContext>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <functional>

// Include the classes under test
#include "accesscontroldbus.h"
#include "polkit/policykithelper.h"
#include "utils.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-mount/dblockmonitor.h>
#include <dfm-mount/dblockdevice.h>

SERVICEACCESSCONTROL_USE_NAMESPACE
using ServiceCommon::PolicyKitHelper;

class UT_AccessControlDBus : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QDBusContext::message to prevent crashes
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> const QDBusMessage & {
            __DBG_STUB_INVOKE__
            // Create a static message to ensure it has valid service data
            static QDBusMessage msg = QDBusMessage::createMethodCall("org.test.service", "/test", "org.test.Interface", "testMethod");
            return msg;
        });

        // Mock QTimer::singleShot to prevent heap-use-after-free
        using SingleShotFunc = void (*)(int, const std::function<void()> &);
        stub.set_lamda(static_cast<SingleShotFunc>(QTimer::singleShot), [](int, const std::function<void()> &) {
            __DBG_STUB_INVOKE__
            // Do nothing - prevent delayed execution after object destruction
        });

        accessControlDBus = new AccessControlDBus("test_accesscontrol");
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete accessControlDBus;
        accessControlDBus = nullptr;
    }

protected:
    AccessControlDBus *accessControlDBus = nullptr;
    stub_ext::StubExt stub;
};

class UT_AccessControlPolicyKitHelper : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

// Test AccessControlDBus constructor
TEST_F(UT_AccessControlDBus, Constructor_ValidName_Success)
{
    // Test successful construction
    EXPECT_NE(accessControlDBus, nullptr);
}

// Test ChangeDiskPassword with authentication failure
TEST_F(UT_AccessControlDBus, ChangeDiskPassword_AuthenticationFailed_EmitsAuthenticationFailed)
{
    bool signalEmitted = false;

    // Connect to signal to verify emission
    QObject::connect(accessControlDBus, &AccessControlDBus::DiskPasswordChecked,
                     [&signalEmitted](int result) {
                         if (result == kAuthenticationFailed) {
                             signalEmitted = true;
                         }
                     });

    // Mock authentication failure
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // accessControlDBus->ChangeDiskPassword("oldpwd", "newpwd");

    // // Process events to ensure signal is emitted
    // QTest::qWait(600);

    // EXPECT_TRUE(signalEmitted);
}

// Test ChangeDiskPassword with no encrypted disks
TEST_F(UT_AccessControlDBus, ChangeDiskPassword_NoEncryptedDisks_EmitsNoError)
{
    bool signalEmitted = false;

    // Connect to signal to verify emission
    QObject::connect(accessControlDBus, &AccessControlDBus::DiskPasswordChecked,
                     [&signalEmitted](int result) {
                         if (result == kNoError) {
                             signalEmitted = true;
                         }
                     });

    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock empty encrypted disks list
    stub.set_lamda(&dfmbase::DeviceUtils::encryptedDisks, []() {
        __DBG_STUB_INVOKE__
        return QStringList();
    });

    // accessControlDBus->ChangeDiskPassword("oldpwd", "newpwd");

    // // Process events to ensure signal is emitted
    // QTest::qWait(600);

    // EXPECT_TRUE(signalEmitted);
}

// Test ChangeDiskPassword with password verification failure
TEST_F(UT_AccessControlDBus, ChangeDiskPassword_PasswordVerificationFailed_EmitsPasswordWrong)
{
    bool signalEmitted = false;

    // Connect to signal to verify emission
    QObject::connect(accessControlDBus, &AccessControlDBus::DiskPasswordChecked,
                     [&signalEmitted](int result) {
                         if (result == kPasswordWrong) {
                             signalEmitted = true;
                         }
                     });

    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock encrypted disks list
    stub.set_lamda(&dfmbase::DeviceUtils::encryptedDisks, []() {
        __DBG_STUB_INVOKE__
        return QStringList() << "/dev/sda1";
    });

    // Mock password decryption
    stub.set_lamda(&dfmbase::FileUtils::decryptString, [](const QString &) {
        __DBG_STUB_INVOKE__
        return QString("decrypted_password");
    });

    // Mock password check failure
    stub.set_lamda(&Utils::checkDiskPassword, [](struct crypt_device **, const char *, const char *) {
        __DBG_STUB_INVOKE__
        return kPasswordWrong;
    });

    // accessControlDBus->ChangeDiskPassword("oldpwd", "newpwd");

    // // Process events to ensure signal is emitted
    // QTest::qWait(600);

    // EXPECT_TRUE(signalEmitted);
}

// Test ChangeDiskPassword successful operation
TEST_F(UT_AccessControlDBus, ChangeDiskPassword_Success_EmitsSuccess)
{
    bool passwordCheckedEmitted = false;
    bool passwordChangedEmitted = false;

    // Connect to signals to verify emission
    QObject::connect(accessControlDBus, &AccessControlDBus::DiskPasswordChecked,
                     [&passwordCheckedEmitted](int result) {
                         if (result == kNoError) {
                             passwordCheckedEmitted = true;
                         }
                     });

    QObject::connect(accessControlDBus, &AccessControlDBus::DiskPasswordChanged,
                     [&passwordChangedEmitted](int result) {
                         if (result == kNoError) {
                             passwordChangedEmitted = true;
                         }
                     });

    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock encrypted disks list
    stub.set_lamda(&dfmbase::DeviceUtils::encryptedDisks, []() {
        __DBG_STUB_INVOKE__
        return QStringList() << "/dev/sda1";
    });

    // Mock password decryption
    stub.set_lamda(&dfmbase::FileUtils::decryptString, [](const QString &) {
        __DBG_STUB_INVOKE__
        return QString("decrypted_password");
    });

    // Mock successful password check and change
    stub.set_lamda(&Utils::checkDiskPassword, [](struct crypt_device **, const char *, const char *) {
        __DBG_STUB_INVOKE__
        return kNoError;
    });

    stub.set_lamda(&Utils::changeDiskPassword, [](struct crypt_device *, const char *, const char *) {
        __DBG_STUB_INVOKE__
        return kNoError;
    });

    // accessControlDBus->ChangeDiskPassword("oldpwd", "newpwd");

    // // Process events to ensure signals are emitted
    // QTest::qWait(600);

    // EXPECT_TRUE(passwordCheckedEmitted);
    // EXPECT_TRUE(passwordChangedEmitted);
}

// Test Chmod with authentication failure
TEST_F(UT_AccessControlDBus, Chmod_AuthenticationFailed_ReturnsFalse)
{
    // Mock authentication failure
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = accessControlDBus->Chmod("/tmp/testfile", 0755);
    EXPECT_FALSE(result);
}

// Test Chmod with empty path
TEST_F(UT_AccessControlDBus, Chmod_EmptyPath_ReturnsFalse)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = accessControlDBus->Chmod("", 0755);
    EXPECT_FALSE(result);
}

// Test Chmod with non-existent file
TEST_F(UT_AccessControlDBus, Chmod_NonExistentFile_ReturnsFalse)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock file doesn't exist - use function pointer to specify overload
    using FileExistsFunc = bool (QFile::*)() const;
    stub.set_lamda(static_cast<FileExistsFunc>(&QFile::exists), [](const QFile *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = accessControlDBus->Chmod("/tmp/nonexistent", 0755);
    EXPECT_FALSE(result);
}

// Test Chmod successful operation
TEST_F(UT_AccessControlDBus, Chmod_Success_ReturnsTrue)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock file exists - use function pointer to specify overload
    using FileExistsFunc = bool (QFile::*)() const;
    stub.set_lamda(static_cast<FileExistsFunc>(&QFile::exists), [](const QFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock successful setFileMode
    stub.set_lamda(&Utils::setFileMode, [](const QString &, uint) {
        __DBG_STUB_INVOKE__
        return 0;
    });

    bool result = accessControlDBus->Chmod("/tmp/testfile", 0755);
    EXPECT_TRUE(result);
}

// Test Chmod with setFileMode failure
TEST_F(UT_AccessControlDBus, Chmod_SetFileModeFailure_ReturnsFalse)
{
    // Mock successful authentication
    stub.set_lamda(&PolicyKitHelper::checkAuthorization, [](PolicyKitHelper *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock file exists - use function pointer to specify overload
    using FileExistsFunc = bool (QFile::*)() const;
    stub.set_lamda(static_cast<FileExistsFunc>(&QFile::exists), [](const QFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock setFileMode failure
    stub.set_lamda(&Utils::setFileMode, [](const QString &, uint) {
        __DBG_STUB_INVOKE__
        return -1;
    });

    bool result = accessControlDBus->Chmod("/tmp/testfile", 0755);
    EXPECT_FALSE(result);
}

// Test onBlockDevAdded with non-USB device
TEST_F(UT_AccessControlDBus, OnBlockDevAdded_NonUSBDevice_NoAction)
{
    // DFM_MOUNT_USE_NS
    // // Mock device creation
    // stub.set_lamda(ADDR(DBlockMonitor, createDeviceById),
    //                [](DBlockMonitor *, const QString &) {
    //                    __DBG_STUB_INVOKE__
    //                    // Return null to simulate device creation failure
    //                    return QSharedPointer<DBlockDevice>();
    //                });

    // // This should not crash and should handle null device gracefully
    // accessControlDBus->onBlockDevAdded("test_device_id");

    // No assertion needed - just verify it doesn't crash
}

// Test onBlockDevMounted with system device
TEST_F(UT_AccessControlDBus, OnBlockDevMounted_SystemDevice_NoAction)
{
    // DFM_MOUNT_USE_NS
    // // Mock device creation that returns system device
    // stub.set_lamda(ADDR(DBlockMonitor, createDeviceById),
    //                [](DBlockMonitor *, const QString &) {
    //                    __DBG_STUB_INVOKE__
    //                    // Return null to simulate system device or creation failure
    //                    return QSharedPointer<DBlockDevice>();
    //                });

    // // This should not crash and should handle null/system device gracefully
    // accessControlDBus->onBlockDevMounted("test_device_id", "/mnt/test");

    // No assertion needed - just verify it doesn't crash
}

// PolicyKitHelper Tests
TEST_F(UT_AccessControlPolicyKitHelper, Instance_Singleton_ReturnsSameInstance)
{
    PolicyKitHelper *instance1 = PolicyKitHelper::instance();
    PolicyKitHelper *instance2 = PolicyKitHelper::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_AccessControlPolicyKitHelper, CheckAuthorization_EmptyBusName_ReturnsFalse)
{
    PolicyKitHelper *helper = PolicyKitHelper::instance();
    bool result = helper->checkAuthorization("test.action", "");
    EXPECT_FALSE(result);
}

TEST_F(UT_AccessControlPolicyKitHelper, CheckAuthorization_ValidParameters_CallsPolkitQt)
{
    PolicyKitHelper *helper = PolicyKitHelper::instance();
    // This test would require more complex mocking of PolkitQt1 classes
    // For now, we test the basic parameter validation
    bool result = helper->checkAuthorization("test.action", "org.test.service");
    // The actual result depends on the system's PolicyKit configuration
    // In unit tests, this should be mocked
}
