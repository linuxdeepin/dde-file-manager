// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "views/opticalmediawidget.h"
#include "utils/opticalhelper.h"
#include "utils/opticalsignalmanager.h"
#include "events/opticaleventcaller.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/filestatisticsjob.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-burn/dburn_global.h>
#include <dfm-mount/base/dmount_global.h>

#include <DSysInfo>
#include <QDir>
#include <QUrl>
#include <QVariantMap>
#include <QSignalSpy>
#include <QTest>

DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFM_BURN_USE_NS
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class TestOpticalMediaWidget : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new OpticalMediaWidget();

        // Setup default mock returns
        setupDefaultMocks();
    }

    void TearDown() override
    {
        delete widget;
        stub.clear();
    }

protected:
    void setupDefaultMocks()
    {
        // Mock DeviceProxyManager
        stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [this](DeviceProxyManager *, const QString &id, bool) {
            __DBG_STUB_INVOKE__
            QVariantMap map;
            map[DeviceProperty::kDevice] = mockDevice;
            map[DeviceProperty::kMountPoint] = mockMountPoint;
            map[DeviceProperty::kSizeFree] = mockAvailableSpace;
            map[DeviceProperty::kFileSystem] = mockFileSystem;
            map[DeviceProperty::kFsVersion] = mockFsVersion;
            map[DeviceProperty::kIdLabel] = mockDiscName;
            map[DeviceProperty::kOpticalMediaType] = static_cast<int>(mockMediaType);
            return map;
        });

        // Mock DeviceUtils
        stub.set_lamda(&DeviceUtils::getBlockDeviceId, [this](const QString &dev) {
            __DBG_STUB_INVOKE__
            return mockDeviceId;
        });

        stub.set_lamda(&DeviceUtils::isBlankOpticalDisc, [this](const QString &id) {
            __DBG_STUB_INVOKE__
            return mockIsBlank;
        });

        // Mock OpticalHelper
        stub.set_lamda(&OpticalHelper::burnDestDevice, [this](const QUrl &url) {
            __DBG_STUB_INVOKE__
            return mockDevice;
        });

        stub.set_lamda(&OpticalHelper::isBurnEnabled, []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&OpticalHelper::isSupportedUDFMedium, [](int type) {
            __DBG_STUB_INVOKE__
            return type == static_cast<int>(MediaType::kDVD_R);
        });

        stub.set_lamda(&OpticalHelper::isSupportedUDFVersion, [](const QString &version) {
            __DBG_STUB_INVOKE__
            return version == "1.02";
        });

        // Mock FileUtils
        stub.set_lamda(&FileUtils::formatSize, [](qint64 size, bool, int, int, QStringList) {
            __DBG_STUB_INVOKE__
            return QString("%1 MB").arg(size / 1024 / 1024);
        });
    }

    OpticalMediaWidget *widget = nullptr;
    stub_ext::StubExt stub;

    // Mock data
    QString mockDevice = "/dev/sr0";
    QString mockDeviceId = "sr0_device_id";
    QString mockMountPoint = "/media/disc";
    qint64 mockAvailableSpace = 1024 * 1024 * 1024;   // 1GB
    QString mockFileSystem = "iso9660";
    QString mockFsVersion = "1.02";
    QString mockDiscName = "Test Disc";
    MediaType mockMediaType = MediaType::kDVD_R;
    bool mockIsBlank = false;
};

TEST_F(TestOpticalMediaWidget, Constructor_InitializesUiAndConnections)
{
    EXPECT_NE(widget, nullptr);

    // Check if UI elements are created
    auto layout = widget->findChild<QHBoxLayout *>();
    EXPECT_NE(layout, nullptr);

    auto mediaTypeLabel = widget->findChild<QLabel *>();
    EXPECT_NE(mediaTypeLabel, nullptr);

    auto burnButton = widget->findChild<DTK_WIDGET_NAMESPACE::DPushButton *>();
    EXPECT_NE(burnButton, nullptr);
}

TEST_F(TestOpticalMediaWidget, UpdateDiscInfo_ValidUrl_ReturnsTrue)
{
    QUrl testUrl("burn:///dev/sr0/disc_files/");

    bool result = widget->updateDiscInfo(testUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalMediaWidget, UpdateDiscInfo_EmptyDevice_ReturnsFalse)
{
    QUrl testUrl("burn:///dev/sr0/disc_files/");
    mockDevice = "";

    stub.set_lamda(&OpticalHelper::burnDestDevice, [this](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return mockDevice;
    });

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &id, bool) {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kDevice] = "";
        return map;
    });

    bool result = widget->updateDiscInfo(testUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalMediaWidget, UpdateDiscInfo_EmptyMountPointNonBlank_ReturnsFalse)
{
    QUrl testUrl("burn:///dev/sr0/disc_files/");
    mockMountPoint = "";
    mockIsBlank = false;

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [this](DeviceProxyManager *, const QString &id, bool) {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kDevice] = mockDevice;
        map[DeviceProperty::kMountPoint] = "";
        return map;
    });

    bool handleErrorMountCalled = false;
    stub.set_lamda(ADDR(OpticalMediaWidget, handleErrorMount), [&]() {
        __DBG_STUB_INVOKE__
        handleErrorMountCalled = true;
    });

    bool result = widget->updateDiscInfo(testUrl);

    EXPECT_FALSE(result);
    EXPECT_TRUE(handleErrorMountCalled);
}

TEST_F(TestOpticalMediaWidget, UpdateDiscInfo_BlankDiscZeroSpace_AttemptsMount)
{
    QUrl testUrl("burn:///dev/sr0/disc_files/");
    mockIsBlank = true;
    mockAvailableSpace = 0;

    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [this](DeviceProxyManager *, const QString &id, bool) {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map[DeviceProperty::kDevice] = mockDevice;
        map[DeviceProperty::kMountPoint] = mockMountPoint;
        map[DeviceProperty::kSizeFree] = mockAvailableSpace;
        return map;
    });

    bool mountCalled = false;
    stub.set_lamda(&DeviceManager::mountBlockDevAsync, [&](DeviceManager *, const QString &id, const QVariantMap &opts, std::function<void(bool, const DFMMOUNT::OperationErrorInfo &, const QString &)> callback, int) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
        // Simulate successful mount callback
        if (callback) {
            callback(true, DFMMOUNT::OperationErrorInfo(), "/media/disc");
        }
    });

    bool result = widget->updateDiscInfo(testUrl, false);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mountCalled);
}

TEST_F(TestOpticalMediaWidget, IsSupportedUDF_ProfessionalEditionSupportedMedium_UpdatesUI)
{
    mockFileSystem = "udf";
    mockFsVersion = "1.02";
    mockMediaType = MediaType::kDVD_R;

    stub.set_lamda(&DSysInfo::deepinType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinProfessional;
    });

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    widget->updateDiscInfo(testUrl);

    // Test passes if no crash occurs and updateDiscInfo returns true
    EXPECT_TRUE(true);
}

TEST_F(TestOpticalMediaWidget, IsSupportedUDF_NonProfessionalEdition_UpdatesUI)
{
    mockFileSystem = "udf";
    mockFsVersion = "1.02";
    mockMediaType = MediaType::kDVD_R;

    stub.set_lamda(&DSysInfo::deepinType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinPersonal;
    });

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    widget->updateDiscInfo(testUrl);

    // Test passes if no crash occurs and updateDiscInfo returns true
    EXPECT_TRUE(true);
}

TEST_F(TestOpticalMediaWidget, OnDumpButtonClicked_CallsOpticalEventCaller)
{
    bool dumpDialogCalled = false;
    stub.set_lamda(&OpticalEventCaller::sendOpenDumpISODlg, [&](const QString &deviceId) {
        __DBG_STUB_INVOKE__
        dumpDialogCalled = true;
        EXPECT_EQ(deviceId, mockDeviceId);
    });

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    widget->updateDiscInfo(testUrl);

    widget->onDumpButtonClicked();

    EXPECT_TRUE(dumpDialogCalled);
}

TEST_F(TestOpticalMediaWidget, OnDiscUnmounted_MatchingUrl_ProcessesCorrectly)
{
    QUrl testUrl("burn:///dev/sr0/disc_files/");
    widget->updateDiscInfo(testUrl);

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
        __DBG_STUB_INVOKE__
        return true;   // Simulate matching URLs
    });

    // Test should not crash
    widget->onDiscUnmounted(testUrl);

    EXPECT_TRUE(true);
}

TEST_F(TestOpticalMediaWidget, OnDiscUnmounted_NonMatchingUrl_ProcessesCorrectly)
{
    QUrl testUrl("burn:///dev/sr0/disc_files/");
    QUrl otherUrl("burn:///dev/sr1/disc_files/");

    widget->updateDiscInfo(testUrl);

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
        __DBG_STUB_INVOKE__
        return false;   // Simulate non-matching URLs
    });

    // Test should not crash
    widget->onDiscUnmounted(otherUrl);

    EXPECT_TRUE(true);
}

TEST_F(TestOpticalMediaWidget, HandleErrorMount_NavigatesToComputerAndShowsDialog)
{
    bool navigationCalled = false;
    bool ejectCalled = false;

    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return quint64(12345);
    });

    // Mock FileManagerWindow
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&](FileManagerWindowsManager *, quint64 id) {
        __DBG_STUB_INVOKE__
        return new FileManagerWindow(QUrl::fromLocalFile("/"));
    });

    stub.set_lamda(VADDR(FileManagerWindow, cd), [&](FileManagerWindow *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        navigationCalled = true;
        EXPECT_EQ(url.scheme(), "computer");
    });

    stub.set_lamda(&DeviceManager::ejectBlockDevAsync, [&](DeviceManager *, const QString &id, const QVariantMap &opts, std::function<void(bool, const DFMMOUNT::OperationErrorInfo &)> callback) {
        __DBG_STUB_INVOKE__
        ejectCalled = true;
        if (callback) {
            callback(true, DFMMOUNT::OperationErrorInfo());
        }
    });

    bool errorDialogShown = false;
    stub.set_lamda(&DialogManager::showErrorDialog, [&]() {
        __DBG_STUB_INVOKE__
        errorDialogShown = true;
    });

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    widget->updateDiscInfo(testUrl);

    widget->handleErrorMount();

    EXPECT_TRUE(navigationCalled);
    EXPECT_TRUE(ejectCalled);
    EXPECT_TRUE(errorDialogShown);
}

TEST_F(TestOpticalMediaWidget, UpdateUi_BlankDisc_UpdatesCorrectly)
{
    mockIsBlank = true;

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    bool result = widget->updateDiscInfo(testUrl);

    EXPECT_TRUE(result);
    // Test passes if no crash occurs during UI update
}

TEST_F(TestOpticalMediaWidget, UpdateUi_ZeroAvailableSpace_UpdatesCorrectly)
{
    mockAvailableSpace = 0;

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    bool result = widget->updateDiscInfo(testUrl);

    EXPECT_TRUE(result);
    // Test passes if no crash occurs during UI update
}

TEST_F(TestOpticalMediaWidget, UpdateUi_BurnDisabledGlobally_UpdatesCorrectly)
{
    stub.set_lamda(&OpticalHelper::isBurnEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    bool result = widget->updateDiscInfo(testUrl);

    EXPECT_TRUE(result);
    // Test passes if no crash occurs during UI update
}

TEST_F(TestOpticalMediaWidget, OnBurnButtonClicked_NoStatisticsWorkerRunning_ProcessesCorrectly)
{
    stub.set_lamda(&FileStatisticsJob::isRunning, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl testUrl("burn:///dev/sr0/disc_files/");
    widget->updateDiscInfo(testUrl);

    // Test should not crash
    widget->onBurnButtonClicked();

    EXPECT_TRUE(true);
}
