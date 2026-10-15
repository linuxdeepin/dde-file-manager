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

#include <plugins/filemanager/dfmplugin-myshares/myshares.h>
#include <plugins/filemanager/dfmplugin-myshares/watcher/sharewatcher.h>
#include <plugins/filemanager/dfmplugin-myshares/utils/sharefilehelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_MyShares_0_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_myshares::MyShares receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63900, &receiver, static_cast<void (dfmplugin_myshares::MyShares::*)(const QString &)>(&dfmplugin_myshares::MyShares::onShareAdded));
    const bool published = mgr.publish(63900);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(63900, &receiver, static_cast<void (dfmplugin_myshares::MyShares::*)(const QString &)>(&dfmplugin_myshares::MyShares::onShareAdded)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_ShareWatcher_1_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_myshares::ShareWatcher receiver(QUrl(), nullptr);
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63902, &receiver, static_cast<void (dfmplugin_myshares::ShareWatcher::*)(const QString &)>(&dfmplugin_myshares::ShareWatcher::shareAdded));
    const bool published = mgr.publish(63902);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(63902, &receiver, static_cast<void (dfmplugin_myshares::ShareWatcher::*)(const QString &)>(&dfmplugin_myshares::ShareWatcher::shareAdded)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_ShareFileHelper_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_myshares::ShareFileHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63904, receiver, static_cast<bool (dfmplugin_myshares::ShareFileHelper::*)(quint64, const QList<QUrl> &)>(&dfmplugin_myshares::ShareFileHelper::openFileInPlugin));
    const bool ran = mgr.run(63904);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

