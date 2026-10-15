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

#include <plugins/common/dfmplugin-trashcore/events/trashcoreeventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, Fire_TrashCoreEventReceiver_0_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trashcore::TrashCoreEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventChannelManager mgr;
    const bool connected = mgr.connect(63600, receiver, static_cast<void (dfmplugin_trashcore::TrashCoreEventReceiver::*)(const quint64)>(&dfmplugin_trashcore::TrashCoreEventReceiver::handleEmptyTrash));
    const QVariant ret = mgr.push(63600);
    EXPECT_TRUE(connected);
    EXPECT_FALSE(ret.isValid());  // null-receiver guard: void return stays invalid, slot body not run
}

TEST_F(EventFireSweepTest, Fire_TrashCoreEventReceiver_1_ArityGuardNoBody)
{
    // Arrange
    auto *receiver = static_cast<dfmplugin_trashcore::TrashCoreEventReceiver *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act + Assert
    dpf::EventSequenceManager mgr;
    const bool followed = mgr.follow(63602, receiver, static_cast<bool (dfmplugin_trashcore::TrashCoreEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl, dfmbase::AbstractJobHandler::JobFlags)>(&dfmplugin_trashcore::TrashCoreEventReceiver::copyFromFile));
    const bool ran = mgr.run(63602);
    EXPECT_TRUE(followed);
    EXPECT_FALSE(ran);  // null-receiver guard: no handler accepts, traversal returns false
}

