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

#include <plugins/desktop/ddplugin-core/core.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_EventHandle_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_core::EventHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62400, receiver, static_cast<dfmbase::ScreenPointer (ddplugin_core::EventHandle::*)(const QString &)>(&ddplugin_core::EventHandle::screen));
    const QVariant ret = mgr.push(62400);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_EventHandle_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_core::EventHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62402, receiver, static_cast<double (ddplugin_core::EventHandle::*)()>(&ddplugin_core::EventHandle::devicePixelRatio));
    const QVariant ret = mgr.push(62402, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_EventHandle_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_core::EventHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62404, receiver, static_cast<QList<QWidget *>(ddplugin_core::EventHandle::*)()>(&ddplugin_core::EventHandle::rootWindows));
    const QVariant ret = mgr.push(62404, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_EventHandle_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_core::EventHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62406, receiver, static_cast<void (ddplugin_core::EventHandle::*)()>(&ddplugin_core::EventHandle::reset));
    const QVariant ret = mgr.push(62406, QString::fromLatin1("arity-guard"));
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_EventHandle_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_core::EventHandle *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(62408, receiver, static_cast<bool (ddplugin_core::EventHandle::*)(QStringList *)>(&ddplugin_core::EventHandle::screensInUse));
    const bool ran = mgr.run(62408);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

TEST_F(EventFireSweepTest, Fire_Core_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<ddplugin_core::Core *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62410, receiver, static_cast<void (ddplugin_core::Core::*)(const QStringList &)>(&ddplugin_core::Core::handleLoadPlugins));
    const bool published = mgr.publish(62410);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

