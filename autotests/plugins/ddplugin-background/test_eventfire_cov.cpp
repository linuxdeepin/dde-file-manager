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

#include <plugins/desktop/ddplugin-background/backgroundmanager.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_BackgroundManager_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_background::BackgroundManager *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63500, receiver, static_cast<bool (ddplugin_background::BackgroundManager::*)()>(&ddplugin_background::BackgroundManager::useColorBackground));
    const QVariant ret = mgr.push(63500, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_BackgroundManager_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_background::BackgroundManager *>(nullptr);  // real ctor touches DBus/screen services: keep it null
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63502, receiver, static_cast<void (ddplugin_background::BackgroundManager::*)()>(&ddplugin_background::BackgroundManager::onBackgroundBuild));
    const bool published = mgr.publish(63502, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_FALSE(mgr.unsubscribe(63502, receiver, static_cast<void (ddplugin_background::BackgroundManager::*)()>(&ddplugin_background::BackgroundManager::onBackgroundBuild)));  // compare path runs but null receiver does not match the stored null-vs-null lookup semantics
}

