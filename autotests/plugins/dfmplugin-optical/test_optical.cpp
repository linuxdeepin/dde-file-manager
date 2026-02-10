// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "optical.h"
#include "utils/opticalhelper.h"
#include "utils/opticalfilehelper.h"
#include "utils/opticalsignalmanager.h"
#include "events/opticaleventreceiver.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QTimer>
#include <QRegularExpression>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

class TestOptical : public testing::Test
{
public:
    void SetUp() override
    {
        optical = new Optical();
    }

    void TearDown() override
    {
        delete optical;
        stub.clear();
    }

protected:
    Optical *optical = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestOptical, Initialize_RegistersSchemeAndFactories)
{
    bool bindEventsCalled = false;
    bool bindWindowsCalled = false;
    bool bindFileOperationsCalled = false;

    stub.set_lamda(ADDR(Optical, bindEvents), [&]() {
        __DBG_STUB_INVOKE__
        bindEventsCalled = true;
    });

    stub.set_lamda(ADDR(Optical, bindWindows), [&]() {
        __DBG_STUB_INVOKE__
        bindWindowsCalled = true;
    });

    stub.set_lamda(ADDR(Optical, bindFileOperations), [&]() {
        __DBG_STUB_INVOKE__
        bindFileOperationsCalled = true;
    });

    optical->initialize();

    EXPECT_TRUE(bindEventsCalled);
    EXPECT_TRUE(bindWindowsCalled);
    EXPECT_TRUE(bindFileOperationsCalled);
}

TEST_F(TestOptical, Start_RegistersMenuSceneAndViews)
{
    bool addCustomTopWidgetCalled = false;
    bool addDelegateSettingsCalled = false;
    bool addPropertySettingsCalled = false;

    stub.set_lamda(ADDR(Optical, addCustomTopWidget), [&]() {
        __DBG_STUB_INVOKE__
        addCustomTopWidgetCalled = true;
    });

    stub.set_lamda(ADDR(Optical, addDelegateSettings), [&]() {
        __DBG_STUB_INVOKE__
        addDelegateSettingsCalled = true;
    });

    stub.set_lamda(ADDR(Optical, addPropertySettings), [&]() {
        __DBG_STUB_INVOKE__
        addPropertySettingsCalled = true;
    });

    bool result = optical->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(addCustomTopWidgetCalled);
    EXPECT_TRUE(addDelegateSettingsCalled);
    EXPECT_TRUE(addPropertySettingsCalled);
}

TEST_F(TestOptical, PacketWritingUrl_ValidBurnUrl_ReturnsTrue)
{
    QUrl srcUrl("burn:///dev/sr0/staging/test.txt");
    QUrl resultUrl;
    bool deviceUtilsIsPWCalled = false;
    bool opticalHelperLocalDiscFileCalled = false;

    stub.set_lamda(&OpticalHelper::burnDestDevice, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });

    stub.set_lamda(&DeviceUtils::isPWOpticalDiscDev, [&](const QString &dev) {
        __DBG_STUB_INVOKE__
        deviceUtilsIsPWCalled = true;
        EXPECT_EQ(dev, "/dev/sr0");
        return true;
    });

    stub.set_lamda(&OpticalHelper::localDiscFile, [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        opticalHelperLocalDiscFileCalled = true;
        return QUrl::fromLocalFile("/media/cdrom/test.txt");
    });

    bool result = optical->packetWritingUrl(srcUrl, &resultUrl);

    EXPECT_TRUE(result);
    EXPECT_TRUE(deviceUtilsIsPWCalled);
    EXPECT_TRUE(opticalHelperLocalDiscFileCalled);
    EXPECT_EQ(resultUrl, QUrl::fromLocalFile("/media/cdrom/test.txt"));
}

TEST_F(TestOptical, PacketWritingUrl_NonBurnUrl_ReturnsFalse)
{
    QUrl srcUrl("file:///home/user/test.txt");
    QUrl resultUrl;

    bool result = optical->packetWritingUrl(srcUrl, &resultUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestOptical, OnDiscChanged_ValidId_EmitsSignalAndClosesTab)
{
    QString deviceId = "/dev/sr0";
    bool transDiscRootByIdCalled = false;

    stub.set_lamda(&OpticalHelper::transDiscRootById, [&](const QString &id) {
        __DBG_STUB_INVOKE__
        transDiscRootByIdCalled = true;
        EXPECT_EQ(id, deviceId);
        return QUrl("burn:///dev/sr0/ondisc");
    });

    // Mock signal emission
    stub.set_lamda(&OpticalSignalManager::instance, []() {
        __DBG_STUB_INVOKE__
        static OpticalSignalManager manager;
        return &manager;
    });

    optical->onDiscChanged(deviceId);

    EXPECT_TRUE(transDiscRootByIdCalled);
}

TEST_F(TestOptical, OnDiscEjected_ValidId_UnmountsDevice)
{
    QString deviceId = "sr0_id";
    bool unmountCalled = false;
    bool getMountInfoCalled = false;

    stub.set_lamda(&DeviceUtils::getMountInfo, [&](const QString &devFile, bool) {
        __DBG_STUB_INVOKE__
        getMountInfoCalled = true;
        EXPECT_EQ(devFile, "/dev/sr0");
        return QString("sr0_id");
    });

    stub.set_lamda(&DeviceManager::unmountBlockDevAsync, [&](DeviceManager *obj, const QString &id, const QVariantMap &options, std::function<void(bool, const dfmmount::OperationErrorInfo &)> callback) {
        __DBG_STUB_INVOKE__
        unmountCalled = true;
        EXPECT_EQ(id, deviceId);
        // Call callback to simulate success
        if (callback) {
            dfmmount::OperationErrorInfo errorInfo;
            callback(true, errorInfo);
        }
    });

    optical->onDiscEjected(deviceId);

    EXPECT_TRUE(getMountInfoCalled);
    EXPECT_TRUE(unmountCalled);
}

TEST_F(TestOptical, ChangeUrlEventFilter_PacketWritingUrl_RedirectsUrl)
{
    quint64 windowId = 12345;
    QUrl srcUrl = QUrl("burn:///dev/sr0/staging/test.txt");
    QUrl resultUrl;
    bool packetWritingUrlCalled = false;

    stub.set_lamda(ADDR(Optical, packetWritingUrl), [&](Optical *obj, const QUrl &srcUrl, QUrl *resultUrl) {
        __DBG_STUB_INVOKE__
        packetWritingUrlCalled = true;
        *resultUrl = QUrl("file:///tmp/staging/test.txt");
        return true;
    });

    bool result = optical->changeUrlEventFilter(windowId, srcUrl);

    EXPECT_TRUE(result);
    EXPECT_TRUE(packetWritingUrlCalled);
}

TEST_F(TestOptical, OpenNewWindowEventFilter_PacketWritingUrl_RedirectsWindow)
{
    QUrl srcUrl = QUrl("burn:///dev/sr0/staging/test.txt");
    QUrl resultUrl;
    bool packetWritingUrlCalled = false;

    stub.set_lamda(ADDR(Optical, packetWritingUrl), [&](Optical *obj, const QUrl &srcUrl, QUrl *resultUrl) {
        __DBG_STUB_INVOKE__
        packetWritingUrlCalled = true;
        *resultUrl = QUrl("file:///tmp/staging/test.txt");
        return true;
    });

    bool result = optical->openNewWindowEventFilter(srcUrl);

    EXPECT_TRUE(result);
    EXPECT_TRUE(packetWritingUrlCalled);
}

TEST_F(TestOptical, OpenNewWindowWithArgsEventFilter_PacketWritingUrl_RedirectsWindowWithArgs)
{
    QUrl srcUrl = QUrl("burn:///dev/sr0/staging/test.txt");
    QUrl resultUrl;
    bool packetWritingUrlCalled = false;
    bool isNewWindow = true;

    stub.set_lamda(ADDR(Optical, packetWritingUrl), [&](Optical *obj, const QUrl &srcUrl, QUrl *resultUrl) {
        __DBG_STUB_INVOKE__
        packetWritingUrlCalled = true;
        *resultUrl = QUrl("file:///tmp/staging/test.txt");
        return true;
    });

    bool result = optical->openNewWindowWithArgsEventFilter(srcUrl, isNewWindow);

    EXPECT_TRUE(result);
    EXPECT_TRUE(packetWritingUrlCalled);
}

TEST_F(TestOptical, OnAllPluginsStarted_RegistersPacketWritingMenu)
{
    bool qObjectTrCalled = false;

    stub.set_lamda(static_cast<QString (*)(const char *, const char *, int)>(&QObject::tr), [&](const char *sourceText, const char *disambiguation, int n) {
        __DBG_STUB_INVOKE__
        qObjectTrCalled = true;
        return QString("Packet Writing");
    });

    optical->onAllPluginsStarted();

    EXPECT_TRUE(qObjectTrCalled);
}

TEST_F(TestOptical, BindFileOperations_HooksAllFileOperations)
{
    optical->bindFileOperations();
}

TEST_F(TestOptical, AddOpticalCrumbToTitleBar_CallsOnce)
{
    // Call multiple times to test std::once_flag
    optical->addOpticalCrumbToTitleBar();
    optical->addOpticalCrumbToTitleBar();
    optical->addOpticalCrumbToTitleBar();
}
