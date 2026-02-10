// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

#include "mountcontroldbus.h"
#include "private/mountcontroldbus_p.h"
#include "mounthelpers/abstractmounthelper.h"
#include "service_mountcontrol_global.h"
#include "mounthelpers/cifsmounthelper.h"

#include <QFile>
#include <QDBusConnection>
#include <QVariantMap>

#include <DConfig>

SERVICEMOUNTCONTROL_USE_NAMESPACE
DCORE_USE_NAMESPACE

using RegisterObjectFuncPtr = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);

class MockMountHelper : public AbstractMountHelper
{
public:
    explicit MockMountHelper(QDBusContext *context)
        : AbstractMountHelper(context) { }

    QVariantMap mount(const QString &path, const QVariantMap &opts) override
    {
        mountCalled = true;
        lastMountPath = path;
        lastMountOpts = opts;
        return mockMountResult;
    }

    QVariantMap unmount(const QString &path, const QVariantMap &opts) override
    {
        unmountCalled = true;
        lastUnmountPath = path;
        lastUnmountOpts = opts;
        return mockUnmountResult;
    }

    // Test data
    bool mountCalled = false;
    bool unmountCalled = false;
    QString lastMountPath;
    QString lastUnmountPath;
    QVariantMap lastMountOpts;
    QVariantMap lastUnmountOpts;
    QVariantMap mockMountResult;
    QVariantMap mockUnmountResult;
};

class UT_MountControlDBus : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QFile::exists to return false (disable daemon mount is not present)
        stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [](QFile *) {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Mock DBus connection registration
        stub.set_lamda(static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) {
                           __DBG_STUB_INVOKE__
                           return QDBusConnection::sessionBus();
                       });

        stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Mock DConfig creation to return null (disable cifs mount)
        stub.set_lamda(static_cast<DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&DConfig::create),
                       [](const QString &, const QString &, const QString &, QObject *) -> DConfig * {
                           __DBG_STUB_INVOKE__
                           return nullptr;
                       });

        mountControlDBus = new MountControlDBus("test.service");
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete mountControlDBus;
    }

private:
    stub_ext::StubExt stub;
    MountControlDBus *mountControlDBus = nullptr;

protected:
    MountControlDBus *getDBus() { return mountControlDBus; }

    void addMockHelper(const QString &fsType, MockMountHelper *helper)
    {
        // Access private member through friend mechanism or reflection
        auto *d = mountControlDBus->d.data();
        d->mountHelpers.insert(fsType, helper);
        if (!d->supportedFS.contains(fsType)) {
            d->supportedFS.append(fsType);
        }
    }
};

TEST_F(UT_MountControlDBus, Constructor_DisableDaemonMountExists_DoesNotRegisterDBus)
{
    // Test constructor when disable daemon mount file exists
    stub.clear();

    // Mock QFile::exists to return true (disable daemon mount file exists)
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [](QFile *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool dbusRegistered = false;
    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject), [&dbusRegistered](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
        __DBG_STUB_INVOKE__
        dbusRegistered = true;
        return true;
    });

    MountControlDBus testDBus("test.service");

    EXPECT_FALSE(dbusRegistered);
}

TEST_F(UT_MountControlDBus, Mount_NoFsTypeSpecified_ReturnsError)
{
    // Test Mount with no fsType specified
    QString path = "smb://example.com/share";
    QVariantMap opts;
    // Don't specify fsType
    stub.set_lamda(&MountControlDBus::checkAuthentication, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    QVariantMap result = getDBus()->Mount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kNoFsTypeSpecified);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("fsType field must be specified."));
}

TEST_F(UT_MountControlDBus, Mount_UnsupportedFsType_ReturnsError)
{
    // Test Mount with unsupported fsType
    QString path = "nfs://example.com/share";
    QVariantMap opts;
    opts.insert(MountOptionsField::kFsType, "nfs");

    stub.set_lamda(&MountControlDBus::checkAuthentication, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    QVariantMap result = getDBus()->Mount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kUnsupportedFsTypeOrProtocol);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("current fsType is not supported"));
}

TEST_F(UT_MountControlDBus, Mount_SupportedFsType_CallsHelper)
{
    // Test Mount with supported fsType calls the helper
    QString path = "smb://example.com/share";
    QVariantMap opts;
    opts.insert(MountOptionsField::kFsType, "cifs");

    MockMountHelper *mockHelper = new MockMountHelper(nullptr);
    mockHelper->mockMountResult = {
        { MountReturnField::kResult, true },
        { MountReturnField::kMountPoint, "/media/user/test-mount" },
        { MountReturnField::kErrorCode, 0 }
    };

    addMockHelper("cifs", mockHelper);

    stub.set_lamda(&MountControlDBus::checkAuthentication, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    QVariantMap result = getDBus()->Mount(path, opts);

    EXPECT_TRUE(mockHelper->mountCalled);
    EXPECT_EQ(mockHelper->lastMountPath, path);
    EXPECT_EQ(mockHelper->lastMountOpts.value(MountOptionsField::kFsType).toString(), QString("cifs"));

    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kMountPoint).toString(),
              QString("/media/user/test-mount"));
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), 0);
}

TEST_F(UT_MountControlDBus, Unmount_NoFsTypeSpecified_ReturnsError)
{
    // Test Unmount with no fsType specified
    QString path = "smb://example.com/share";
    QVariantMap opts;
    // Don't specify fsType
    stub.set_lamda(&MountControlDBus::checkAuthentication, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    QVariantMap result = getDBus()->Unmount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kNoFsTypeSpecified);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("fsType field must be specified."));
}

TEST_F(UT_MountControlDBus, Unmount_UnsupportedFsType_ReturnsError)
{
    // Test Unmount with unsupported fsType
    QString path = "nfs://example.com/share";
    QVariantMap opts;
    opts.insert(MountOptionsField::kFsType, "nfs");
    stub.set_lamda(&MountControlDBus::checkAuthentication, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    QVariantMap result = getDBus()->Unmount(path, opts);

    EXPECT_FALSE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), -kUnsupportedFsTypeOrProtocol);
    EXPECT_EQ(result.value(MountReturnField::kErrorMessage).toString(),
              QString("current fsType is not supported"));
}

TEST_F(UT_MountControlDBus, Unmount_SupportedFsType_CallsHelper)
{
    // Test Unmount with supported fsType calls the helper
    QString path = "smb://example.com/share";
    QVariantMap opts;
    opts.insert(MountOptionsField::kFsType, "cifs");

    MockMountHelper *mockHelper = new MockMountHelper(nullptr);
    mockHelper->mockUnmountResult = {
        { MountReturnField::kResult, true },
        { MountReturnField::kErrorCode, 0 }
    };

    addMockHelper("cifs", mockHelper);
    stub.set_lamda(&MountControlDBus::checkAuthentication, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    QVariantMap result = getDBus()->Unmount(path, opts);

    EXPECT_TRUE(mockHelper->unmountCalled);
    EXPECT_EQ(mockHelper->lastUnmountPath, path);
    EXPECT_EQ(mockHelper->lastUnmountOpts.value(MountOptionsField::kFsType).toString(), QString("cifs"));

    EXPECT_TRUE(result.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(result.value(MountReturnField::kErrorCode).toInt(), 0);
}

TEST_F(UT_MountControlDBus, SupportedFileSystems_ReturnsConfiguredSystems)
{
    // Test SupportedFileSystems returns the configured systems

    // Add some mock helpers to test
    MockMountHelper *cifsHelper = new MockMountHelper(nullptr);
    MockMountHelper *dlnfsHelper = new MockMountHelper(nullptr);

    addMockHelper("cifs", cifsHelper);
    addMockHelper("dlnfs", dlnfsHelper);

    QStringList result = getDBus()->SupportedFileSystems();

    EXPECT_TRUE(result.contains("cifs"));
    EXPECT_TRUE(result.contains("dlnfs"));
}

class UT_MountControlDBusPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QFile::exists to return false
        stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [](QFile *) {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Mock DBus connection registration
        stub.set_lamda(static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) {
                           __DBG_STUB_INVOKE__
                           return QDBusConnection::sessionBus();
                       });

        stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
            __DBG_STUB_INVOKE__
            return true;
        });

        mountControlDBus = new MountControlDBus("test.service");
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete mountControlDBus;
    }

private:
    stub_ext::StubExt stub;
    MountControlDBus *mountControlDBus = nullptr;

protected:
    MountControlDBus *getDBus() { return mountControlDBus; }
};

TEST_F(UT_MountControlDBusPrivate, Constructor_CifsDisabled_DoesNotAddCifsHelper)
{
    // Test constructor when CIFS mount is disabled
    stub.clear();

    // Mock QFile::exists to return false
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [](QFile *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock DBus connection registration
    stub.set_lamda(static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                   [](QDBusConnection::BusType, const QString &) {
                       __DBG_STUB_INVOKE__
                       return QDBusConnection::sessionBus();
                   });

    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock DConfig creation to return null (disable cifs mount)
    stub.set_lamda(static_cast<DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&DConfig::create),
                   [](const QString &, const QString &, const QString &, QObject *) -> DConfig * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    MountControlDBus testDBus("test.service");
    QStringList supportedFS = testDBus.SupportedFileSystems();

    EXPECT_FALSE(supportedFS.contains("cifs"));
    EXPECT_TRUE(supportedFS.contains("dlnfs"));   // Should still have dlnfs
}

TEST_F(UT_MountControlDBusPrivate, Constructor_CifsEnabled_AddsCifsHelper)
{
    // Test constructor when CIFS mount is enabled
    stub.clear();

    // Mock QFile::exists to return false
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [](QFile *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock DBus connection registration
    stub.set_lamda(static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                   [](QDBusConnection::BusType, const QString &) {
                       __DBG_STUB_INVOKE__
                       return QDBusConnection::sessionBus();
                   });

    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Create mock DConfig that enables CIFS mount
    static DConfig mockConfig("");
    stub.set_lamda(static_cast<DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&DConfig::create),
                   [](const QString &, const QString &, const QString &, QObject *) -> DConfig * {
                       __DBG_STUB_INVOKE__
                       return &mockConfig;
                   });

    stub.set_lamda(&DConfig::value, [](DConfig *, const QString &key, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == "enableCifsMount")
            return QVariant(true);
        return QVariant();
    });

    stub.set_lamda(static_cast<void (QObject::*)()>(&DConfig::deleteLater), [](QObject *) {
        __DBG_STUB_INVOKE__
    });

    // Mock CifsMountHelper::cleanMountPoint
    stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {
        __DBG_STUB_INVOKE__
    });

    MountControlDBus testDBus("test.service");
    QStringList supportedFS = testDBus.SupportedFileSystems();

    EXPECT_TRUE(supportedFS.contains("cifs"));
    EXPECT_TRUE(supportedFS.contains("dlnfs"));
}
