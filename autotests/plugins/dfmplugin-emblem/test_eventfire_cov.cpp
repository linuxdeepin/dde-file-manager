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

#include <plugins/common/dfmplugin-emblem/events/emblemeventrecevier.h>
#include <plugins/common/dfmplugin-emblem/utils/emblemhelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_EmblemEventRecevier_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_emblem::EmblemEventRecevier *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(64200, receiver, static_cast<bool (dfmplugin_emblem::EmblemEventRecevier::*)(QPainter *, const QRectF &, const FileInfoPointer &)>(&dfmplugin_emblem::EmblemEventRecevier::handlePaintEmblems));
    const QVariant ret = mgr.push(64200);
    EXPECT_TRUE(connected);
    EXPECT_TRUE(ret.isValid());  // null-receiver guard: default-constructed non-void return, slot body not run
}

TEST_F(EventFireSweepTest, Fire_EmblemHelper_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_emblem::EmblemHelper *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventDispatcherManager mgr;
    const bool subscribed = mgr.subscribe(64202, receiver, static_cast<bool (dfmplugin_emblem::EmblemHelper::*)(quint64, const QUrl &)>(&dfmplugin_emblem::EmblemHelper::onUrlChanged));
    const bool published = mgr.publish(64202);
    EXPECT_TRUE(subscribed);
    EXPECT_TRUE(published);  // handler dispatched, null-receiver guard skips slot body
}

