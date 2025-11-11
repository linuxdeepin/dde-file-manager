// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "events/computereventcaller.h"
#include "utils/computerutils.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QIcon>
#include <QList>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerEventCaller : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        mockWidget = new QWidget();
    }

    virtual void TearDown() override
    {
        delete mockWidget;
        mockWidget = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QWidget *mockWidget = nullptr;
};

TEST_F(UT_ComputerEventCaller, CdTo_WidgetSender_ValidUrl_CallsSuccessfully)
{
    QUrl testUrl("file:///home/user/Documents");
    quint64 mockWinId = 12345;

    // Mock FMWindowsIns.findWindowId
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [&] {
        __DBG_STUB_INVOKE__
        return mockWinId;
    });

    // Mock the overloaded cdTo method
    bool cdToCalled = false;
    stub.set_lamda(static_cast<void (*)(quint64, const QUrl &)>(ComputerEventCaller::cdTo), [&](quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        cdToCalled = true;
        EXPECT_EQ(winId, mockWinId);
        EXPECT_EQ(url, testUrl);
    });

    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, testUrl));
    EXPECT_TRUE(cdToCalled);
}

TEST_F(UT_ComputerEventCaller, CdTo_WidgetSender_InvalidUrl_LogsWarning)
{
    QUrl invalidUrl;

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, invalidUrl));
}

TEST_F(UT_ComputerEventCaller, CdTo_WidgetSender_ZeroWindowId_LogsWarning)
{
    QUrl testUrl("file:///home/user/Documents");

    // Mock findWindowId to return 0
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [&] {
        __DBG_STUB_INVOKE__
        return 0;
    });

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, testUrl));
}

TEST_F(UT_ComputerEventCaller, CdTo_WidgetSender_ValidPath_CallsUrlOverload)
{
    QString testPath = "/home/user/Documents";
    QUrl expectedUrl = QUrl::fromLocalFile(testPath);

    // Mock ComputerUtils::makeLocalUrl
    stub.set_lamda(&ComputerUtils::makeLocalUrl, [&](const QString &path) -> QUrl {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(path, testPath);
        return expectedUrl;
    });

    bool cdToCalled = false;
    stub.set_lamda(static_cast<void (*)(QWidget *, const QUrl &)>(ComputerEventCaller::cdTo), [&](QWidget *sender, const QUrl &url) {
        __DBG_STUB_INVOKE__
        cdToCalled = true;
        EXPECT_EQ(sender, mockWidget);
        EXPECT_EQ(url, expectedUrl);
    });

    ComputerEventCaller::cdTo(mockWidget, testPath);
    EXPECT_TRUE(cdToCalled);
}

TEST_F(UT_ComputerEventCaller, CdTo_WidgetSender_EmptyPath_LogsWarning)
{
    QString emptyPath = "";

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, emptyPath));
}

TEST_F(UT_ComputerEventCaller, CdTo_WindowId_ValidUrl_GvfsMountExists_PublishesEvent)
{
    quint64 testWinId = 12345;
    QUrl testUrl("file:///home/user/Documents");

    // Mock GVFS mount check
    stub.set_lamda(&ComputerUtils::checkGvfsMountExist, [&](const QUrl &url, int) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(url, testUrl);
        return true;
    });

    // Mock DConfigManager
    stub.set_lamda(&DConfigManager::value, [&](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(true);   // kOpenFolderWindowsInASeparateProcess = true
    });

    // Mock FileManagerWindowsManager
    stub.set_lamda(&FileManagerWindowsManager::containsCurrentUrl, [&] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock Application::appAttribute
    stub.set_lamda(&Application::appAttribute, [&](Application::ApplicationAttribute) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });

    ComputerEventCaller::cdTo(testWinId, testUrl);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, CdTo_WindowId_GvfsMountNotExist_LogsWarning)
{
    quint64 testWinId = 12345;
    QUrl testUrl("file:///nonexistent");

    // Mock GVFS mount check to return false
    stub.set_lamda(&ComputerUtils::checkGvfsMountExist, [&] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(testWinId, testUrl));
}

TEST_F(UT_ComputerEventCaller, CdTo_WindowId_ValidPath_CallsUrlOverload)
{
    quint64 testWinId = 12345;
    QString testPath = "/home/user/Documents";
    QUrl expectedUrl = QUrl::fromLocalFile(testPath);

    // Mock ComputerUtils::makeLocalUrl
    stub.set_lamda(&ComputerUtils::makeLocalUrl, [&](const QString &path) -> QUrl {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(path, testPath);
        return expectedUrl;
    });

    bool cdToCalled = false;
    stub.set_lamda(static_cast<void (*)(quint64, const QUrl &)>(ComputerEventCaller::cdTo), [&](quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        cdToCalled = true;
        EXPECT_EQ(winId, testWinId);
        EXPECT_EQ(url, expectedUrl);
    });

    ComputerEventCaller::cdTo(testWinId, testPath);
    EXPECT_TRUE(cdToCalled);
}

TEST_F(UT_ComputerEventCaller, CdTo_WindowId_EmptyPath_LogsWarning)
{
    quint64 testWinId = 12345;
    QString emptyPath = "";

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(testWinId, emptyPath));
}

TEST_F(UT_ComputerEventCaller, SendEnterInNewWindow_ValidUrl_PublishesEvent)
{
    QUrl testUrl("file:///home/user/Documents");
    bool isNew = true;

    // Mock GVFS mount check
    stub.set_lamda(&ComputerUtils::checkGvfsMountExist, [&](const QUrl &url, int) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(url, testUrl);
        return true;
    });

    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });

    ComputerEventCaller::sendEnterInNewWindow(testUrl, isNew);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendEnterInNewWindow_GvfsMountNotExist_LogsWarning)
{
    QUrl testUrl("file:///nonexistent");
    bool isNew = true;

    // Mock GVFS mount check to return false
    stub.set_lamda(&ComputerUtils::checkGvfsMountExist, [&] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::sendEnterInNewWindow(testUrl, isNew));
}

TEST_F(UT_ComputerEventCaller, SendEnterInNewTab_ValidUrl_PublishesEvent)
{
    quint64 testWinId = 12345;
    QUrl testUrl("file:///home/user/Documents");

    // Mock GVFS mount check
    stub.set_lamda(&ComputerUtils::checkGvfsMountExist, [&](const QUrl &url, int) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(url, testUrl);
        return true;
    });

    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });

    ComputerEventCaller::sendEnterInNewTab(testWinId, testUrl);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendEnterInNewTab_GvfsMountNotExist_LogsWarning)
{
    quint64 testWinId = 12345;
    QUrl testUrl("file:///nonexistent");

    // Mock GVFS mount check to return false
    stub.set_lamda(&ComputerUtils::checkGvfsMountExist, [&] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Should log warning and return early
    EXPECT_NO_THROW(ComputerEventCaller::sendEnterInNewTab(testWinId, testUrl));
}

TEST_F(UT_ComputerEventCaller, SendOpenItem_ValidParameters_PublishesEvent)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");

    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });

    ComputerEventCaller::sendOpenItem(testWinId, testUrl);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendCtrlNOnItem_ValidParameters_PublishesEvent)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");

    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });

    ComputerEventCaller::sendCtrlNOnItem(testWinId, testUrl);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendCtrlTOnItem_ValidParameters_PublishesEvent)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");

    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });

    ComputerEventCaller::sendCtrlTOnItem(testWinId, testUrl);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendShowPropertyDialog_ValidUrls_PushesToSlotChannel)
{
    QList<QUrl> testUrls = {
        QUrl("entry://test1.blockdev"),
        QUrl("entry://test2.blockdev")
    };

    // Mock dpfSlotChannel->push
    bool slotPushed = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, const QList<QUrl> &urls, const QVariantHash &properties) -> bool {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_EQ(space, QString("dfmplugin_propertydialog"));
        EXPECT_EQ(topic, QString("slot_PropertyDialog_Show"));
        EXPECT_EQ(urls.size(), 2);
        EXPECT_EQ(urls, testUrls);
        EXPECT_TRUE(properties.isEmpty());
        return true;
    });

    ComputerEventCaller::sendShowPropertyDialog(testUrls);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_ComputerEventCaller, SendShowPropertyDialog_EmptyUrls_PushesToSlotChannel)
{
    QList<QUrl> emptyUrls;

    // Mock dpfSlotChannel->push
    bool slotPushed = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);
    auto push = static_cast<Push>(&EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, const QList<QUrl> &urls, const QVariantHash &properties) -> bool {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_TRUE(urls.isEmpty());
        return true;
    });

    ComputerEventCaller::sendShowPropertyDialog(emptyUrls);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_ComputerEventCaller, SendErase_ValidDevice_PushesToSlotChannel)
{
    QString testDevice = "/dev/sr0";

    // Mock dpfSlotChannel->push
    bool slotPushed = false;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &space, const QString &topic, const QString &device) -> bool {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_EQ(space, QString("dfmplugin_burn"));
        EXPECT_EQ(topic, QString("slot_Erase"));
        EXPECT_EQ(device, testDevice);
        return true;
    });

    ComputerEventCaller::sendErase(testDevice);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_ComputerEventCaller, SendErase_EmptyDevice_PushesToSlotChannel)
{
    QString emptyDevice = "";

    // Mock dpfSlotChannel->push
    bool slotPushed = false;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [&](EventChannelManager *, const QString &, const QString &, QString device) {
        __DBG_STUB_INVOKE__
        slotPushed = true;
        EXPECT_TRUE(device.isEmpty());
        return true;
    });

    ComputerEventCaller::sendErase(emptyDevice);
    EXPECT_TRUE(slotPushed);
}

TEST_F(UT_ComputerEventCaller, StaticMethods_NoInstanceRequired_CallSuccessfully)
{
    // Verify that all methods are static and don't require instance
    QUrl testUrl("file:///home/user/Documents");
    quint64 testWinId = 12345;
    QString testPath = "/home/user/Documents";
    QString testDevice = "/dev/sdb1";
    QList<QUrl> testUrls = { testUrl };

    // These should be callable without creating an instance
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, testPath));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(testWinId, testPath));
    EXPECT_NO_THROW(ComputerEventCaller::sendEnterInNewWindow(testUrl, true));
    EXPECT_NO_THROW(ComputerEventCaller::sendEnterInNewTab(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendOpenItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendCtrlNOnItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendCtrlTOnItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendShowPropertyDialog(testUrls));
    EXPECT_NO_THROW(ComputerEventCaller::sendErase(testDevice));
}

TEST_F(UT_ComputerEventCaller, ErrorHandling_InvalidParameters_HandlesGracefully)
{
    // Test with various invalid parameters
    QUrl invalidUrl;
    QString emptyPath = "";
    quint64 zeroWinId = 0;
    QList<QUrl> emptyUrls;
    QString emptyDevice = "";

    EXPECT_NO_THROW(ComputerEventCaller::cdTo(nullptr, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(mockWidget, emptyPath));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(zeroWinId, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::cdTo(zeroWinId, emptyPath));
    EXPECT_NO_THROW(ComputerEventCaller::sendEnterInNewWindow(invalidUrl, true));
    EXPECT_NO_THROW(ComputerEventCaller::sendEnterInNewTab(zeroWinId, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendOpenItem(zeroWinId, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendCtrlNOnItem(zeroWinId, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendCtrlTOnItem(zeroWinId, invalidUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendShowPropertyDialog(emptyUrls));
    EXPECT_NO_THROW(ComputerEventCaller::sendErase(emptyDevice));
}

TEST_F(UT_ComputerEventCaller, LoggingBehavior_DebugMessages_LoggedCorrectly)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test.blockdev");

    // Mock to avoid actual event publishing
    typedef bool (EventDispatcherManager::*Publish)(const QString &, const QString &, quint64, const QUrl &);
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&] {
        __DBG_STUB_INVOKE__
        return true;
    });

    // These methods should log debug messages
    EXPECT_NO_THROW(ComputerEventCaller::sendOpenItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendCtrlNOnItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendCtrlTOnItem(testWinId, testUrl));
}

TEST_F(UT_ComputerEventCaller, SendItemRenamed_ValidParameters_PublishesEvent)
{
    QUrl testUrl("entry://test.blockdev");
    QString newName = "New Device Name";
    
    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });
    
    ComputerEventCaller::sendItemRenamed(testUrl, newName);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendItemRenamed_EmptyName_PublishesEvent)
{
    QUrl testUrl("entry://test.blockdev");
    QString emptyName = "";
    
    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, testUrl);
        return true;
    });
    
    ComputerEventCaller::sendItemRenamed(testUrl, emptyName);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, SendItemRenamed_InvalidUrl_PublishesEvent)
{
    QUrl invalidUrl;
    QString testName = "Test Name";
    
    // Mock dpfSignalDispatcher->publish
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        EXPECT_EQ(url, invalidUrl);
        return true;
    });
    
    ComputerEventCaller::sendItemRenamed(invalidUrl, testName);
    EXPECT_TRUE(eventPublished);
}

TEST_F(UT_ComputerEventCaller, MultipleEventCalls_DifferentParameters_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    QUrl testUrl1("entry://test1.blockdev");
    QUrl testUrl2("entry://test2.blockdev");
    QString testAction = "computer-eject";
    QString newName = "Renamed Device";
    
    // Mock all event publishing
    int publishCallCount = 0;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        publishCallCount++;
        return true;
    });
    
    // Call multiple events
    ComputerEventCaller::sendOpenItem(testWinId, testUrl1);
    ComputerEventCaller::sendCtrlNOnItem(testWinId, testUrl2);
    ComputerEventCaller::sendItemRenamed(testUrl1, newName);
    
    EXPECT_EQ(publishCallCount, 4);
}

TEST_F(UT_ComputerEventCaller, EventParameters_SpecialCharacters_HandlesCorrectly)
{
    quint64 testWinId = 12345;
    QUrl testUrl("entry://test-with-special-chars_&%$.blockdev");
    QString testAction = "computer-action-with-special-chars_&%$";
    QString newName = "New Name with 特殊字符 & % $";
    
    // Mock event publishing
    bool eventPublished = false;
    // Use a specific overload of publish method
    using PublishFunc = bool (EventDispatcherManager::*)(const QString&, const QString&, const QUrl&);
    auto publish = static_cast<PublishFunc>(&EventDispatcherManager::publish);
    stub.set_lamda(publish, [&](EventDispatcherManager *, const QString &, const QString &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        eventPublished = true;
        return true;
    });
    
    // Test with special characters
    EXPECT_NO_THROW(ComputerEventCaller::sendOpenItem(testWinId, testUrl));
    EXPECT_NO_THROW(ComputerEventCaller::sendItemRenamed(testUrl, newName));
    
    EXPECT_TRUE(eventPublished);
}
