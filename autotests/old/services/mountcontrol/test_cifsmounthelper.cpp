// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

#include "mounthelpers/cifsmounthelper.h"
#include "mounthelpers/cifsmounthelper_p.h"
#include "service_mountcontrol_global.h"

#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QVariantMap>
#include <QUrl>
#include <QDir>

#include <DConfig>

#include <libmount/libmount.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>

SERVICEMOUNTCONTROL_USE_NAMESPACE

class MockDBusContext : public QDBusContext
{
public:
    QDBusMessage mockMessage;

    MockDBusContext()
    {
        // Create a method call message that simulates a D-Bus call
        mockMessage = QDBusMessage::createMethodCall("com.test.service", "/test/path", "com.test.interface", "testMethod");
        // Note: We can't directly set the service on the message, but we'll handle this in the stub
    }

    QDBusMessage message() const
    {
        return mockMessage;
    }

    // Override to provide a service name
    QString service() const
    {
        return "com.test.service";
    }
};

class UT_CifsMountHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        mockContext = new MockDBusContext();
        helper = new CifsMountHelper(mockContext);

        // Set up common stubs that are needed for most tests
        // Mock QDBusMessage::service() to return a valid service name
        stub.set_lamda(&QDBusMessage::service, [](QDBusMessage *) {
            __DBG_STUB_INVOKE__
            return QString("com.test.service");
        });

        // Mock QDBusConnection operations for invokerUid()
        stub.set_lamda(static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) {
                           __DBG_STUB_INVOKE__
                           return QDBusConnection::sessionBus();
                       });

        stub.set_lamda(&QDBusConnection::isConnected, [](QDBusConnection *) {
            __DBG_STUB_INVOKE__
            return true;
        });

        // stub.set_lamda(&QDBusConnection::interface, [](QDBusConnection *) {
        //     __DBG_STUB_INVOKE__
        //     return QDBusConnection::sessionBus().interface();
        // });

        stub.set_lamda(&QDBusConnectionInterface::serviceUid, [](QDBusConnectionInterface *, const QString &) -> QDBusReply<uint> {
            __DBG_STUB_INVOKE__
            QDBusMessage call = QDBusMessage::createMethodCall("", "", "", "");
            QDBusMessage reply = call.createReply(QVariantList() << 1000u);
            return QDBusReply<uint>(reply);
        });
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete helper;
        delete mockContext;
    }

private:
    stub_ext::StubExt stub;
    CifsMountHelper *helper = nullptr;
    MockDBusContext *mockContext = nullptr;

protected:
    CifsMountHelper *getHelper() { return helper; }
    MockDBusContext *getContext() { return mockContext; }
};

TEST_F(UT_CifsMountHelper, Mount_NonSmbScheme_ReturnsError)
{
    // Test mount with non-smb scheme
    QString path = "ftp://example.com/share";
    QVariantMap opts;

    QVariantMap result = getHelper()->mount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kNotSupportedScheme);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("smb is only supported scheme now"));
    EXPECT_TRUE(result.value(MountReturnField::kMountPoint).toString().isEmpty());
}

TEST_F(UT_CifsMountHelper, Mount_AlreadyMounted_ReturnsSuccess)
{
    // Test mount when already mounted
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    // Mock checkMount to return already mounted
    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &mpt) {
        __DBG_STUB_INVOKE__
        mpt = "/media/user/smbmounts/existing-mount";
        return CifsMountHelper::kAlreadyMounted;
    });

    QVariantMap result = getHelper()->mount(path, opts);

    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), 0);
    EXPECT_EQ(result.value(MountReturnField::kMountPoint).toString(),
              QString("/media/user/smbmounts/existing-mount"));
}

TEST_F(UT_CifsMountHelper, Mount_CannotGenerateMountPath_ReturnsError)
{
    // Test mount when cannot generate mount path
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    // Mock checkMount to return not exist
    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &) {
        __DBG_STUB_INVOKE__
        return CifsMountHelper::kNotExist;
    });

    // Mock generateMountPath to return empty
    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return QString();
    });

    QVariantMap result = getHelper()->mount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kCannotGenerateMountPath);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("cannot generate mount point"));
}

TEST_F(UT_CifsMountHelper, Mount_CannotMkdir_ReturnsError)
{
    // Test mount when mkdir fails
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &) {
        __DBG_STUB_INVOKE__
        return CifsMountHelper::kNotExist;
    });

    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return QString("/media/user/smbmounts/test-mount");
    });

    // Mock mkdir to fail
    stub.set_lamda(&CifsMountHelper::mkdir, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QVariantMap result = getHelper()->mount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kCannotMkdirMountPoint);
    EXPECT_TRUE(result.value(MountReturnField::kErrorMessage).toString().contains("cannot create mount point"));
}

TEST_F(UT_CifsMountHelper, Mount_SuccessfulMount_ReturnsSuccess)
{
    // Test successful mount
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;
    opts.insert(MountOptionsField::kUser, "testuser");
    opts.insert(MountOptionsField::kPasswd, "dGVzdHBhc3M=");   // base64 encoded "testpass"

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &) {
        __DBG_STUB_INVOKE__
        return CifsMountHelper::kNotExist;
    });

    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return QString("/media/user/smbmounts/test-mount");
    });

    stub.set_lamda(&CifsMountHelper::mkdir, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock system mount call to succeed
    stub.set_lamda(mount, [](const char *, const char *, const char *, unsigned long, const void *) -> int {
        __DBG_STUB_INVOKE__
        return 0;   // Success
    });

    // Mock private helper methods
    stub.set_lamda(&CifsMountHelperPrivate::parseIP, [](CifsMountHelperPrivate *, const QString &, uint16_t) {
        __DBG_STUB_INVOKE__
        return QString("192.168.1.100");
    });

    stub.set_lamda(&CifsMountHelperPrivate::probeVersion, [](CifsMountHelperPrivate *, const QString &, ushort) {
        __DBG_STUB_INVOKE__
        return QString("3.0");
    });
    stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> const QDBusMessage & {
        __DBG_STUB_INVOKE__
        // Create a static message to ensure it has valid service data
        static QDBusMessage msg = QDBusMessage::createMethodCall("org.test.service", "/test", "org.test.Interface", "testMethod");
        return msg;
    });
    QVariantMap result = getHelper()->mount(path, opts);

    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), 0);
    EXPECT_EQ(result.value(MountReturnField::kMountPoint).toString(),
              QString("/media/user/smbmounts/test-mount"));
}

TEST_F(UT_CifsMountHelper, Mount_MountFailed_ReturnsError)
{
    // Test mount when system mount fails
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &) {
        __DBG_STUB_INVOKE__
        return CifsMountHelper::kNotExist;
    });

    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return QString("/media/user/smbmounts/test-mount");
    });

    stub.set_lamda(&CifsMountHelper::mkdir, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&CifsMountHelper::rmdir, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock system mount call to fail
    stub.set_lamda(mount, [](const char *, const char *, const char *, unsigned long, const void *) -> int {
        __DBG_STUB_INVOKE__
        errno = EACCES;   // Permission denied
        return -1;   // Failure
    });

    stub.set_lamda(&CifsMountHelperPrivate::parseIP, [](CifsMountHelperPrivate *, const QString &, uint16_t) {
        __DBG_STUB_INVOKE__
        return QString("192.168.1.100");
    });

    stub.set_lamda(&CifsMountHelperPrivate::probeVersion, [](CifsMountHelperPrivate *, const QString &, ushort) {
        __DBG_STUB_INVOKE__
        return QString("3.0");
    });

    stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> const QDBusMessage & {
        __DBG_STUB_INVOKE__
        // Create a static message to ensure it has valid service data
        static QDBusMessage msg = QDBusMessage::createMethodCall("org.test.service", "/test", "org.test.Interface", "testMethod");
        return msg;
    });

    QVariantMap result = getHelper()->mount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), EACCES);
    EXPECT_TRUE(result.value(MountReturnField::kErrorMessage).toString().contains("Permission denied"));
}

TEST_F(UT_CifsMountHelper, Unmount_MountNotExist_ReturnsError)
{
    // Test unmount when mount doesn't exist
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &) {
        __DBG_STUB_INVOKE__
        return CifsMountHelper::kNotExist;
    });

    QVariantMap result = getHelper()->unmount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kMountNotExist);
    EXPECT_TRUE(result.value(MountReturnField::kErrorMessage).toString().contains("is not mounted"));
}

TEST_F(UT_CifsMountHelper, Unmount_NotOwnerWithoutAuth_ReturnsError)
{
    // Test unmount when not owner - the method directly returns error without separate auth check
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &) {
        __DBG_STUB_INVOKE__
        return CifsMountHelper::kNotOwner;
    });

    QVariantMap result = getHelper()->unmount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kNotOwnerOfMount);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("invoker is not the owner of mount"));
}

TEST_F(UT_CifsMountHelper, Unmount_SuccessfulUnmount_ReturnsSuccess)
{
    // Test successful unmount
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &mpt) {
        __DBG_STUB_INVOKE__
        mpt = "/media/user/smbmounts/test-mount";
        return CifsMountHelper::kOkay;
    });

    // Mock successful umount
    stub.set_lamda(umount, [](const char *) -> int {
        __DBG_STUB_INVOKE__
        errno = 0;
        return 0;   // Success
    });

    stub.set_lamda(&CifsMountHelper::rmdir, [](CifsMountHelper *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QVariantMap result = getHelper()->unmount(path, opts);

    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), 0);
}

TEST_F(UT_CifsMountHelper, Unmount_UmountFailed_ReturnsError)
{
    // Test unmount when umount system call fails
    QString path = "smb://192.168.1.100/share";
    QVariantMap opts;

    stub.set_lamda(&CifsMountHelper::checkMount, [](CifsMountHelper *, const QString &, QString &mpt) {
        __DBG_STUB_INVOKE__
        mpt = "/media/user/smbmounts/test-mount";
        return CifsMountHelper::kOkay;
    });

    // Mock failed umount
    stub.set_lamda(umount, [](const char *) -> int {
        __DBG_STUB_INVOKE__
        errno = EBUSY;   // Device busy
        return -1;   // Failure
    });

    QVariantMap result = getHelper()->unmount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), EBUSY);
    EXPECT_TRUE(result.value(MountReturnField::kErrorMessage).toString().contains("Device or resource busy"));
}

TEST_F(UT_CifsMountHelper, GenerateMountPath_ValidAddress_ReturnsPath)
{
    // Test generateMountPath with valid SMB address
    QString address = "smb://192.168.1.100/share";

    // Mock cleanMountPoint
    stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {
        __DBG_STUB_INVOKE__
    });

    // Mock mkdirMountRootPath to succeed
    stub.set_lamda(&CifsMountHelper::mkdirMountRootPath, [](CifsMountHelper *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock mountRoot
    stub.set_lamda(&CifsMountHelper::mountRoot, [](CifsMountHelper *) {
        __DBG_STUB_INVOKE__
        return QString("/media/testuser/smbmounts");
    });

    // Mock QDir::exists to return false (path doesn't exist)
    stub.set_lamda(static_cast<bool (QDir::*)(const QString &) const>(&QDir::exists), [](QDir *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QString result = getHelper()->generateMountPath(address);

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("smb-share:server=192.168.1.100,share=share"));
}

TEST_F(UT_CifsMountHelper, InvokerUid_ValidDBusContext_ReturnsUid)
{
    // Test getting invoker UID from DBus context
    // The DBus mocking is already set up in SetUp()
    stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> const QDBusMessage & {
        __DBG_STUB_INVOKE__
        // Create a static message to ensure it has valid service data
        static QDBusMessage msg = QDBusMessage::createMethodCall("org.test.service", "/test", "org.test.Interface", "testMethod");
        return msg;
    });

    uint result = getHelper()->invokerUid();

    EXPECT_EQ(result, 1000u);
}

TEST_F(UT_CifsMountHelper, MountRoot_ValidUser_ReturnsPath)
{
    // Test mountRoot with valid user

    // Mock getpwuid to return valid user
    stub.set_lamda(
            getpwuid, [](uid_t) -> struct passwd * {
                __DBG_STUB_INVOKE__
                static struct passwd pw;
                static char name[] = "testuser";
                pw.pw_name = name;
                pw.pw_uid = 1000;
                pw.pw_gid = 1000;
                return &pw;
            });

    stub.set_lamda(&CifsMountHelper::invokerUid, [](CifsMountHelper *) {
        __DBG_STUB_INVOKE__
        return 1000u;
    });

    QString result = getHelper()->mountRoot();

    EXPECT_EQ(result, QString("/media/testuser/smbmounts"));
}

TEST_F(UT_CifsMountHelper, MountRoot_InvalidUser_ReturnsEmpty)
{
    // Test mountRoot with invalid user

    // Mock getpwuid to return null
    stub.set_lamda(
            getpwuid, [](uid_t) -> struct passwd * {
                __DBG_STUB_INVOKE__
                return nullptr;
            });

    stub.set_lamda(&CifsMountHelper::invokerUid, [](CifsMountHelper *) {
        __DBG_STUB_INVOKE__
        return 1000u;
    });

    QString result = getHelper()->mountRoot();

    EXPECT_TRUE(result.isEmpty());
}
