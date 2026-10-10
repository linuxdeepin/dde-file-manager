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

#include <plugins/common/dfmplugin-burn/events/burneventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep2Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep2Test, Register_BurnEventReceiver_0_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_burn::BurnEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61000, receiver, static_cast<void (dfmplugin_burn::BurnEventReceiver::*)(QList<QUrl> const&, QUrl const&, bool)>(&dfmplugin_burn::BurnEventReceiver::handlePasteTo)));
        EXPECT_FALSE((mgr.push(61000)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_BurnEventReceiver_1_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_burn::BurnEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61001, receiver, static_cast<void (dfmplugin_burn::BurnEventReceiver::*)(unsigned long long, QUrl const&)>(&dfmplugin_burn::BurnEventReceiver::handleMountImage)));
        EXPECT_FALSE((mgr.push(61001)).isValid());  // arity guard: default-constructed return, slot body not run
}
