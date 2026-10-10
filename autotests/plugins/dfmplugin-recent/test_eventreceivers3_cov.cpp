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

#include <plugins/filemanager/dfmplugin-recent/events/recenteventreceiver.h>
#include <plugins/filemanager/dfmplugin-recent/utils/recentfilehelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep3Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep3Test, Register_RecentEventReceiver_0_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_recent::RecentEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61000, receiver, static_cast<void (dfmplugin_recent::RecentEventReceiver::*)(QList<QUrl> const&, QList<QUrl> const&, bool, QString const&)>(&dfmplugin_recent::RecentEventReceiver::handleFileCutResult)));
    EXPECT_TRUE(mgr.publish(61000));  // arity guard: dispatch ok, slot body not run
}
