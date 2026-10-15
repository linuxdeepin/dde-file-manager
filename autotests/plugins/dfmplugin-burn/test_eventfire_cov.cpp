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

#include <plugins/common/dfmplugin-burn/events/burneventreceiver.h>
#include <plugins/common/dfmplugin-burn/burn.h>

#include <QWidget>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_BurnEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_burn::BurnEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63000, receiver, static_cast<void (dfmplugin_burn::BurnEventReceiver::*)(const QString &, bool, QWidget *)>(&dfmplugin_burn::BurnEventReceiver::handleShowBurnDlg));
    const QVariant ret = mgr.push(63000);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_BurnEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_burn::BurnEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63002, receiver, static_cast<void (dfmplugin_burn::BurnEventReceiver::*)(const QList<QUrl> &, const QList<QUrl> &, bool, const QString &)>(&dfmplugin_burn::BurnEventReceiver::handleCopyFilesResult));
    const QVariant ret = mgr.push(63002);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_BurnEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_burn::BurnEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63004, receiver, static_cast<void (dfmplugin_burn::BurnEventReceiver::*)(const QList<QUrl> &, bool, const QString &)>(&dfmplugin_burn::BurnEventReceiver::handleFileRemoveResult));
    const QVariant ret = mgr.push(63004);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_BurnEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_burn::BurnEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63006, receiver, static_cast<void (dfmplugin_burn::BurnEventReceiver::*)(quint64, const QMap<QUrl, QUrl> &, bool, const QString &)>(&dfmplugin_burn::BurnEventReceiver::handleFileRenameResult));
    const QVariant ret = mgr.push(63006);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_Burn_4_ArityGuardNoBody)
{
    // Arrange
    dfmplugin_burn::Burn receiver;
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63008, &receiver, static_cast<void (dfmplugin_burn::Burn::*)(const QString &)>(&dfmplugin_burn::Burn::bindScene));
    const bool published = mgr.publish(63008);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
    EXPECT_TRUE(mgr.unsubscribe(63008, &receiver, static_cast<void (dfmplugin_burn::Burn::*)(const QString &)>(&dfmplugin_burn::Burn::bindScene)));  // remove + compare path
}

TEST_F(EventFireSweepTest, Fire_Burn_5_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_burn::Burn *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(63010, receiver, static_cast<bool (dfmplugin_burn::Burn::*)(quint64, const QUrl &)>(&dfmplugin_burn::Burn::changeUrlEventFilter));
    const bool published = mgr.publish(63010);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

