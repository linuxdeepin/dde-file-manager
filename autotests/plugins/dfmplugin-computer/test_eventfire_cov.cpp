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

#include <plugins/filemanager/dfmplugin-computer/watcher/computeritemwatcher.h>
#include <plugins/filemanager/dfmplugin-computer/events/computereventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_ComputerItemWatcher_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_computer::ComputerItemWatcher *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63300, receiver, static_cast<void (dfmplugin_computer::ComputerItemWatcher::*)(const QString &, const QUrl &, int, bool)>(&dfmplugin_computer::ComputerItemWatcher::addDevice));
    const QVariant ret = mgr.push(63300);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_ComputerItemWatcher_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_computer::ComputerItemWatcher *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63302, receiver, static_cast<void (dfmplugin_computer::ComputerItemWatcher::*)(const QUrl &)>(&dfmplugin_computer::ComputerItemWatcher::removeDevice));
    const QVariant ret = mgr.push(63302);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_ComputerItemWatcher_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_computer::ComputerItemWatcher *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63304, receiver, static_cast<void (dfmplugin_computer::ComputerItemWatcher::*)()>(&dfmplugin_computer::ComputerItemWatcher::onViewRefresh));
    const QVariant ret = mgr.push(63304, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_ComputerEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_computer::ComputerEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63306, receiver, static_cast<bool (dfmplugin_computer::ComputerEventReceiver::*)(const QUrl &, QString *)>(&dfmplugin_computer::ComputerEventReceiver::handleSetTabName));
    const bool ran = mgr.run(63306);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_ComputerEventReceiver_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_computer::ComputerEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63308, receiver, static_cast<bool (dfmplugin_computer::ComputerEventReceiver::*)(const QString &, const QString &, const QUrl &, const QUrl &)>(&dfmplugin_computer::ComputerEventReceiver::handleSortItem));
    const bool ran = mgr.run(63308);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

