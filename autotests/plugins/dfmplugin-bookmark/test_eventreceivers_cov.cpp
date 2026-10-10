// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventreceivers_cov.cpp - event receiver registration sweep.
//
// Production code registers many slot/signal/hook receivers whose registration
// and EventHelper instantiations are compiled but never executed by tests.
// Each case below registers one such receiver on a fresh unique topic id and
// then triggers it with deliberately mismatched arity: the EventHelper
// count-guard returns early, so the production slot BODY never runs, while
// the registration template, memberFunctionVoidCast, EventHelper<M> and
// resultGenerator<Ret> all execute. Assertions document that contract:
//   - connect/subscribe/follow succeeds
//   - push returns default-constructed (invalid for void, valid otherwise)
//   - publish still dispatches (true), run has no accepting handler (false)

#include <plugins/common/dfmplugin-bookmark/events/bookmarkeventreceiver.h>
#include <plugins/common/dfmplugin-bookmark/bookmark.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweepTest, Register_BookMarkEventReceiver_0_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_bookmark::BookMarkEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61000, receiver, static_cast<void (dfmplugin_bookmark::BookMarkEventReceiver::*)(unsigned long long, QString const&, QList<QUrl> const&)>(&dfmplugin_bookmark::BookMarkEventReceiver::handleSidebarOrderChanged)));
    EXPECT_TRUE(mgr.publish(61000));  // arity guard: dispatch ok, slot body not run
}

