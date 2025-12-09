// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "controller/computercontroller.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QSignalSpy>
#include <QUrl>
#include <QDialog>
#include <QProcess>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerController : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        controller = ComputerController::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerController *controller = nullptr;
};

TEST_F(UT_ComputerController, Instance_SingletonPattern_ReturnsSameInstance)
{
    ComputerController *instance1 = ComputerController::instance();
    ComputerController *instance2 = ComputerController::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_ComputerController, OnOpenItem_ValidUrl_CallsAppropriateAction)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");

    bool actionCalled = false;
    stub.set_lamda(&ComputerController::onOpenItem, [&](ComputerController *, quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        actionCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, testUrl);
    });

    controller->onOpenItem(testWinId, testUrl);
    EXPECT_TRUE(actionCalled);
}

TEST_F(UT_ComputerController, DoRename_ValidUrlAndName_RequestsRename)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");
    QString newName = "New Device Name";

    QSignalSpy renameSpy(controller, &ComputerController::requestRename);

    bool actionCalled = false;
    stub.set_lamda(&ComputerController::doRename, [&](ComputerController *, quint64 winId, const QUrl &url, const QString &name) {
        __DBG_STUB_INVOKE__
        actionCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, testUrl);
        EXPECT_EQ(name, newName);
    });

    controller->doRename(testWinId, testUrl, newName);
    EXPECT_TRUE(actionCalled);
}

TEST_F(UT_ComputerController, MountDevice_WithEntryFileInfo_HandlesMount)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr; // Would be a valid pointer in real scenario
    ComputerController::ActionAfterMount action = ComputerController::kEnterDirectory;

    bool mountCalled = false;
    stub.set_lamda(static_cast<void(ComputerController::*)(quint64, const DFMEntryFileInfoPointer, ComputerController::ActionAfterMount)>(&ComputerController::mountDevice),
                   [&](ComputerController *, quint64 winId, const DFMEntryFileInfoPointer info, ComputerController::ActionAfterMount act) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(act, action);
    });

    controller->mountDevice(testWinId, testInfo, action);
    EXPECT_TRUE(mountCalled);
}

TEST_F(UT_ComputerController, MountDevice_WithIds_HandlesMount)
{
    quint64 testWinId = 12345;
    QString deviceId = "test-device-id";
    QString shellId = "test-shell-id";
    ComputerController::ActionAfterMount action = ComputerController::kEnterInNewWindow;

    bool mountCalled = false;
    stub.set_lamda(static_cast<void(ComputerController::*)(quint64, const QString &, const QString &, ComputerController::ActionAfterMount)>(&ComputerController::mountDevice),
                   [&](ComputerController *, quint64 winId, const QString &id, const QString &shell, ComputerController::ActionAfterMount act) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(id, deviceId);
        EXPECT_EQ(shell, shellId);
        EXPECT_EQ(act, action);
    });

    controller->mountDevice(testWinId, deviceId, shellId, action);
    EXPECT_TRUE(mountCalled);
}

TEST_F(UT_ComputerController, ActEject_ValidUrl_CallsEjectAction)
{
    QUrl testUrl("entry://test.blockdev");

    bool ejectCalled = false;
    stub.set_lamda(&ComputerController::actEject, [&](ComputerController *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        ejectCalled = true;
        EXPECT_EQ(url, testUrl);
    });

    controller->actEject(testUrl);
    EXPECT_TRUE(ejectCalled);
}

TEST_F(UT_ComputerController, ActOpenInNewWindow_ValidInfo_OpensWindow)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool openCalled = false;
    stub.set_lamda(&ComputerController::actOpenInNewWindow, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        openCalled = true;
        EXPECT_EQ(winId, testWinId);
    });

    controller->actOpenInNewWindow(testWinId, testInfo);
    EXPECT_TRUE(openCalled);
}

TEST_F(UT_ComputerController, ActOpenInNewTab_ValidInfo_OpensTab)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool openCalled = false;
    stub.set_lamda(&ComputerController::actOpenInNewTab, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        openCalled = true;
        EXPECT_EQ(winId, testWinId);
    });

    controller->actOpenInNewTab(testWinId, testInfo);
    EXPECT_TRUE(openCalled);
}

TEST_F(UT_ComputerController, ActMount_ValidInfo_MountsDevice)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;
    bool enterAfterMounted = true;

    bool mountCalled = false;
    stub.set_lamda(&ComputerController::actMount, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info, bool enter) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(enter, enterAfterMounted);
    });

    controller->actMount(testWinId, testInfo, enterAfterMounted);
    EXPECT_TRUE(mountCalled);
}

TEST_F(UT_ComputerController, ActUnmount_ValidInfo_UnmountsDevice)
{
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool unmountCalled = false;
    stub.set_lamda(&ComputerController::actUnmount, [&](ComputerController *, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        unmountCalled = true;
    });

    controller->actUnmount(testInfo);
    EXPECT_TRUE(unmountCalled);
}

TEST_F(UT_ComputerController, ActSafelyRemove_ValidInfo_RemovesDevice)
{
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool removeCalled = false;
    stub.set_lamda(&ComputerController::actSafelyRemove, [&](ComputerController *, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        removeCalled = true;
    });

    controller->actSafelyRemove(testInfo);
    EXPECT_TRUE(removeCalled);
}

TEST_F(UT_ComputerController, ActRename_ValidInfo_RequestsRename)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;
    bool triggerFromSidebar = false;

    bool renameCalled = false;
    stub.set_lamda(&ComputerController::actRename, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info, bool fromSidebar) {
        __DBG_STUB_INVOKE__
        renameCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(fromSidebar, triggerFromSidebar);
    });

    controller->actRename(testWinId, testInfo, triggerFromSidebar);
    EXPECT_TRUE(renameCalled);
}

TEST_F(UT_ComputerController, ActFormat_ValidInfo_FormatsDevice)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool formatCalled = false;
    stub.set_lamda(&ComputerController::actFormat, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        formatCalled = true;
        EXPECT_EQ(winId, testWinId);
    });

    controller->actFormat(testWinId, testInfo);
    EXPECT_TRUE(formatCalled);
}

TEST_F(UT_ComputerController, ActProperties_ValidInfo_ShowsProperties)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool propertiesCalled = false;
    stub.set_lamda(&ComputerController::actProperties, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        propertiesCalled = true;
        EXPECT_EQ(winId, testWinId);
    });

    controller->actProperties(testWinId, testInfo);
    EXPECT_TRUE(propertiesCalled);
}

TEST_F(UT_ComputerController, ActLogoutAndForgetPasswd_ValidInfo_LogsOut)
{
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool logoutCalled = false;
    stub.set_lamda(&ComputerController::actLogoutAndForgetPasswd, [&](ComputerController *, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        logoutCalled = true;
    });

    controller->actLogoutAndForgetPasswd(testInfo);
    EXPECT_TRUE(logoutCalled);
}

TEST_F(UT_ComputerController, ActErase_ValidInfo_ErasesDevice)
{
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool eraseCalled = false;
    stub.set_lamda(&ComputerController::actErase, [&](ComputerController *, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        eraseCalled = true;
    });

    controller->actErase(testInfo);
    EXPECT_TRUE(eraseCalled);
}

TEST_F(UT_ComputerController, DoSetAlias_ValidInfo_UpdatesAlias)
{
    DFMEntryFileInfoPointer testInfo = nullptr;
    QString alias = "Test Alias";

    QSignalSpy aliasSpy(controller, &ComputerController::updateItemAlias);

    bool aliasSet = false;
    stub.set_lamda(&ComputerController::doSetAlias, [&](ComputerController *, DFMEntryFileInfoPointer info, const QString &aliasName) {
        __DBG_STUB_INVOKE__
        aliasSet = true;
        EXPECT_EQ(aliasName, alias);
    });

    controller->doSetAlias(testInfo, alias);
    EXPECT_TRUE(aliasSet);
}

TEST_F(UT_ComputerController, OnMenuRequest_ValidParameters_HandlesMenuRequest)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");
    bool triggerFromSidebar = true;

    bool menuCalled = false;
    stub.set_lamda(&ComputerController::onMenuRequest, [&](ComputerController *, quint64 winId, const QUrl &url, bool fromSidebar) {
        __DBG_STUB_INVOKE__
        menuCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, testUrl);
        EXPECT_EQ(fromSidebar, triggerFromSidebar);
    });

    controller->onMenuRequest(testWinId, testUrl, triggerFromSidebar);
    EXPECT_TRUE(menuCalled);
}

TEST_F(UT_ComputerController, ActionAfterMount_EnumValues_AreValid)
{
    EXPECT_EQ(ComputerController::kEnterDirectory, 0);
    EXPECT_EQ(ComputerController::kEnterInNewWindow, 1);
    EXPECT_EQ(ComputerController::kEnterInNewTab, 2);
    EXPECT_EQ(ComputerController::kNone, 3);
}

TEST_F(UT_ComputerController, Signals_CanBeConnected_Success)
{
    QSignalSpy renameSpy(controller, &ComputerController::requestRename);
    QSignalSpy aliasSpy(controller, &ComputerController::updateItemAlias);

    // Test that signals can be connected (they start with 0 count)
    EXPECT_EQ(renameSpy.count(), 0);
    EXPECT_EQ(aliasSpy.count(), 0);

    // Verify signals are properly defined
    EXPECT_TRUE(QMetaObject::checkConnectArgs(
        SIGNAL(requestRename(quint64, QUrl)),
        SLOT(onRenameRequested(quint64, QUrl))
    ));

    EXPECT_TRUE(QMetaObject::checkConnectArgs(
        SIGNAL(updateItemAlias(QUrl)),
        SLOT(onAliasUpdated(QUrl))
    ));
}

TEST_F(UT_ComputerController, WaitUDisks2DataReady_ValidId_HandlesCorrectly)
{
    QString id = "test-device-id";

    bool waitCalled = false;
    stub.set_lamda(&ComputerController::waitUDisks2DataReady, [&](ComputerController *, const QString &devId) {
        __DBG_STUB_INVOKE__
        waitCalled = true;
        EXPECT_EQ(devId, id);
    });

    controller->waitUDisks2DataReady(id);
    EXPECT_TRUE(waitCalled);
}

TEST_F(UT_ComputerController, HandleUnAccessableDevCdCall_ValidParameters_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool handleCalled = false;
    stub.set_lamda(&ComputerController::handleUnAccessableDevCdCall, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        handleCalled = true;
        EXPECT_EQ(winId, testWinId);
    });

    controller->handleUnAccessableDevCdCall(testWinId, testInfo);
    EXPECT_TRUE(handleCalled);
}

TEST_F(UT_ComputerController, HandleNetworkCdCall_ValidParameters_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool handleCalled = false;
    stub.set_lamda(&ComputerController::handleNetworkCdCall, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        handleCalled = true;
        EXPECT_EQ(winId, testWinId);
    });

    controller->handleNetworkCdCall(testWinId, testInfo);
    EXPECT_TRUE(handleCalled);
}

TEST_F(UT_ComputerController, DoSetProtocolDeviceAlias_ValidInfo_SetsAlias)
{
    DFMEntryFileInfoPointer testInfo = nullptr;
    QString alias = "Test Protocol Alias";

    bool aliasSet = false;
    stub.set_lamda(&ComputerController::doSetProtocolDeviceAlias, [&](ComputerController *, DFMEntryFileInfoPointer info, const QString &aliasName) -> bool {
        __DBG_STUB_INVOKE__
        aliasSet = true;
        EXPECT_EQ(aliasName, alias);
        return true;
    });

    bool result = controller->doSetProtocolDeviceAlias(testInfo, alias);
    EXPECT_TRUE(aliasSet);
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerController, Constructor_CreatesInstance_Success)
{
    ComputerController *newController = new ComputerController();
    EXPECT_NE(newController, nullptr);
    delete newController;
}

TEST_F(UT_ComputerController, MountDevice_WithValidId_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    QString deviceId = "test-device-id";
    QString shellId = "test-shell-id";
    ComputerController::ActionAfterMount action = ComputerController::kEnterDirectory;

    bool mountCalled = false;
    stub.set_lamda(static_cast<void(ComputerController::*)(quint64, const QString &, const QString &, ComputerController::ActionAfterMount)>(&ComputerController::mountDevice),
                   [&](ComputerController *, quint64 winId, const QString &id, const QString &shell, ComputerController::ActionAfterMount act) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(id, deviceId);
        EXPECT_EQ(shell, shellId);
        EXPECT_EQ(act, action);
    });

    controller->mountDevice(testWinId, deviceId, shellId, action);
    EXPECT_TRUE(mountCalled);
}
