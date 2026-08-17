// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "events/titlebareventreceiver.h"
#include "views/titlebarwidget.h"
#include "views/navwidget.h"
#include "views/tabbar.h"
#include "views/crumbbar.h"
#include "utils/titlebarhelper.h"
#include "utils/crumbmanager.h"
#include "utils/crumbinterface.h"
#include "utils/optionbuttonmanager.h"

#include <dfm-base/utils/universalutils.h>

#include <gtest/gtest.h>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
using namespace dfmbase::Global;

class TitleBarEventReceiverTest : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = TitleBarEventReceiver::instance();
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    TitleBarEventReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TitleBarEventReceiverTest, Instance_Singleton_ReturnsSameInstance)
{
    auto receiver1 = TitleBarEventReceiver::instance();
    auto receiver2 = TitleBarEventReceiver::instance();
    EXPECT_EQ(receiver1, receiver2);
    EXPECT_NE(receiver1, nullptr);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_ValidScheme_ReturnsTrue)
{
    QString scheme = "testscheme";
    QVariantMap properties;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    bool result = receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(result);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_AlreadyRegistered_ReturnsFalse)
{
    QString scheme = "testscheme";
    QVariantMap properties;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = receiver->handleCustomRegister(scheme, properties);
    EXPECT_FALSE(result);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_KeepAddressBarTrue_RegistersScheme)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[CustomKey::kKeepAddressBar] = true;

    bool registerCalled = false;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TitleBarHelper::registerKeepTitleStatusScheme, [&registerCalled](const QString &) {
        __DBG_STUB_INVOKE__
        registerCalled = true;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(registerCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_HideDetailSpaceBtn_SetsState)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[CustomKey::kHideDetailSpaceBtn] = true;

    bool setStateCalled = false;
    OptionButtonManager::OptBtnVisibleState capturedState = OptionButtonManager::kDoNotHide;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&OptionButtonManager::setOptBtnVisibleState, [&setStateCalled, &capturedState](OptionButtonManager *, const QString &, OptionButtonManager::OptBtnVisibleState state) {
        __DBG_STUB_INVOKE__
        setStateCalled = true;
        capturedState = state;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(setStateCalled);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideDetailSpaceBtn);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_HideListViewBtn_SetsState)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[ViewCustomKeys::kSupportListMode] = false;

    bool setStateCalled = false;
    OptionButtonManager::OptBtnVisibleState capturedState = OptionButtonManager::kDoNotHide;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&OptionButtonManager::setOptBtnVisibleState, [&setStateCalled, &capturedState](OptionButtonManager *, const QString &, OptionButtonManager::OptBtnVisibleState state) {
        __DBG_STUB_INVOKE__
        setStateCalled = true;
        capturedState = state;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(setStateCalled);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideListViewBtn);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_HideIconViewBtn_SetsState)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[ViewCustomKeys::kSupportIconMode] = false;

    bool setStateCalled = false;
    OptionButtonManager::OptBtnVisibleState capturedState = OptionButtonManager::kDoNotHide;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&OptionButtonManager::setOptBtnVisibleState, [&setStateCalled, &capturedState](OptionButtonManager *, const QString &, OptionButtonManager::OptBtnVisibleState state) {
        __DBG_STUB_INVOKE__
        setStateCalled = true;
        capturedState = state;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(setStateCalled);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideIconViewBtn);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_HideTreeViewBtn_SetsState)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[ViewCustomKeys::kSupportTreeMode] = false;

    bool setStateCalled = false;
    OptionButtonManager::OptBtnVisibleState capturedState = OptionButtonManager::kDoNotHide;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&OptionButtonManager::setOptBtnVisibleState, [&setStateCalled, &capturedState](OptionButtonManager *, const QString &, OptionButtonManager::OptBtnVisibleState state) {
        __DBG_STUB_INVOKE__
        setStateCalled = true;
        capturedState = state;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(setStateCalled);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideTreeViewBtn);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_HideListHeightOpt_SetsState)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[ViewCustomKeys::kAllowChangeListHeight] = false;

    bool setStateCalled = false;
    OptionButtonManager::OptBtnVisibleState capturedState = OptionButtonManager::kDoNotHide;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&OptionButtonManager::setOptBtnVisibleState, [&setStateCalled, &capturedState](OptionButtonManager *, const QString &, OptionButtonManager::OptBtnVisibleState state) {
        __DBG_STUB_INVOKE__
        setStateCalled = true;
        capturedState = state;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(setStateCalled);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideListHeightOpt);
}

TEST_F(TitleBarEventReceiverTest, HandleCustomRegister_MultipleFlags_CombinesStates)
{
    QString scheme = "testscheme";
    QVariantMap properties;
    properties[CustomKey::kHideDetailSpaceBtn] = true;
    properties[ViewCustomKeys::kSupportListMode] = false;
    properties[ViewCustomKeys::kSupportIconMode] = false;

    bool setStateCalled = false;
    OptionButtonManager::OptBtnVisibleState capturedState = OptionButtonManager::kDoNotHide;

    stub.set_lamda(&CrumbManager::isRegistered, [](CrumbManager *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&CrumbManager::registerCrumbCreator, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&OptionButtonManager::setOptBtnVisibleState, [&setStateCalled, &capturedState](OptionButtonManager *, const QString &, OptionButtonManager::OptBtnVisibleState state) {
        __DBG_STUB_INVOKE__
        setStateCalled = true;
        capturedState = state;
    });

    receiver->handleCustomRegister(scheme, properties);
    EXPECT_TRUE(setStateCalled);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideDetailSpaceBtn);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideListViewBtn);
    EXPECT_TRUE(capturedState & OptionButtonManager::kHideIconViewBtn);
}

TEST_F(TitleBarEventReceiverTest, HandleStartSpinner_ValidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    receiver->handleStartSpinner(windowId);
    // Function is empty, just verify it doesn't crash
}

TEST_F(TitleBarEventReceiverTest, HandleStopSpinner_ValidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    receiver->handleStopSpinner(windowId);
    // Function is empty, just verify it doesn't crash
}

TEST_F(TitleBarEventReceiverTest, HandleShowFilterButton_ValidWindowId_ShowsButton)
{
    quint64 windowId = 12345;
    bool visible = true;
    bool showCalled = false;

    TitleBarWidget mockWidget;
    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    stub.set_lamda(&TitleBarWidget::showSearchFilterButton, [&showCalled](TitleBarWidget *, bool vis) {
        __DBG_STUB_INVOKE__
        showCalled = true;
        EXPECT_TRUE(vis);
    });

    receiver->handleShowFilterButton(windowId, visible);
    EXPECT_TRUE(showCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleShowFilterButton_InvalidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    bool visible = true;
    bool showCalled = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&TitleBarWidget::showSearchFilterButton, [&showCalled](TitleBarWidget *, bool) {
        __DBG_STUB_INVOKE__
        showCalled = true;
    });

    receiver->handleShowFilterButton(windowId, visible);
    EXPECT_FALSE(showCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleViewModeChanged_ValidWindowId_ChangesMode)
{
    quint64 windowId = 12345;
    int mode = 1;
    bool modeChangedCalled = false;

    TitleBarWidget mockWidget;
    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    stub.set_lamda(&TitleBarWidget::setViewModeState, [&modeChangedCalled](TitleBarWidget *, int m) {
        __DBG_STUB_INVOKE__
        modeChangedCalled = true;
        EXPECT_EQ(m, 1);
    });

    receiver->handleViewModeChanged(windowId, mode);
    EXPECT_TRUE(modeChangedCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleViewModeChanged_InvalidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    int mode = 1;
    bool modeChangedCalled = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&TitleBarWidget::setViewModeState, [&modeChangedCalled](TitleBarWidget *, int) {
        __DBG_STUB_INVOKE__
        modeChangedCalled = true;
    });

    receiver->handleViewModeChanged(windowId, mode);
    EXPECT_FALSE(modeChangedCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleSetNewWindowAndTabEnable_True_SetsEnabled)
{
    receiver->handleSetNewWindowAndTabEnable(true);
    EXPECT_TRUE(TitleBarHelper::newWindowAndTabEnabled);
}

TEST_F(TitleBarEventReceiverTest, HandleSetNewWindowAndTabEnable_False_SetsDisabled)
{
    receiver->handleSetNewWindowAndTabEnable(false);
    EXPECT_FALSE(TitleBarHelper::newWindowAndTabEnabled);
}

TEST_F(TitleBarEventReceiverTest, HandleWindowForward_ValidWindowId_NavigatesForward)
{
    quint64 windowId = 12345;
    bool forwardCalled = false;

    TitleBarWidget mockWidget;
    NavWidget mockNavWidget;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    stub.set_lamda(&TitleBarWidget::navWidget, [&mockNavWidget](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockNavWidget;
    });

    stub.set_lamda(&NavWidget::forward, [&forwardCalled](NavWidget *) {
        __DBG_STUB_INVOKE__
        forwardCalled = true;
    });

    receiver->handleWindowForward(windowId);
    EXPECT_TRUE(forwardCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleWindowForward_InvalidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    bool forwardCalled = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&NavWidget::forward, [&forwardCalled](NavWidget *) {
        __DBG_STUB_INVOKE__
        forwardCalled = true;
    });

    receiver->handleWindowForward(windowId);
    EXPECT_FALSE(forwardCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleWindowBackward_ValidWindowId_NavigatesBackward)
{
    quint64 windowId = 12345;
    bool backwardCalled = false;

    TitleBarWidget mockWidget;
    NavWidget mockNavWidget;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    stub.set_lamda(&TitleBarWidget::navWidget, [&mockNavWidget](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockNavWidget;
    });

    stub.set_lamda(&NavWidget::back, [&backwardCalled](NavWidget *) {
        __DBG_STUB_INVOKE__
        backwardCalled = true;
    });

    receiver->handleWindowBackward(windowId);
    EXPECT_TRUE(backwardCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleWindowBackward_InvalidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    bool backwardCalled = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&NavWidget::back, [&backwardCalled](NavWidget *) {
        __DBG_STUB_INVOKE__
        backwardCalled = true;
    });

    receiver->handleWindowBackward(windowId);
    EXPECT_FALSE(backwardCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleRemoveHistory_ValidWindowId_RemovesUrl)
{
    quint64 windowId = 12345;
    QUrl url("file:///home/test");
    bool removeCalled = false;

    TitleBarWidget mockWidget;
    NavWidget mockNavWidget;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    stub.set_lamda(&TitleBarWidget::navWidget, [&mockNavWidget](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockNavWidget;
    });

    stub.set_lamda(&NavWidget::removeUrlFromHistoryStack, [&removeCalled](NavWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        removeCalled = true;
    });

    receiver->handleRemoveHistory(windowId, url);
    EXPECT_TRUE(removeCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleRemoveHistory_InvalidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    QUrl url("file:///home/test");
    bool removeCalled = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&NavWidget::removeUrlFromHistoryStack, [&removeCalled](NavWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        removeCalled = true;
    });

    receiver->handleRemoveHistory(windowId, url);
    EXPECT_FALSE(removeCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleTabAddable_ValidWindowId_ReturnsTrue)
{
    quint64 windowId = 12345;

    TitleBarWidget mockWidget;
    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    bool result = receiver->handleTabAddable(windowId);
    EXPECT_TRUE(result);
}

TEST_F(TitleBarEventReceiverTest, HandleTabAddable_InvalidWindowId_ReturnsFalse)
{
    quint64 windowId = 12345;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    bool result = receiver->handleTabAddable(windowId);
    EXPECT_FALSE(result);
}

TEST_F(TitleBarEventReceiverTest, HandleCloseTabs_ValidUrl_ClosesMatchingTabs)
{
    QUrl url("file:///home/test");
    bool closeTabCalled = false;

    QList<TitleBarWidget *> mockWidgets;
    TitleBarWidget mockWidget1;
    TitleBarWidget mockWidget2;
    mockWidgets << &mockWidget1 << &mockWidget2;

    TabBar mockTabBar;

    stub.set_lamda(&TitleBarHelper::titlebars, [&mockWidgets]() {
        __DBG_STUB_INVOKE__
        return mockWidgets;
    });

    stub.set_lamda(&TitleBarWidget::tabBar, [&mockTabBar](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockTabBar;
    });

    stub.set_lamda(&TabBar::closeTab, [&closeTabCalled](TabBar *, const QUrl &) {
        __DBG_STUB_INVOKE__
        closeTabCalled = true;
    });

    receiver->handleCloseTabs(url);
    EXPECT_TRUE(closeTabCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleCloseTabs_EmptyUrl_ClosesTabsInAllWindows)
{
    QUrl emptyUrl;
    int closeTabCallCount = 0;

    QList<TitleBarWidget *> mockWidgets;
    TitleBarWidget mockWidget1;
    TitleBarWidget mockWidget2;
    mockWidgets << &mockWidget1 << &mockWidget2;

    TabBar mockTabBar;

    stub.set_lamda(&TitleBarHelper::titlebars, [&mockWidgets]() {
        __DBG_STUB_INVOKE__
        return mockWidgets;
    });

    stub.set_lamda(&TitleBarWidget::tabBar, [&mockTabBar](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockTabBar;
    });

    stub.set_lamda(&TabBar::closeTab, [&closeTabCallCount](TabBar *, const QUrl &) {
        __DBG_STUB_INVOKE__
        closeTabCallCount++;
    });

    receiver->handleCloseTabs(emptyUrl);
    EXPECT_EQ(closeTabCallCount, 2);
}

TEST_F(TitleBarEventReceiverTest, HandleSetTabAlias_ValidUrl_SetsAliasForMatchingTabs)
{
    QUrl url("file:///home/test");
    QString name = "My Tab";
    int setAliasCallCount = 0;

    QList<TitleBarWidget *> mockWidgets;
    TitleBarWidget mockWidget;
    mockWidgets << &mockWidget;

    TabBar mockTabBar;

    stub.set_lamda(&TitleBarHelper::titlebars, [&mockWidgets]() {
        __DBG_STUB_INVOKE__
        return mockWidgets;
    });

    stub.set_lamda(&TitleBarWidget::tabBar, [&mockTabBar](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockTabBar;
    });

    stub.set_lamda(&TabBar::count, [] {
        __DBG_STUB_INVOKE__
        return 2;
    });

    stub.set_lamda(&TabBar::tabUrl, [&url](TabBar *, int) {
        __DBG_STUB_INVOKE__
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&TabBar::setTabAlias, [&setAliasCallCount](TabBar *, int, const QString &) {
        __DBG_STUB_INVOKE__
        setAliasCallCount++;
    });

    receiver->handleSetTabAlias(url, name);
    EXPECT_EQ(setAliasCallCount, 2);
}

TEST_F(TitleBarEventReceiverTest, HandleSetTabAlias_NoMatchingTabs_DoesNotSetAlias)
{
    QUrl url("file:///home/test");
    QString name = "My Tab";
    bool setAliasCalled = false;

    QList<TitleBarWidget *> mockWidgets;
    TitleBarWidget mockWidget;
    mockWidgets << &mockWidget;

    TabBar mockTabBar;

    stub.set_lamda(&TitleBarHelper::titlebars, [&mockWidgets]() {
        __DBG_STUB_INVOKE__
        return mockWidgets;
    });

    stub.set_lamda(&TitleBarWidget::tabBar, [&mockTabBar](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockTabBar;
    });

    stub.set_lamda(&TabBar::count, [] {
        __DBG_STUB_INVOKE__
        return 2;
    });

    stub.set_lamda(&TabBar::tabUrl, [](TabBar *, int) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/other");
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&TabBar::setTabAlias, [&setAliasCalled](TabBar *, int, const QString &) {
        __DBG_STUB_INVOKE__
        setAliasCalled = true;
    });

    receiver->handleSetTabAlias(url, name);
    EXPECT_FALSE(setAliasCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleOpenNewTabTriggered_ValidWindowId_OpensTab)
{
    quint64 windowId = 12345;
    QUrl url("file:///home/test");
    bool openTabCalled = false;

    TitleBarWidget mockWidget;
    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [&mockWidget](quint64) {
        __DBG_STUB_INVOKE__
        return &mockWidget;
    });

    stub.set_lamda(&TitleBarWidget::openNewTab, [&openTabCalled](TitleBarWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        openTabCalled = true;
    });

    receiver->handleOpenNewTabTriggered(windowId, url);
    EXPECT_TRUE(openTabCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleOpenNewTabTriggered_InvalidWindowId_DoesNothing)
{
    quint64 windowId = 12345;
    QUrl url("file:///home/test");
    bool openTabCalled = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&TitleBarWidget::openNewTab, [&openTabCalled](TitleBarWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        openTabCalled = true;
    });

    receiver->handleOpenNewTabTriggered(windowId, url);
    EXPECT_FALSE(openTabCalled);
}

TEST_F(TitleBarEventReceiverTest, HandleUpdateCrumb_ValidUrl_UpdatesCrumbBars)
{
    QUrl url("file:///home/test");
    int updateCallCount = 0;

    QList<TitleBarWidget *> mockWidgets;
    TitleBarWidget mockWidget1;
    TitleBarWidget mockWidget2;
    mockWidgets << &mockWidget1 << &mockWidget2;

    CrumbBar mockCrumbBar;

    stub.set_lamda(&TitleBarHelper::titlebars, [&mockWidgets]() {
        __DBG_STUB_INVOKE__
        return mockWidgets;
    });

    stub.set_lamda(&TitleBarWidget::titleCrumbBar, [&mockCrumbBar](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return &mockCrumbBar;
    });

    stub.set_lamda(&CrumbBar::lastUrl, [&url](CrumbBar *) {
        __DBG_STUB_INVOKE__
        return url;
    });

    stub.set_lamda(&CrumbBar::onUrlChanged, [&updateCallCount](CrumbBar *, const QUrl &) {
        __DBG_STUB_INVOKE__
        updateCallCount++;
    });

    receiver->handleUpdateCrumb(url);
    EXPECT_EQ(updateCallCount, 2);
}

TEST_F(TitleBarEventReceiverTest, HandleUpdateCrumb_NullCrumbBar_DoesNotCrash)
{
    QUrl url("file:///home/test");

    QList<TitleBarWidget *> mockWidgets;
    TitleBarWidget mockWidget;
    mockWidgets << &mockWidget;

    stub.set_lamda(&TitleBarHelper::titlebars, [&mockWidgets]() {
        __DBG_STUB_INVOKE__
        return mockWidgets;
    });

    stub.set_lamda(&TitleBarWidget::titleCrumbBar, [](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<CrumbBar *>(nullptr);
    });

    receiver->handleUpdateCrumb(url);
    // Should not crash
}
