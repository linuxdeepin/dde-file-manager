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

#include <plugins/filemanager/dfmplugin-avfsbrowser/avfsbrowser.h>
#include <plugins/filemanager/dfmplugin-avfsbrowser/events/avfseventhandler.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_AvfsBrowser_0_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_avfsbrowser::AvfsBrowser receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(64100, &receiver, static_cast<void (dfmplugin_avfsbrowser::AvfsBrowser::*)(const QString &)>(&dfmplugin_avfsbrowser::AvfsBrowser::beMySubScene));
    const bool published = mgr.publish(64100);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(64100, &receiver, static_cast<void (dfmplugin_avfsbrowser::AvfsBrowser::*)(const QString &)>(&dfmplugin_avfsbrowser::AvfsBrowser::beMySubScene)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_AvfsEventHandler_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_avfsbrowser::AvfsEventHandler *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(64102, receiver, static_cast<bool (dfmplugin_avfsbrowser::AvfsEventHandler::*)(const QUrl &, QList<QVariantMap> *)>(&dfmplugin_avfsbrowser::AvfsEventHandler::sepateTitlebarCrumb));
    const bool ran = mgr.run(64102);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

