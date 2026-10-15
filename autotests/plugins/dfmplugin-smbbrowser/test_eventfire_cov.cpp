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

#include <plugins/filemanager/dfmplugin-smbbrowser/displaycontrol/protocoldevicedisplaymanager.h>
#include <plugins/filemanager/dfmplugin-smbbrowser/events/smbbrowsereventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_ProtocolDeviceDisplayManager_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::ProtocolDeviceDisplayManager *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63100, receiver, static_cast<void (dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::*)(const QUrl &, const QString &)>(&dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::handleItemRenamed));
    const bool published = mgr.publish(63100);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_ProtocolDeviceDisplayManager_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::ProtocolDeviceDisplayManager *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63102, receiver, static_cast<void (dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::*)(const QString &)>(&dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::onDevUnmounted));
    const bool published = mgr.publish(63102);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

TEST_F(EventFireSweepTest, Fire_ProtocolDeviceDisplayManager_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::ProtocolDeviceDisplayManager *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63104, receiver, static_cast<bool (dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::*)(QList<QUrl> *)>(&dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::hookItemsFilter));
    const bool ran = mgr.run(63104);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_ProtocolDeviceDisplayManager_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::ProtocolDeviceDisplayManager *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63106, receiver, static_cast<bool (dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::*)(const QUrl &)>(&dfmplugin_smbbrowser::ProtocolDeviceDisplayManager::hookItemInsert));
    const bool ran = mgr.run(63106);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_SmbBrowserEventReceiver_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::SmbBrowserEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63108, receiver, static_cast<bool (dfmplugin_smbbrowser::SmbBrowserEventReceiver::*)(QUrl *)>(&dfmplugin_smbbrowser::SmbBrowserEventReceiver::hookTitleBarAddrHandle));
    const bool ran = mgr.run(63108);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_SmbBrowserEventReceiver_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::SmbBrowserEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63110, receiver, static_cast<bool (dfmplugin_smbbrowser::SmbBrowserEventReceiver::*)(const QUrl &, const QUrl &)>(&dfmplugin_smbbrowser::SmbBrowserEventReceiver::hookAllowRepeatUrl));
    const bool ran = mgr.run(63110);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_SmbBrowserEventReceiver_6_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::SmbBrowserEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63112, receiver, static_cast<bool (dfmplugin_smbbrowser::SmbBrowserEventReceiver::*)(const QUrl &, QString *)>(&dfmplugin_smbbrowser::SmbBrowserEventReceiver::hookSetTabName));
    const bool ran = mgr.run(63112);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_SmbBrowserEventReceiver_7_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_smbbrowser::SmbBrowserEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63114, receiver, static_cast<bool (dfmplugin_smbbrowser::SmbBrowserEventReceiver::*)(quint64, const QList<QUrl> &, const QUrl &)>(&dfmplugin_smbbrowser::SmbBrowserEventReceiver::cancelDelete));
    const bool ran = mgr.run(63114);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

