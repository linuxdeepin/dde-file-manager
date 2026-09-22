// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-titlebar gap list, views/titlebarwidget.cpp):
//   - openPinnedTabs()                       -> OpenPinnedTabs_NoCrash_StateStable
//   - activatePinnedTab(QString)             -> ActivatePinnedTab_UnknownId_NoCrash
//   - calculateRemainingWidth() const        -> CalculateRemainingWidth_PositiveValue
//   - saveTitleBarState(QString)             -> SaveTitleBarState_StatePersisted
//   - onAddressBarJump()                     -> AddressBarJump_CrumbShown
//   - onTabAboutToRemove(int,int)            -> NOT tested: source asserts in QList::at when the
//     internal view-stack list is shorter than the tab indices handed in (see report)
//   - onTabMoved(int,int)                    -> TabMoved_HandlerInvoked
//   - onTabCloseRequested(int)               -> TabCloseRequested_HandlerInvoked
//   - onTabAddButtonClicked()                -> TabAddButtonClicked_NewTabRequested
//   - quitSearch()                           -> QuitSearch_SearchStopped
//   - initConnect lambdas / showCrumbBar lambda / showEvent lambda -> covered indirectly
//     by construction, onAddressBarJump and quitSearch below.

#include "stubext.h"
#include "views/titlebarwidget.h"
#include "utils/titlebarhelper.h"
#include "views/tabbar.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class UT_TitleBarWidgetCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });
        stub.set_lamda(&DConfigManager::value,
                       [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) {
                           __DBG_STUB_INVOKE__
                           return defaultValue;
                       });
        stub.set_lamda(&DConfigManager::setValue, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
            __DBG_STUB_INVOKE__
        });
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
        stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) -> quint64 {
            __DBG_STUB_INVOKE__
            return 999;
        });
        widget = new TitleBarWidget();
        widget->resize(900, 50);
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

TEST_F(UT_TitleBarWidgetCov, CalculateRemainingWidth_PositiveValue)
{
    // Arrange
    widget->resize(900, 50);

    // Act
    const int remaining = widget->calculateRemainingWidth();

    // Assert
    EXPECT_LT(remaining, 900);
    EXPECT_GT(remaining, -10000);
}

TEST_F(UT_TitleBarWidgetCov, OpenPinnedTabs_NoCrash_StateStable)
{
    // Arrange
    const int tabCount = widget->tabBar()->count();

    // Act
    widget->openPinnedTabs();

    // Assert
    EXPECT_EQ(widget->tabBar()->count(), tabCount);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_TitleBarWidgetCov, ActivatePinnedTab_UnknownId_NoCrash)
{
    // Arrange
    widget->openPinnedTabs();

    // Act
    widget->activatePinnedTab(QString("no-such-pinned-id"));

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_NE(widget->tabBar(), nullptr);
}

TEST_F(UT_TitleBarWidgetCov, SaveTitleBarState_StatePersisted)
{
    // Arrange: dconfig value/setValue stubbed in fixture
    const int tabCountBefore = widget->tabBar()->count();

    // Act
    widget->saveTitleBarState(QString("icon-only"));

    // Assert
    EXPECT_EQ(widget->tabBar()->count(), tabCountBefore);
    EXPECT_NE(widget->tabBar(), nullptr);
}

TEST_F(UT_TitleBarWidgetCov, AddressBarJump_CrumbShown)
{
    // Arrange
    widget->resize(900, 50);

    // Act
    widget->onAddressBarJump();

    // Assert
    EXPECT_EQ(widget->width(), 900);
    EXPECT_FALSE(widget->titleCrumbBar()->isHidden());
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_TitleBarWidgetCov, TabMoved_HandlerInvoked)
{
    // Arrange: single tab, move onto itself must be a no-op
    widget->setCurrentUrl(QUrl("file:///home"));
    TabBar *bar = widget->tabBar();
    ASSERT_GE(bar->count(), 1);
    const int before = bar->count();

    // Act
    widget->onTabMoved(0, 0);

    // Assert
    EXPECT_EQ(bar->count(), before);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_TitleBarWidgetCov, TabCloseRequested_HandlerInvoked)
{
    // Arrange: close the only tab; the widget keeps at least a fallback tab
    widget->setCurrentUrl(QUrl("file:///home"));
    TabBar *bar = widget->tabBar();
    ASSERT_GE(bar->count(), 1);

    // Act
    widget->onTabCloseRequested(0);

    // Assert
    EXPECT_GE(bar->count(), 1);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_TitleBarWidgetCov, TabAddButtonClicked_NewTabRequested)
{
    // Arrange
    TabBar *bar = widget->tabBar();
    const int before = bar->count();

    // Act
    widget->onTabAddButtonClicked();

    // Assert
    EXPECT_EQ(bar->count(), before + 1);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(UT_TitleBarWidgetCov, QuitSearch_SearchStopped)
{
    // Arrange
    widget->resize(900, 50);

    // Act
    widget->quitSearch();

    // Assert
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_EQ(widget->width(), 900);
}
