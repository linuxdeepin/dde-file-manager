// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "mastered/masteredmediafilewatcher.h"
#include "mastered/masteredmediafilewatcher_p.h"
#include "utils/opticalhelper.h"
#include "utils/opticalsignalmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <QUrl>
#include <QSignalSpy>
#include <QFutureWatcher>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class TestMasteredMediaFileWatcher : public testing::Test
{
public:
    void SetUp() override
    {
        testUrl = QUrl("burn:///dev/sr0/staging");

        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    QUrl testUrl;
    stub_ext::StubExt stub;
};

TEST_F(TestMasteredMediaFileWatcher, Constructor_ValidUrl_CreatesWatchers)
{
    bool burnDestDeviceCalled = false;
    bool createStagingFolderCalled = false;
    bool localStagingFileCalled = false;
    bool getBlockDeviceIdCalled = false;
    bool queryBlockInfoCalled = false;

    stub.set_lamda(&OpticalHelper::burnDestDevice, [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        burnDestDeviceCalled = true;
        EXPECT_EQ(url, testUrl);
        return QString("/dev/sr0");
    });

    stub.set_lamda(&OpticalHelper::createStagingFolder, [&](const QString &devFile) {
        __DBG_STUB_INVOKE__
        createStagingFolderCalled = true;
        EXPECT_EQ(devFile, "/dev/sr0");
    });

    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile), [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        localStagingFileCalled = true;
        return QUrl::fromLocalFile("/tmp/staging/test");
    });

    stub.set_lamda(&DeviceUtils::getBlockDeviceId, [&](const QString &devFile) {
        __DBG_STUB_INVOKE__
        getBlockDeviceIdCalled = true;
        EXPECT_EQ(devFile, "/dev/sr0");
        return QString("sr0_id");
    });

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [&](DeviceProxyManager *obj, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        queryBlockInfoCalled = true;
        QVariantMap map;
        map[DeviceProperty::kMountPoint] = QString("/media/cdrom");
        map[DeviceProperty::kOpticalBlank] = false;
        return map;
    });

    MasteredMediaFileWatcher watcher(testUrl);

    EXPECT_TRUE(burnDestDeviceCalled);
    EXPECT_TRUE(createStagingFolderCalled);
    EXPECT_TRUE(localStagingFileCalled);
    EXPECT_TRUE(getBlockDeviceIdCalled);
    EXPECT_TRUE(queryBlockInfoCalled);
}

TEST_F(TestMasteredMediaFileWatcher, Constructor_EmptyDeviceId_HandlesGracefully)
{
    bool burnDestDeviceCalled = false;

    stub.set_lamda(&OpticalHelper::burnDestDevice, [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        burnDestDeviceCalled = true;
        return QString();   // Empty device ID
    });

    // Should handle empty device gracefully without crashing
    EXPECT_NO_THROW(MasteredMediaFileWatcher watcher(testUrl));
    EXPECT_TRUE(burnDestDeviceCalled);
}

TEST_F(TestMasteredMediaFileWatcher, Constructor_InvalidStagingUrl_HandlesGracefully)
{
    bool burnDestDeviceCalled = false;
    bool createStagingFolderCalled = false;
    bool localStagingFileCalled = false;

    stub.set_lamda(&OpticalHelper::burnDestDevice, [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        burnDestDeviceCalled = true;
        return QString("/dev/sr0");
    });

    stub.set_lamda(&OpticalHelper::createStagingFolder, [&](const QString &devFile) {
        __DBG_STUB_INVOKE__
        createStagingFolderCalled = true;
    });

    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&OpticalHelper::localStagingFile), [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        localStagingFileCalled = true;
        return QUrl();   // Invalid staging URL
    });

    // Should handle invalid staging URL gracefully
    EXPECT_NO_THROW(MasteredMediaFileWatcher watcher(testUrl));
    EXPECT_TRUE(burnDestDeviceCalled);
    EXPECT_TRUE(createStagingFolderCalled);
    EXPECT_TRUE(localStagingFileCalled);
}

TEST_F(TestMasteredMediaFileWatcher, OnMountPointDeleted_ValidId_EmitsSignals)
{
    // Setup basic mocks for constructor
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(&OpticalHelper::createStagingFolder, [](const QString &devFile) {
        __DBG_STUB_INVOKE__
    });

    using StagingPtr = QUrl (*)(const QUrl &dest);
    stub.set_lamda(static_cast<StagingPtr>(&OpticalHelper::localStagingFile), [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging/test");
    });

    stub.set_lamda(&DeviceUtils::getBlockDeviceId, [](const QString &devFile) {
        __DBG_STUB_INVOKE__
        return QString("sr0_id");
    });

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *obj, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kMountPoint] = QString("/media/cdrom");
        return map;
    });

    stub.set_lamda(&OpticalHelper::transDiscRootById, [](const QString &id) {
        __DBG_STUB_INVOKE__
        return QUrl("burn:///dev/sr0/ondisc");
    });

    stub.set_lamda(&OpticalSignalManager::instance, []() {
        __DBG_STUB_INVOKE__
        static OpticalSignalManager manager;
        return &manager;
    });

    MasteredMediaFileWatcher watcher(testUrl);

    // Use QSignalSpy to capture signal emission
    QSignalSpy spy(&watcher, &MasteredMediaFileWatcher::fileDeleted);

    // Simulate mount point deletion
    QString deviceId = "sr0_id";
    watcher.onMountPointDeleted(deviceId);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toUrl(), QUrl("burn:///dev/sr0/ondisc"));
}

TEST_F(TestMasteredMediaFileWatcher, OnMountPointDeleted_InvalidId_DoesNotEmit)
{
    // Setup basic mocks for constructor
    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(&OpticalHelper::createStagingFolder, [](const QString &devFile) {
        __DBG_STUB_INVOKE__
    });

    using StagingPtr = QUrl (*)(const QUrl &dest);
    stub.set_lamda(static_cast<StagingPtr>(&OpticalHelper::localStagingFile), [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/staging/test");
    });

    stub.set_lamda(&DeviceUtils::getBlockDeviceId, [](const QString &devFile) {
        __DBG_STUB_INVOKE__
        return QString("sr0_id");
    });

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *obj, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kMountPoint] = QString("/media/cdrom");
        return map;
    });

    stub.set_lamda(&OpticalHelper::transDiscRootById, [](const QString &id) {
        __DBG_STUB_INVOKE__
        return QUrl();   // Invalid URL
    });

    MasteredMediaFileWatcher watcher(testUrl);

    // Use QSignalSpy to capture signal emission
    QSignalSpy spy(&watcher, &MasteredMediaFileWatcher::fileDeleted);

    // Simulate mount point deletion with invalid ID
    QString deviceId = "invalid_id";
    watcher.onMountPointDeleted(deviceId);

    EXPECT_EQ(spy.count(), 0);   // Should not emit
}
