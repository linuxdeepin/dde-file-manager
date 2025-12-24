// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/titlebarwidget.h"
#include "views/tabbar.h"
#include "views/navwidget.h"
#include "views/addressbar.h"
#include "views/searcheditwidget.h"
#include "views/crumbbar.h"
#include "views/optionbuttonbox.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/dpf.h>

#include <DTitlebar>

#include <gtest/gtest.h>
#include <QUrl>
#include <QResizeEvent>
#include <QCloseEvent>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class TitleBarWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        // Stub DConfig to avoid crashes
        stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });

        // Stub setValue to prevent actual config writes
        stub.set_lamda(&DConfigManager::setValue, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
            __DBG_STUB_INVOKE__
        });

        // Stub dpfSlotChannel push
        typedef QVariant (EventChannelManager::*PushFunc1)(const QString &, const QString &, QWidget *, const char(&)[15]);
        stub.set_lamda(static_cast<PushFunc1>(&EventChannelManager::push),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });
        typedef QVariant (EventChannelManager::*PushFunc2)(const QString &, const QString &, QWidget *, const char(&)[17]);
        stub.set_lamda(static_cast<PushFunc2>(&EventChannelManager::push),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });

        widget = new TitleBarWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    TitleBarWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TitleBarWidgetTest, Constructor_Called_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_NE(widget->tabBar(), nullptr);
    EXPECT_NE(widget->navWidget(), nullptr);
    EXPECT_NE(widget->titleCrumbBar(), nullptr);
    EXPECT_NE(widget->titleBar(), nullptr);
}

TEST_F(TitleBarWidgetTest, SetCurrentUrl_ValidUrl_UrlSetAndSignalEmitted)
{
    QUrl testUrl("file:///home/test");
    bool signalEmitted = false;

    QObject::connect(widget, &TitleBarWidget::currentUrlChanged, [&signalEmitted](const QUrl &) {
        signalEmitted = true;
    });

    widget->setCurrentUrl(testUrl);
    EXPECT_EQ(widget->currentUrl(), testUrl);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(TitleBarWidgetTest, SetCurrentUrl_EmptyUrl_UrlSet)
{
    QUrl emptyUrl;
    widget->setCurrentUrl(emptyUrl);
    EXPECT_EQ(widget->currentUrl(), emptyUrl);
}

TEST_F(TitleBarWidgetTest, CurrentUrl_AfterConstruction_ReturnsEmptyUrl)
{
    EXPECT_TRUE(widget->currentUrl().isEmpty());
}

TEST_F(TitleBarWidgetTest, NavWidget_AfterConstruction_ReturnsValidPointer)
{
    NavWidget *navWidget = widget->navWidget();
    EXPECT_NE(navWidget, nullptr);
}

TEST_F(TitleBarWidgetTest, TitleBar_AfterConstruction_ReturnsValidPointer)
{
    DTitlebar *titleBar = widget->titleBar();
    EXPECT_NE(titleBar, nullptr);
}

TEST_F(TitleBarWidgetTest, TabBar_AfterConstruction_ReturnsValidPointer)
{
    TabBar *tabBar = widget->tabBar();
    EXPECT_NE(tabBar, nullptr);
}

TEST_F(TitleBarWidgetTest, TitleCrumbBar_AfterConstruction_ReturnsValidPointer)
{
    CrumbBar *crumbBar = widget->titleCrumbBar();
    EXPECT_NE(crumbBar, nullptr);
}

TEST_F(TitleBarWidgetTest, OpenNewTab_EmptyUrl_CreatesTabWithHomeUrl)
{
    bool createTabCalled = false;
    bool sendCdCalled = false;

    stub.set_lamda(&TabBar::appendTab, [&createTabCalled](TabBar *) {
        __DBG_STUB_INVOKE__
        createTabCalled = true;
        return 0;
    });

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&sendCdCalled](EventDispatcherManager *, EventType, quint64, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       sendCdCalled = true;
                       return true;
                   });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    widget->openNewTab(QUrl());
    EXPECT_TRUE(createTabCalled);
    EXPECT_TRUE(sendCdCalled);
}

TEST_F(TitleBarWidgetTest, OpenNewTab_ValidUrl_CreatesTabAndNavigates)
{
    QUrl testUrl("file:///home/test");
    bool createTabCalled = false;
    bool sendCdCalled = false;

    stub.set_lamda(&TabBar::appendTab, [&createTabCalled](TabBar *) {
        __DBG_STUB_INVOKE__
        createTabCalled = true;
        return 0;
    });

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&sendCdCalled](EventDispatcherManager *, EventType, quint64, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       sendCdCalled = true;
                       return true;
                   });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    widget->openNewTab(testUrl);
    EXPECT_TRUE(createTabCalled);
    EXPECT_TRUE(sendCdCalled);
}

TEST_F(TitleBarWidgetTest, OpenCustomFixedTabs_ValidUrls_CreatesInactiveTabs)
{
    QStringList testUrls;
    testUrls << "file:///home/test1"
             << "file:///home/test2";

    int createCount = 0;
    int setUserDataCount = 0;

    stub.set_lamda(&DConfigManager::value, [&testUrls](DConfigManager *, const QString &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "dfm.custom.fixedtab")
            return QVariant(testUrls);
        return QVariant();
    });

    stub.set_lamda(&TabBar::appendInactiveTab, [&createCount] {
        __DBG_STUB_INVOKE__
        return createCount++;
    });

    stub.set_lamda(&TabBar::setTabUserData, [&setUserDataCount](TabBar *, int, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        setUserDataCount++;
    });

    widget->openCustomFixedTabs();
    EXPECT_EQ(createCount, 2);
    EXPECT_EQ(setUserDataCount, 2);
}

TEST_F(TitleBarWidgetTest, OpenCustomFixedTabs_EmptyList_NoTabsCreated)
{
    bool createCalled = false;

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    stub.set_lamda(&TabBar::appendInactiveTab, [&createCalled] {
        __DBG_STUB_INVOKE__
        createCalled = true;
        return 0;
    });

    widget->openCustomFixedTabs();
    EXPECT_FALSE(createCalled);
}

TEST_F(TitleBarWidgetTest, HandleSplitterAnimation_ValidPosition_UpdatesPlaceholderWidth)
{
    QVariant position(50);
    widget->handleSplitterAnimation(position);
    // Verify it doesn't crash
}

TEST_F(TitleBarWidgetTest, HandleSplitterAnimation_ZeroPosition_SetsMaxWidth)
{
    QVariant position(0);
    widget->handleSplitterAnimation(position);
    // Verify it doesn't crash
}

TEST_F(TitleBarWidgetTest, HandleSplitterAnimation_LargePosition_SetsMinWidth)
{
    QVariant position(100);
    widget->handleSplitterAnimation(position);
    // Verify it doesn't crash
}

TEST_F(TitleBarWidgetTest, HandleHotkeyCtrlF_Called_ActivatesSearchEdit)
{
    bool activateCalled = false;

    stub.set_lamda(&SearchEditWidget::activateEdit, [&activateCalled] {
        __DBG_STUB_INVOKE__
        activateCalled = true;
    });

    widget->handleHotkeyCtrlF();
    EXPECT_TRUE(activateCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotkeyCtrlL_Called_ShowsAddressBar)
{
    QUrl testUrl("file:///home/test");
    widget->setCurrentUrl(testUrl);

    bool showCalled = false;

    stub.set_lamda(&AddressBar::show, [&showCalled] {
        __DBG_STUB_INVOKE__
        showCalled = true;
    });

    stub.set_lamda(qOverload<>(&AddressBar::setFocus), [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&AddressBar::setCurrentUrl, [](AddressBar *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&CrumbBar::hide, [] {
        __DBG_STUB_INVOKE__
    });

    widget->handleHotkeyCtrlL();
    EXPECT_TRUE(showCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotketSwitchViewMode_Mode0_SendsIconMode)
{
    bool eventSent = false;
    Global::ViewMode sentMode = Global::ViewMode::kListMode;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, int &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&eventSent, &sentMode](EventDispatcherManager *, EventType, quint64, int mode) {
                       __DBG_STUB_INVOKE__
                       eventSent = true;
                       sentMode = static_cast<Global::ViewMode>(mode);
                       return true;
                   });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    widget->handleHotketSwitchViewMode(0);
    EXPECT_TRUE(eventSent);
    EXPECT_EQ(sentMode, Global::ViewMode::kIconMode);
}

TEST_F(TitleBarWidgetTest, HandleHotketSwitchViewMode_Mode1_SendsListMode)
{
    bool eventSent = false;
    Global::ViewMode sentMode = Global::ViewMode::kIconMode;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, int &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&eventSent, &sentMode](EventDispatcherManager *, EventType, quint64, int mode) {
                       __DBG_STUB_INVOKE__
                       eventSent = true;
                       sentMode = static_cast<Global::ViewMode>(mode);
                       return true;
                   });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    widget->handleHotketSwitchViewMode(1);
    EXPECT_TRUE(eventSent);
    EXPECT_EQ(sentMode, Global::ViewMode::kListMode);
}

TEST_F(TitleBarWidgetTest, HandleHotketSwitchViewMode_Mode2_SendsTreeMode)
{
    bool eventSent = false;
    Global::ViewMode sentMode = Global::ViewMode::kIconMode;

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "dfm.treeview.enable")
            return QVariant(true);
        return QVariant();
    });

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, int &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&eventSent, &sentMode](EventDispatcherManager *, EventType, quint64, int mode) {
                       __DBG_STUB_INVOKE__
                       eventSent = true;
                       sentMode = static_cast<Global::ViewMode>(mode);
                       return true;
                   });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    widget->handleHotketSwitchViewMode(2);
    EXPECT_TRUE(eventSent);
    EXPECT_EQ(sentMode, Global::ViewMode::kTreeMode);
}

TEST_F(TitleBarWidgetTest, HandleHotketSwitchViewMode_Mode2TreeDisabled_DoesNotSendEvent)
{
    bool eventSent = false;

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &key, const QVariant &) {
        __DBG_STUB_INVOKE__
        if (key == "treeViewEnable")
            return QVariant(false);
        return QVariant();
    });

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, int &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&eventSent](EventDispatcherManager *, EventType, quint64, int) {
                       __DBG_STUB_INVOKE__
                       eventSent = true;
                       return true;
                   });

    widget->handleHotketSwitchViewMode(2);
    EXPECT_FALSE(eventSent);
}

TEST_F(TitleBarWidgetTest, HandleHotketCloseCurrentTab_OnlyOneTab_ClosesWindow)
{
    bool windowCloseCalled = false;

    stub.set_lamda(&TabBar::count, [] {
        __DBG_STUB_INVOKE__
        return 1;
    });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&] {
        __DBG_STUB_INVOKE__
        windowCloseCalled = true;
        return static_cast<FileManagerWindow *>(nullptr);
    });

    widget->handleHotketCloseCurrentTab();
    EXPECT_TRUE(windowCloseCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotketCloseCurrentTab_MultipleTabs_RemovesTab)
{
    bool removeTabCalled = false;

    stub.set_lamda(&TabBar::count, [] {
        __DBG_STUB_INVOKE__
        return 3;
    });

    stub.set_lamda(&TabBar::currentIndex, [] {
        __DBG_STUB_INVOKE__
        return 1;
    });

    stub.set_lamda(&TabBar::removeTab, [&removeTabCalled](TabBar *, int, int) {
        __DBG_STUB_INVOKE__
        removeTabCalled = true;
    });

    widget->handleHotketCloseCurrentTab();
    EXPECT_TRUE(removeTabCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotketNextTab_Called_ActivatesNextTab)
{
    bool nextTabCalled = false;

    stub.set_lamda(&TabBar::activateNextTab, [&nextTabCalled](TabBar *) {
        __DBG_STUB_INVOKE__
        nextTabCalled = true;
    });

    widget->handleHotketNextTab();
    EXPECT_TRUE(nextTabCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotketPreviousTab_Called_ActivatesPreviousTab)
{
    bool previousTabCalled = false;

    stub.set_lamda(&TabBar::activatePreviousTab, [&previousTabCalled](TabBar *) {
        __DBG_STUB_INVOKE__
        previousTabCalled = true;
    });

    widget->handleHotketPreviousTab();
    EXPECT_TRUE(previousTabCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotketCreateNewTab_NoSelection_CreatesTabWithCurrentUrl)
{
    QUrl currentUrl("file:///home/test");
    widget->setCurrentUrl(currentUrl);

    bool createTabCalled = false;

    stub.set_lamda(&TabBar::appendTab, [&createTabCalled](TabBar *) {
        __DBG_STUB_INVOKE__
        createTabCalled = true;
        return 0;
    });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    typedef QVariant (DPF_NAMESPACE::EventChannelManager::*PushFunc)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<PushFunc>(&DPF_NAMESPACE::EventChannelManager::push),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QVariant::fromValue(QList<QUrl>());
                   });

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [](EventDispatcherManager *, EventType, quint64, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    widget->handleHotketCreateNewTab();
    EXPECT_TRUE(createTabCalled);
}

TEST_F(TitleBarWidgetTest, HandleCreateTabList_ValidUrls_CreatesTabsForDirectories)
{
    QList<QUrl> urlList;
    urlList << QUrl("file:///home/test1") << QUrl("file:///home/test2");

    int createTabCount = 0;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&TabBar::appendTab, [&createTabCount](TabBar *) {
        __DBG_STUB_INVOKE__
        createTabCount++;
        return createTabCount;
    });

    stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [](EventDispatcherManager *, EventType, quint64, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    widget->handleCreateTabList(urlList);
    EXPECT_EQ(createTabCount, 2);
}

TEST_F(TitleBarWidgetTest, HandleCreateTabList_EmptyList_NoTabsCreated)
{
    QList<QUrl> emptyList;
    bool createTabCalled = false;

    stub.set_lamda(&TabBar::appendTab, [&createTabCalled](TabBar *) {
        __DBG_STUB_INVOKE__
        createTabCalled = true;
        return 0;
    });

    widget->handleCreateTabList(emptyList);
    EXPECT_FALSE(createTabCalled);
}

TEST_F(TitleBarWidgetTest, HandleHotketActivateTab_ValidIndex_ActivatesTab)
{
    bool setCurrentIndexCalled = false;

    stub.set_lamda(&TabBar::setCurrentIndex, [&setCurrentIndexCalled] {
        __DBG_STUB_INVOKE__
        setCurrentIndexCalled = true;
    });

    widget->handleHotketActivateTab(2);
    EXPECT_TRUE(setCurrentIndexCalled);
}

TEST_F(TitleBarWidgetTest, ShowSearchFilterButton_Visible_SetsButtonVisible)
{
    bool setVisibleCalled = false;

    stub.set_lamda(&SearchEditWidget::setAdvancedButtonVisible, [&setVisibleCalled](SearchEditWidget *, bool visible) {
        __DBG_STUB_INVOKE__
        setVisibleCalled = true;
        EXPECT_TRUE(visible);
    });

    widget->showSearchFilterButton(true);
    EXPECT_TRUE(setVisibleCalled);
}

TEST_F(TitleBarWidgetTest, ShowSearchFilterButton_Hidden_SetsButtonHidden)
{
    bool setVisibleCalled = false;

    stub.set_lamda(&SearchEditWidget::setAdvancedButtonVisible, [&setVisibleCalled](SearchEditWidget *, bool visible) {
        __DBG_STUB_INVOKE__
        setVisibleCalled = true;
        EXPECT_FALSE(visible);
    });

    widget->showSearchFilterButton(false);
    EXPECT_TRUE(setVisibleCalled);
}

TEST_F(TitleBarWidgetTest, SetViewModeState_IconMode_SetsMode)
{
    bool setModeCalled = false;

    stub.set_lamda(&OptionButtonBox::setViewMode, [&setModeCalled](OptionButtonBox *, int mode) {
        __DBG_STUB_INVOKE__
        setModeCalled = true;
        EXPECT_EQ(mode, static_cast<int>(Global::ViewMode::kIconMode));
    });

    widget->setViewModeState(static_cast<int>(Global::ViewMode::kIconMode));
    EXPECT_TRUE(setModeCalled);
}

TEST_F(TitleBarWidgetTest, SetViewModeState_ListMode_SetsMode)
{
    bool setModeCalled = false;

    stub.set_lamda(&OptionButtonBox::setViewMode, [&setModeCalled](OptionButtonBox *, int mode) {
        __DBG_STUB_INVOKE__
        setModeCalled = true;
        EXPECT_EQ(mode, static_cast<int>(Global::ViewMode::kListMode));
    });

    widget->setViewModeState(static_cast<int>(Global::ViewMode::kListMode));
    EXPECT_TRUE(setModeCalled);
}

TEST_F(TitleBarWidgetTest, ResizeEvent_Called_UpdatesChildWidgets)
{
    bool optionBoxUpdateCalled = false;
    bool searchWidgetUpdateCalled = false;

    stub.set_lamda(&OptionButtonBox::updateOptionButtonBox, [&optionBoxUpdateCalled](OptionButtonBox *, int) {
        __DBG_STUB_INVOKE__
        optionBoxUpdateCalled = true;
    });

    stub.set_lamda(&SearchEditWidget::updateSearchEditWidget, [&searchWidgetUpdateCalled](SearchEditWidget *, int) {
        __DBG_STUB_INVOKE__
        searchWidgetUpdateCalled = true;
    });

    QResizeEvent event(QSize(800, 600), QSize(600, 400));
    widget->resizeEvent(&event);

    EXPECT_TRUE(optionBoxUpdateCalled);
    EXPECT_TRUE(searchWidgetUpdateCalled);
}

TEST_F(TitleBarWidgetTest, EventFilter_AddressBarHide_ShowsCrumbBar)
{
    bool showCalled = false;

    stub.set_lamda(&CrumbBar::show, [&showCalled] {
        __DBG_STUB_INVOKE__
        showCalled = true;
    });

    QEvent hideEvent(QEvent::Hide);
    bool filtered = widget->eventFilter(widget->addressBar, &hideEvent);

    EXPECT_TRUE(filtered);
    EXPECT_TRUE(showCalled);
}

TEST_F(TitleBarWidgetTest, EventFilter_OtherObject_NotFiltered)
{
    QObject otherObject;
    QEvent event(QEvent::Show);
    bool filtered = widget->eventFilter(&otherObject, &event);
    EXPECT_FALSE(filtered);
}
