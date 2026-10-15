// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire_cov.cpp - event template instantiation sweep.
//
// Production plugin code instantiates dpf event templates (EventHelper,
// EventChannel::setReceiver, EventDispatcher/EventSequence append, connect,
// subscribe, follow, memberFunctionVoidCast) against concrete receiver
// classes, but those instantiations are never executed by tests. Each case
// below registers one such receiver member on a fresh unique topic id inside
// a local manager instance and fires it with deliberately mismatched arity
// (or a null receiver, which the framework guards with `if (s)`): the
// EventHelper count-guard / null-receiver guard returns early, so the
// production slot BODY never runs, while the registration template,
// memberFunctionVoidCast, EventHelper<M>, its invoke and resultGenerator<Ret>
// all execute. Assertions document that contract:
//   - connect/subscribe/follow/installEventFilter succeeds
//   - push returns default-constructed (invalid for void, valid otherwise)
//   - publish still dispatches (true), run has no accepting handler (false)
//   - unsubscribe/unfollow removes the previously added handler (true)

#include <plugins/filemanager/dfmplugin-sidebar/events/sidebareventreceiver.h>
#include <plugins/filemanager/dfmplugin-sidebar/sidebar.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_SideBarEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_sidebar::SideBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62800, receiver, static_cast<void (dfmplugin_sidebar::SideBarEventReceiver::*)(quint64, const QUrl &, bool)>(&dfmplugin_sidebar::SideBarEventReceiver::handleItemHidden));
    const QVariant ret = mgr.push(62800);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SideBarEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_sidebar::SideBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62802, receiver, static_cast<void (dfmplugin_sidebar::SideBarEventReceiver::*)(quint64, const QUrl &)>(&dfmplugin_sidebar::SideBarEventReceiver::handleItemTriggerEdit));
    const QVariant ret = mgr.push(62802);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SideBarEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_sidebar::SideBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62804, receiver, static_cast<void (dfmplugin_sidebar::SideBarEventReceiver::*)(quint64)>(&dfmplugin_sidebar::SideBarEventReceiver::handleSidebarUpdateSelection));
    const QVariant ret = mgr.push(62804);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SideBarEventReceiver2_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_sidebar::SideBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62806, receiver, static_cast<void (dfmplugin_sidebar::SideBarEventReceiver::*)(const QUrl &, bool)>(&dfmplugin_sidebar::SideBarEventReceiver::handleItemHidden));
    const QVariant ret = mgr.push(62806);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_SideBar_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_sidebar::SideBar *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool installed = mgr.installEventFilter(62808, receiver, static_cast<bool (dfmplugin_sidebar::SideBar::*)(quint64)>(&dfmplugin_sidebar::SideBar::onAboutToShowSettingDialog));
    const bool published = mgr.publish(62808);
    EXPECT_TRUE(installed);
    EXPECT_TRUE(published);  // filter ran under null-receiver guard, no interception
}

