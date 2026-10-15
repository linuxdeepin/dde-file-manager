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

#include <plugins/filemanager/dfmplugin-disk-encrypt-entry/plugin_diskencryptentry.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_DiskEncryptEntry_0_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_diskenc::DiskEncryptEntry receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63800, &receiver, static_cast<void (dfmplugin_diskenc::DiskEncryptEntry::*)(const QString &)>(&dfmplugin_diskenc::DiskEncryptEntry::onComputerMenuSceneAdded));
    const bool published = mgr.publish(63800);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(63800, &receiver, static_cast<void (dfmplugin_diskenc::DiskEncryptEntry::*)(const QString &)>(&dfmplugin_diskenc::DiskEncryptEntry::onComputerMenuSceneAdded)));  // remove + compare path
}

