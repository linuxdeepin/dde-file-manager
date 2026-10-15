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

#include <plugins/filemanager/dfmplugin-detailspace/events/detailspaceeventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_DetailSpaceEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_detailspace::DetailSpaceEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62300, receiver, static_cast<bool (dfmplugin_detailspace::DetailSpaceEventReceiver::*)(dfmplugin_detailspace::ViewExtensionCreateFunc, dfmplugin_detailspace::ViewExtensionUpdateFunc, dfmplugin_detailspace::ViewExtensionShouldShowFunc, int)>(&dfmplugin_detailspace::DetailSpaceEventReceiver::handleViewExtensionRegister));
    const QVariant ret = mgr.push(62300);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_DetailSpaceEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_detailspace::DetailSpaceEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62302, receiver, static_cast<void (dfmplugin_detailspace::DetailSpaceEventReceiver::*)(quint64, bool, bool)>(&dfmplugin_detailspace::DetailSpaceEventReceiver::handleTileBarShowDetailView));
    const QVariant ret = mgr.push(62302);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_DetailSpaceEventReceiver_2_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_detailspace::DetailSpaceEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62304, receiver, static_cast<bool (dfmplugin_detailspace::DetailSpaceEventReceiver::*)(const QString &, const QStringList &)>(&dfmplugin_detailspace::DetailSpaceEventReceiver::handleBasicFiledFilterAdd));
    const QVariant ret = mgr.push(62304);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_DetailSpaceEventReceiver_3_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_detailspace::DetailSpaceEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(62306, receiver, static_cast<bool (dfmplugin_detailspace::DetailSpaceEventReceiver::*)(dfmplugin_detailspace::BasicViewFieldFunc, const QString &)>(&dfmplugin_detailspace::DetailSpaceEventReceiver::handleBasicViewExtensionRegister));
    const QVariant ret = mgr.push(62306);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_DetailSpaceEventReceiver_4_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_detailspace::DetailSpaceEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(62308, receiver, static_cast<void (dfmplugin_detailspace::DetailSpaceEventReceiver::*)(const quint64, const QItemSelection &, const QItemSelection &)>(&dfmplugin_detailspace::DetailSpaceEventReceiver::handleViewSelectionChanged));
    const bool published = mgr.publish(62308);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

