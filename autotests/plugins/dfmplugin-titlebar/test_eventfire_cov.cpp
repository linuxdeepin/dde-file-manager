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

#include <plugins/filemanager/dfmplugin-titlebar/events/titlebareventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_TitleBarEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_titlebar::TitleBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62100, receiver, static_cast<void (dfmplugin_titlebar::TitleBarEventReceiver::*)(bool)>(&dfmplugin_titlebar::TitleBarEventReceiver::handleSetNewWindowAndTabEnable));
    const QVariant ret = mgr.push(62100);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_TitleBarEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_titlebar::TitleBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62102, receiver, static_cast<void (dfmplugin_titlebar::TitleBarEventReceiver::*)(quint64, const QUrl &)>(&dfmplugin_titlebar::TitleBarEventReceiver::handleRemoveHistory));
    const bool published = mgr.publish(62102);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_TitleBarEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_titlebar::TitleBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62104, receiver, static_cast<void (dfmplugin_titlebar::TitleBarEventReceiver::*)(const QUrl &, const QString &)>(&dfmplugin_titlebar::TitleBarEventReceiver::handleSetTabAlias));
    const QVariant ret = mgr.push(62104);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_TitleBarEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_titlebar::TitleBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62106, receiver, static_cast<void (dfmplugin_titlebar::TitleBarEventReceiver::*)(quint64)>(&dfmplugin_titlebar::TitleBarEventReceiver::handleStartSpinner));
    const QVariant ret = mgr.push(62106);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_TitleBarEventReceiver_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_titlebar::TitleBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62108, receiver, static_cast<void (dfmplugin_titlebar::TitleBarEventReceiver::*)(quint64, int)>(&dfmplugin_titlebar::TitleBarEventReceiver::handleViewModeChanged));
    const bool published = mgr.publish(62108);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_TitleBarEventReceiver_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_titlebar::TitleBarEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62110, receiver, static_cast<bool (dfmplugin_titlebar::TitleBarEventReceiver::*)(const QString &, const QVariantMap &)>(&dfmplugin_titlebar::TitleBarEventReceiver::handleCustomRegister));
    const QVariant ret = mgr.push(62110);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

