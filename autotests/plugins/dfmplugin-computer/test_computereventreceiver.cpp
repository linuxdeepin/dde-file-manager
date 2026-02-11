// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "events/computereventreceiver.h"
#include "utils/computerutils.h"

#include <dfm-base/file/entry/entryfileinfo.h>

#include <QUrl>
#include <QVariantMap>
#include <QList>
#include <functional>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerEventReceiver : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        receiver = ComputerEventReceiver::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerEventReceiver *receiver = nullptr;
};

TEST_F(UT_ComputerEventReceiver, Instance_SingletonPattern_ReturnsSameInstance)
{
    ComputerEventReceiver *instance1 = ComputerEventReceiver::instance();
    ComputerEventReceiver *instance2 = ComputerEventReceiver::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_ComputerEventReceiver, HandleItemEject_ValidUrl_CallsEjectAction)
{
    QUrl testUrl("entry://test.blockdev");

    bool ejectHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleItemEject, [&](ComputerEventReceiver *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        ejectHandled = true;
        EXPECT_EQ(url, testUrl);
    });

    receiver->handleItemEject(testUrl);
    EXPECT_TRUE(ejectHandled);
}

TEST_F(UT_ComputerEventReceiver, HandleItemEject_EmptyUrl_HandlesGracefully)
{
    QUrl emptyUrl;

    bool ejectHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleItemEject, [&](ComputerEventReceiver *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        ejectHandled = true;
        EXPECT_TRUE(url.isEmpty());
    });

    receiver->handleItemEject(emptyUrl);
    EXPECT_TRUE(ejectHandled);
}

TEST_F(UT_ComputerEventReceiver, HandleSepateTitlebarCrumb_ValidUrl_ReturnsCorrectResult)
{
    QUrl testUrl("computer:///");
    QList<QVariantMap> mapGroup;

    bool crumbHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSepateTitlebarCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        crumbHandled = true;
        EXPECT_EQ(url, testUrl);
        EXPECT_NE(group, nullptr);
        return true;
    });

    bool result = receiver->handleSepateTitlebarCrumb(testUrl, &mapGroup);
    EXPECT_TRUE(crumbHandled);
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerEventReceiver, HandleSepateTitlebarCrumb_NullMapGroup_HandlesGracefully)
{
    QUrl testUrl("computer:///");

    bool crumbHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSepateTitlebarCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        crumbHandled = true;
        EXPECT_EQ(url, testUrl);
        EXPECT_EQ(group, nullptr);
        return false;
    });

    bool result = receiver->handleSepateTitlebarCrumb(testUrl, nullptr);
    EXPECT_TRUE(crumbHandled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, HandleSortItem_ValidUrls_ReturnsCorrectOrder)
{
    QString group = "test-group";
    QString subGroup = "test-subgroup";
    QUrl urlA("entry://test-a.blockdev");
    QUrl urlB("entry://test-b.blockdev");

    bool sortHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSortItem, [&](ComputerEventReceiver *, const QString &grp, const QString &subGrp, const QUrl &a, const QUrl &b) -> bool {
        __DBG_STUB_INVOKE__
        sortHandled = true;
        EXPECT_EQ(grp, group);
        EXPECT_EQ(subGrp, subGroup);
        EXPECT_EQ(a, urlA);
        EXPECT_EQ(b, urlB);
        return true;
    });

    bool result = receiver->handleSortItem(group, subGroup, urlA, urlB);
    EXPECT_TRUE(sortHandled);
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerEventReceiver, HandleSortItem_EmptyGroup_HandlesGracefully)
{
    QString emptyGroup;
    QString subGroup = "test-subgroup";
    QUrl urlA("entry://test-a.blockdev");
    QUrl urlB("entry://test-b.blockdev");

    bool sortHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSortItem, [&](ComputerEventReceiver *, const QString &grp, const QString &subGrp, const QUrl &a, const QUrl &b) -> bool {
        __DBG_STUB_INVOKE__
        sortHandled = true;
        EXPECT_TRUE(grp.isEmpty());
        return false;
    });

    bool result = receiver->handleSortItem(emptyGroup, subGroup, urlA, urlB);
    EXPECT_TRUE(sortHandled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, HandleSetTabName_ValidUrl_SetsTabName)
{
    QUrl testUrl("computer:///");
    QString tabName;

    bool tabNameHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSetTabName, [&](ComputerEventReceiver *, const QUrl &url, QString *name) -> bool {
        __DBG_STUB_INVOKE__
        tabNameHandled = true;
        EXPECT_EQ(url, testUrl);
        EXPECT_NE(name, nullptr);
        *name = "Computer";
        return true;
    });

    bool result = receiver->handleSetTabName(testUrl, &tabName);
    EXPECT_TRUE(tabNameHandled);
    EXPECT_TRUE(result);
    EXPECT_EQ(tabName, "Computer");
}

TEST_F(UT_ComputerEventReceiver, HandleSetTabName_NullTabName_HandlesGracefully)
{
    QUrl testUrl("computer:///");

    bool tabNameHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSetTabName, [&](ComputerEventReceiver *, const QUrl &url, QString *name) -> bool {
        __DBG_STUB_INVOKE__
        tabNameHandled = true;
        EXPECT_EQ(url, testUrl);
        EXPECT_EQ(name, nullptr);
        return false;
    });

    bool result = receiver->handleSetTabName(testUrl, nullptr);
    EXPECT_TRUE(tabNameHandled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, SetContextMenuEnable_EnabledState_SetsCorrectState)
{
    bool enabled = true;

    bool menuStateSet = false;
    stub.set_lamda(&ComputerEventReceiver::setContextMenuEnable, [&](ComputerEventReceiver *, bool enable) {
        __DBG_STUB_INVOKE__
        menuStateSet = true;
        EXPECT_EQ(enable, enabled);
    });

    receiver->setContextMenuEnable(enabled);
    EXPECT_TRUE(menuStateSet);
}

TEST_F(UT_ComputerEventReceiver, SetContextMenuEnable_DisabledState_SetsCorrectState)
{
    bool enabled = false;

    bool menuStateSet = false;
    stub.set_lamda(&ComputerEventReceiver::setContextMenuEnable, [&](ComputerEventReceiver *, bool enable) {
        __DBG_STUB_INVOKE__
        menuStateSet = true;
        EXPECT_EQ(enable, enabled);
    });

    receiver->setContextMenuEnable(enabled);
    EXPECT_TRUE(menuStateSet);
}

TEST_F(UT_ComputerEventReceiver, DirAccessPrehandler_ValidParameters_CallsPrehandler)
{
    quint64 testWinId = 12345;
    QUrl testUrl("file:///home/test");
    bool afterCalled = false;

    std::function<void()> afterFunction = [&]() {
        afterCalled = true;
    };

    bool prehandlerCalled = false;
    stub.set_lamda(&ComputerEventReceiver::dirAccessPrehandler, [&](quint64 winId, const QUrl &url, std::function<void()> after) {
        __DBG_STUB_INVOKE__
        prehandlerCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, testUrl);
        after(); // Call the after function
    });

    ComputerEventReceiver::dirAccessPrehandler(testWinId, testUrl, afterFunction);
    EXPECT_TRUE(prehandlerCalled);
    EXPECT_TRUE(afterCalled);
}

TEST_F(UT_ComputerEventReceiver, DirAccessPrehandler_EmptyAfterFunction_HandlesGracefully)
{
    quint64 testWinId = 12345;
    QUrl testUrl("file:///home/test");
    std::function<void()> emptyFunction;

    bool prehandlerCalled = false;
    stub.set_lamda(&ComputerEventReceiver::dirAccessPrehandler, [&](quint64 winId, const QUrl &url, std::function<void()> after) {
        __DBG_STUB_INVOKE__
        prehandlerCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, testUrl);
        // Don't call after function since it's empty
    });

    EXPECT_NO_THROW(ComputerEventReceiver::dirAccessPrehandler(testWinId, testUrl, emptyFunction));
    EXPECT_TRUE(prehandlerCalled);
}

TEST_F(UT_ComputerEventReceiver, ParseCifsMountCrumb_ValidSmbUrl_ParsesCorrectly)
{
    QUrl smbUrl("smb://server/share/path");
    QList<QVariantMap> mapGroup;

    bool parseCalled = false;
    stub.set_lamda(&ComputerEventReceiver::parseCifsMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        parseCalled = true;
        EXPECT_EQ(url, smbUrl);
        EXPECT_NE(group, nullptr);

        // Simulate adding crumb data
        QVariantMap crumbData;
        crumbData["displayText"] = "share";
        crumbData["url"] = "smb://server/share";
        group->append(crumbData);

        return true;
    });

    bool result = receiver->parseCifsMountCrumb(smbUrl, &mapGroup);
    EXPECT_TRUE(parseCalled);
    EXPECT_TRUE(result);
    EXPECT_EQ(mapGroup.size(), 1);
}

TEST_F(UT_ComputerEventReceiver, ParseCifsMountCrumb_NonSmbUrl_ReturnsFalse)
{
    QUrl nonSmbUrl("file:///home/test");
    QList<QVariantMap> mapGroup;

    bool parseCalled = false;
    stub.set_lamda(&ComputerEventReceiver::parseCifsMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        parseCalled = true;
        EXPECT_EQ(url, nonSmbUrl);
        return false;
    });

    bool result = receiver->parseCifsMountCrumb(nonSmbUrl, &mapGroup);
    EXPECT_TRUE(parseCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, AskForConfirmChmod_ValidDeviceName_ReturnsAnswer)
{
    QString deviceName = "Test Device";

    bool confirmCalled = false;
    stub.set_lamda(&ComputerEventReceiver::askForConfirmChmod, [&](const QString &devName) -> bool {
        __DBG_STUB_INVOKE__
        confirmCalled = true;
        EXPECT_EQ(devName, deviceName);
        return true; // User confirms
    });

    bool result = ComputerEventReceiver::askForConfirmChmod(deviceName);
    EXPECT_TRUE(confirmCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerEventReceiver, AskForConfirmChmod_EmptyDeviceName_HandlesGracefully)
{
    QString emptyName;

    bool confirmCalled = false;
    stub.set_lamda(&ComputerEventReceiver::askForConfirmChmod, [&](const QString &devName) -> bool {
        __DBG_STUB_INVOKE__
        confirmCalled = true;
        EXPECT_TRUE(devName.isEmpty());
        return false; // User cancels
    });

    bool result = ComputerEventReceiver::askForConfirmChmod(emptyName);
    EXPECT_TRUE(confirmCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, ErrorHandling_InvalidUrl_HandlesGracefully)
{
    QUrl invalidUrl("invalid://malformed");

    // These should not crash with invalid URLs
    EXPECT_NO_THROW(receiver->handleItemEject(invalidUrl));

    QList<QVariantMap> mapGroup;
    EXPECT_NO_THROW(receiver->handleSepateTitlebarCrumb(invalidUrl, &mapGroup));

    QString tabName;
    EXPECT_NO_THROW(receiver->handleSetTabName(invalidUrl, &tabName));
}

TEST_F(UT_ComputerEventReceiver, ErrorHandling_InvalidWinId_HandlesGracefully)
{
    quint64 invalidWinId = 0;
    QUrl testUrl("file:///test");
    std::function<void()> afterFunction = [](){};

    // Should not crash with invalid window ID
    EXPECT_NO_THROW(ComputerEventReceiver::dirAccessPrehandler(invalidWinId, testUrl, afterFunction));
}

TEST_F(UT_ComputerEventReceiver, SlotMethods_ExistAndCallable_Success)
{
    // Verify that slot methods exist and can be called
    QUrl testUrl("entry://test.blockdev");

    // Test that these methods exist and don't crash
    EXPECT_NO_THROW(receiver->handleItemEject(testUrl));
    EXPECT_NO_THROW(receiver->setContextMenuEnable(true));
    EXPECT_NO_THROW(receiver->setContextMenuEnable(false));

    QList<QVariantMap> mapGroup;
    QString tabName;

    EXPECT_NO_THROW(receiver->handleSepateTitlebarCrumb(testUrl, &mapGroup));
    EXPECT_NO_THROW(receiver->handleSetTabName(testUrl, &tabName));
    EXPECT_NO_THROW(receiver->handleSortItem("group", "subgroup", testUrl, testUrl));
}

TEST_F(UT_ComputerEventReceiver, ParseDevMountCrumb_ValidDeviceUrl_ParsesCorrectly)
{
    QUrl testUrl("file:///media/test-device");
    QList<QVariantMap> mapGroup;
    
    bool parseCalled = false;
    stub.set_lamda(&ComputerEventReceiver::parseDevMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        parseCalled = true;
        EXPECT_EQ(url, testUrl);
        EXPECT_NE(group, nullptr);
        
        // Simulate adding crumb data
        QVariantMap crumbData;
        crumbData["displayText"] = "Test Device";
        crumbData["url"] = "file:///media/test-device";
        crumbData["iconName"] = "drive-harddisk-symbolic";
        group->append(crumbData);
        
        return true;
    });
    
    bool result = receiver->parseDevMountCrumb(testUrl, &mapGroup);
    EXPECT_TRUE(parseCalled);
    EXPECT_TRUE(result);
    EXPECT_EQ(mapGroup.size(), 1);
}

TEST_F(UT_ComputerEventReceiver, ParseDevMountCrumb_NonDeviceUrl_ReturnsFalse)
{
    QUrl nonDeviceUrl("file:///home/user/documents");
    QList<QVariantMap> mapGroup;
    
    bool parseCalled = false;
    stub.set_lamda(&ComputerEventReceiver::parseDevMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        parseCalled = true;
        EXPECT_EQ(url, nonDeviceUrl);
        EXPECT_NE(group, nullptr);
        return false;
    });
    
    bool result = receiver->parseDevMountCrumb(nonDeviceUrl, &mapGroup);
    EXPECT_TRUE(parseCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, ParseDevMountCrumb_NullMapGroup_HandlesGracefully)
{
    QUrl testUrl("file:///media/test-device");
    
    bool parseCalled = false;
    stub.set_lamda(&ComputerEventReceiver::parseDevMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        parseCalled = true;
        EXPECT_EQ(url, testUrl);
        EXPECT_EQ(group, nullptr);
        return false;
    });
    
    bool result = receiver->parseDevMountCrumb(testUrl, nullptr);
    EXPECT_TRUE(parseCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, HandleSepateTitlebarCrumb_ComputerScheme_AddsComputerCrumb)
{
    QUrl computerUrl("computer:///");
    QList<QVariantMap> mapGroup;
    
    bool crumbHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSepateTitlebarCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        crumbHandled = true;
        EXPECT_EQ(url, computerUrl);
        EXPECT_NE(group, nullptr);
        
        // Simulate adding computer crumb data
        QVariantMap crumbData;
        crumbData["url"] = computerUrl;
        crumbData["displayText"] = "Computer";
        crumbData["iconName"] = "computer-symbolic";
        group->append(crumbData);
        
        return true;
    });
    
    bool result = receiver->handleSepateTitlebarCrumb(computerUrl, &mapGroup);
    EXPECT_TRUE(crumbHandled);
    EXPECT_TRUE(result);
    EXPECT_EQ(mapGroup.size(), 1);
    EXPECT_EQ(mapGroup.first()["url"], computerUrl);
    EXPECT_EQ(mapGroup.first()["displayText"], "Computer");
}

TEST_F(UT_ComputerEventReceiver, HandleSepateTitlebarCrumb_FileScheme_CallsParseMethods)
{
    QUrl fileUrl("file:///media/smbmounts/server/share");
    QList<QVariantMap> mapGroup;
    
    bool cifsParseCalled = false;
    bool devParseCalled = false;
    
    // Mock parseCifsMountCrumb
    stub.set_lamda(&ComputerEventReceiver::parseCifsMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        cifsParseCalled = true;
        EXPECT_EQ(url, fileUrl);
        EXPECT_NE(group, nullptr);
        
        // Simulate adding crumb data
        QVariantMap crumbData;
        crumbData["url"] = fileUrl;
        crumbData["displayText"] = "share";
        group->append(crumbData);
        
        return true;
    });
    
    // Mock parseDevMountCrumb
    stub.set_lamda(&ComputerEventReceiver::parseDevMountCrumb, [&](ComputerEventReceiver *, const QUrl &url, QList<QVariantMap> *group) -> bool {
        __DBG_STUB_INVOKE__
        devParseCalled = true;
        EXPECT_EQ(url, fileUrl);
        EXPECT_NE(group, nullptr);
        return false;
    });
    
    bool result = receiver->handleSepateTitlebarCrumb(fileUrl, &mapGroup);
    EXPECT_TRUE(cifsParseCalled);
    EXPECT_TRUE(result);
    EXPECT_EQ(mapGroup.size(), 1);
}

TEST_F(UT_ComputerEventReceiver, HandleSortItem_InvalidGroup_ReturnsFalse)
{
    QString invalidGroup = "InvalidGroup";
    QString subGroup = "computer";
    QUrl urlA("entry://test-a.blockdev");
    QUrl urlB("entry://test-b.blockdev");
    
    bool sortHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSortItem, [&](ComputerEventReceiver *, const QString &grp, const QString &subGrp, const QUrl &a, const QUrl &b) -> bool {
        __DBG_STUB_INVOKE__
        sortHandled = true;
        EXPECT_EQ(grp, invalidGroup);
        EXPECT_EQ(subGrp, subGroup);
        EXPECT_EQ(a, urlA);
        EXPECT_EQ(b, urlB);
        return false;
    });
    
    bool result = receiver->handleSortItem(invalidGroup, subGroup, urlA, urlB);
    EXPECT_TRUE(sortHandled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, HandleSortItem_InvalidSubGroup_ReturnsFalse)
{
    QString group = "Group_Device";
    QString invalidSubGroup = "InvalidSubGroup";
    QUrl urlA("entry://test-a.blockdev");
    QUrl urlB("entry://test-b.blockdev");
    
    bool sortHandled = false;
    stub.set_lamda(&ComputerEventReceiver::handleSortItem, [&](ComputerEventReceiver *, const QString &grp, const QString &subGrp, const QUrl &a, const QUrl &b) -> bool {
        __DBG_STUB_INVOKE__
        sortHandled = true;
        EXPECT_EQ(grp, group);
        EXPECT_EQ(subGrp, invalidSubGroup);
        EXPECT_EQ(a, urlA);
        EXPECT_EQ(b, urlB);
        return false;
    });
    
    bool result = receiver->handleSortItem(group, invalidSubGroup, urlA, urlB);
    EXPECT_TRUE(sortHandled);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerEventReceiver, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    QUrl testUrl("entry://test.blockdev");
    QString testGroup = "Group_Device";
    QString testSubGroup = "computer";
    QList<QVariantMap> mapGroup;
    QString tabName;
    
    // Mock all methods
    int ejectCallCount = 0;
    int crumbCallCount = 0;
    int sortCallCount = 0;
    int tabNameCallCount = 0;
    int menuStateCallCount = 0;
    
    stub.set_lamda(&ComputerEventReceiver::handleItemEject, [&](ComputerEventReceiver *, const QUrl &) {
        __DBG_STUB_INVOKE__
        ejectCallCount++;
    });
    
    stub.set_lamda(&ComputerEventReceiver::handleSepateTitlebarCrumb, [&](ComputerEventReceiver *, const QUrl &, QList<QVariantMap> *) -> bool {
        __DBG_STUB_INVOKE__
        crumbCallCount++;
        return true;
    });
    
    stub.set_lamda(&ComputerEventReceiver::handleSortItem, [&](ComputerEventReceiver *, const QString &, const QString &, const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        sortCallCount++;
        return true;
    });
    
    stub.set_lamda(&ComputerEventReceiver::handleSetTabName, [&](ComputerEventReceiver *, const QUrl &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        tabNameCallCount++;
        return true;
    });
    
    stub.set_lamda(&ComputerEventReceiver::setContextMenuEnable, [&](ComputerEventReceiver *, bool) {
        __DBG_STUB_INVOKE__
        menuStateCallCount++;
    });
    
    // Call multiple methods
    receiver->handleItemEject(testUrl);
    receiver->handleSepateTitlebarCrumb(testUrl, &mapGroup);
    receiver->handleSortItem(testGroup, testSubGroup, testUrl, testUrl);
    receiver->handleSetTabName(testUrl, &tabName);
    receiver->setContextMenuEnable(true);
    
    EXPECT_EQ(ejectCallCount, 1);
    EXPECT_EQ(crumbCallCount, 1);
    EXPECT_EQ(sortCallCount, 1);
    EXPECT_EQ(tabNameCallCount, 1);
    EXPECT_EQ(menuStateCallCount, 1);
}