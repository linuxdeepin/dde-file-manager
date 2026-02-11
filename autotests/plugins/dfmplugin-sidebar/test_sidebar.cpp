// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "sidebar.h"
#include "treeviews/sidebarwidget.h"
#include "treeviews/sidebarview.h"
#include "utils/sidebarhelper.h"
#include "events/sidebareventreceiver.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QSignalSpy>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SideBar : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new SideBar();
        ASSERT_NE(plugin, nullptr);

        // Stub thread-related functions
        stub.set_lamda(ADDR(QThread, start), [](QThread *, QThread::Priority) {
            __DBG_STUB_INVOKE__
            // Prevent thread start in unit tests
        });
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

protected:
    SideBar *plugin { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBar, Initialize)
{
    bool connectCalled = false;

    stub.set_lamda(&SideBarEventReceiver::instance, []() -> SideBarEventReceiver * {
        __DBG_STUB_INVOKE__
        static SideBarEventReceiver receiver;
        return &receiver;
    });

    stub.set_lamda(&SideBarEventReceiver::bindEvents, [&connectCalled]() {
        __DBG_STUB_INVOKE__
        connectCalled = true;
    });

    plugin->initialize();

    EXPECT_TRUE(connectCalled);
}

TEST_F(UT_SideBar, Start_Success)
{
    bool addConfigCalled = false;
    bool initSettingCalled = false;
    bool registCustomCalled = false;
    bool installFilterCalled = false;

    stub.set_lamda(&DConfigManager::instance, []() -> DConfigManager * {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::addConfig,
                   [&addConfigCalled](DConfigManager *, const QString &, QString *) -> bool {
                       __DBG_STUB_INVOKE__
                       addConfigCalled = true;
                       return true;
                   });

    stub.set_lamda(&SideBarHelper::initDefaultSettingPanel, [&initSettingCalled]() {
        __DBG_STUB_INVOKE__
        initSettingCalled = true;
    });

    stub.set_lamda(&SideBarHelper::registCustomSettingItem, [&registCustomCalled]() {
        __DBG_STUB_INVOKE__
        registCustomCalled = true;
    });

    typedef bool (EventDispatcherManager::*InstallFilterFunc)(EventType, SideBar *, bool (SideBar::*)(quint64));
    stub.set_lamda(static_cast<InstallFilterFunc>(&EventDispatcherManager::installEventFilter),
                   [&installFilterCalled] {
                       __DBG_STUB_INVOKE__
                       installFilterCalled = true;
                       return true;
                   });

    bool result = plugin->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(addConfigCalled);
    EXPECT_TRUE(initSettingCalled);
    EXPECT_TRUE(registCustomCalled);
    EXPECT_TRUE(installFilterCalled);
}

TEST_F(UT_SideBar, Start_ConfigFailed)
{
    stub.set_lamda(&DConfigManager::instance, []() -> DConfigManager * {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::addConfig,
                   [](DConfigManager *, const QString &, QString *err) -> bool {
                       __DBG_STUB_INVOKE__
                       if (err)
                           *err = "Test error";
                       return false;
                   });

    bool result = plugin->start();

    EXPECT_FALSE(result);
}

TEST_F(UT_SideBar, OnWindowOpened)
{
    bool addSideBarCalled = false;
    bool installSideBarCalled = false;
    bool updateVisiableCalled = false;

    quint64 testWindowId = 12345;

    FileManagerWindow mockWindow(QUrl("file:///home"));

    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    stub.set_lamda(&SideBarHelper::addSideBar,
                   [&addSideBarCalled](quint64, SideBarWidget *) {
                       __DBG_STUB_INVOKE__
                       addSideBarCalled = true;
                   });

    stub.set_lamda(&FileManagerWindow::installSideBar,
                   [&installSideBarCalled](FileManagerWindow *, QWidget *) {
                       __DBG_STUB_INVOKE__
                       installSideBarCalled = true;
                   });

    stub.set_lamda(&SideBarWidget::updateItemVisiable,
                   [&updateVisiableCalled](SideBarWidget *, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       updateVisiableCalled = true;
                   });

    stub.set_lamda(&SideBarHelper::hiddenRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        return QVariantMap();
    });

    stub.set_lamda(&SideBarHelper::preDefineItemProperties, []() -> QMap<QUrl, QPair<int, QVariantMap>> {
        __DBG_STUB_INVOKE__
        return QMap<QUrl, QPair<int, QVariantMap>>();
    });

    // Stub dpfSlotChannel for accessibility
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QWidget *, char const(&)[14]);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push), [] {
        __DBG_STUB_INVOKE__
        return QVariant();
    });

    stub.set_lamda(&SideBarView::updateSeparatorVisibleState, [] { __DBG_STUB_INVOKE__ });

    plugin->onWindowOpened(testWindowId);

    EXPECT_TRUE(addSideBarCalled);
    EXPECT_TRUE(installSideBarCalled);
    EXPECT_TRUE(updateVisiableCalled);
}

TEST_F(UT_SideBar, OnWindowClosed_LastWindow)
{
    bool removeSideBarCalled = false;
    bool saveStateCalled = false;

    quint64 testWindowId = 12345;

    FileManagerWindow mockWindow(QUrl("file:///home"));
    SideBarWidget *mockSideBar = new SideBarWidget();

    stub.set_lamda(&FileManagerWindowsManager::windowIdList,
                   [testWindowId](FileManagerWindowsManager *) -> QList<quint64> {
                       __DBG_STUB_INVOKE__
                       return QList<quint64>() << testWindowId;
                   });

    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    stub.set_lamda(&FileManagerWindow::sideBar,
                   [mockSideBar](FileManagerWindow *) {
                       __DBG_STUB_INVOKE__
                       return mockSideBar;
                   });

    stub.set_lamda(&SideBarWidget::saveStateWhenClose,
                   [&saveStateCalled](SideBarWidget *) {
                       __DBG_STUB_INVOKE__
                       saveStateCalled = true;
                   });

    stub.set_lamda(&SideBarHelper::removeSideBar,
                   [&removeSideBarCalled](quint64) {
                       __DBG_STUB_INVOKE__
                       removeSideBarCalled = true;
                   });

    plugin->onWindowClosed(testWindowId);

    EXPECT_TRUE(saveStateCalled);
    EXPECT_TRUE(removeSideBarCalled);

    delete mockSideBar;
}

TEST_F(UT_SideBar, OnWindowClosed_NotLastWindow)
{
    bool removeSideBarCalled = false;

    quint64 testWindowId = 12345;

    stub.set_lamda(&FileManagerWindowsManager::windowIdList,
                   [](FileManagerWindowsManager *) -> QList<quint64> {
                       __DBG_STUB_INVOKE__
                       return QList<quint64>() << 1 << 2;   // Multiple windows
                   });

    stub.set_lamda(&SideBarHelper::removeSideBar,
                   [&removeSideBarCalled](quint64) {
                       __DBG_STUB_INVOKE__
                       removeSideBarCalled = true;
                   });

    plugin->onWindowClosed(testWindowId);

    EXPECT_TRUE(removeSideBarCalled);
}

TEST_F(UT_SideBar, OnConfigChanged_VisiableKey)
{
    bool updateVisiableCalled = false;

    FileManagerWindow mockWindow(QUrl("file:///home"));
    SideBarWidget *mockSideBar = new SideBarWidget();

    stub.set_lamda(&FileManagerWindowsManager::windowIdList,
                   [](FileManagerWindowsManager *) -> QList<quint64> {
                       __DBG_STUB_INVOKE__
                       return QList<quint64>() << 1;
                   });

    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    stub.set_lamda(&FileManagerWindow::sideBar,
                   [mockSideBar](FileManagerWindow *) {
                       __DBG_STUB_INVOKE__
                       return mockSideBar;
                   });

    stub.set_lamda(&SideBarWidget::updateItemVisiable,
                   [&updateVisiableCalled](SideBarWidget *, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       updateVisiableCalled = true;
                   });

    stub.set_lamda(&SideBarHelper::hiddenRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        return QVariantMap();
    });

    plugin->onConfigChanged(ConfigInfos::kConfName, ConfigInfos::kVisiableKey);

    EXPECT_TRUE(updateVisiableCalled);

    delete mockSideBar;
}

TEST_F(UT_SideBar, OnConfigChanged_WrongConfig)
{
    bool updateVisiableCalled = false;

    stub.set_lamda(&SideBarWidget::updateItemVisiable,
                   [&updateVisiableCalled](SideBarWidget *, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       updateVisiableCalled = true;
                   });

    plugin->onConfigChanged("wrong.config", ConfigInfos::kVisiableKey);

    EXPECT_FALSE(updateVisiableCalled);
}

TEST_F(UT_SideBar, OnAboutToShowSettingDialog)
{
    bool resetPanelCalled = false;

    quint64 testWindowId = 12345;

    FileManagerWindow mockWindow(QUrl("file:///home"));
    SideBarWidget *mockSideBar = new SideBarWidget();

    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [&mockWindow](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return &mockWindow;
                   });

    stub.set_lamda(&FileManagerWindow::sideBar,
                   [mockSideBar](FileManagerWindow *) {
                       __DBG_STUB_INVOKE__
                       return mockSideBar;
                   });

    stub.set_lamda(&SideBarWidget::resetSettingPanel,
                   [&resetPanelCalled](SideBarWidget *) {
                       __DBG_STUB_INVOKE__
                       resetPanelCalled = true;
                   });

    bool result = plugin->onAboutToShowSettingDialog(testWindowId);

    EXPECT_FALSE(result);   // Always returns false
    EXPECT_TRUE(resetPanelCalled);

    delete mockSideBar;
}

TEST_F(UT_SideBar, OnAboutToShowSettingDialog_InvalidWindow)
{
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    bool result = plugin->onAboutToShowSettingDialog(99999);

    EXPECT_FALSE(result);
}
