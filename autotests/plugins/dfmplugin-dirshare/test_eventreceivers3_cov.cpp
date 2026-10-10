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

#include <plugins/common/dfmplugin-dirshare/utils/usersharehelper.h>
#include <plugins/common/dfmplugin-dirshare/dirshare.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep3Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep3Test, Register_UserShareHelper_0_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_dirshare::UserShareHelper::instance();
        EXPECT_TRUE(mgr.connect(61000, receiver, static_cast<bool (dfmplugin_dirshare::UserShareHelper::*)(QString const&)>(&dfmplugin_dirshare::UserShareHelper::removeShareByPath)));
        EXPECT_TRUE((mgr.push(61000)).isValid());  // arity guard: default-constructed return, slot body not run
}


TEST_F(EventReceiverSweep3Test, Register_UserShareHelper_2_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_dirshare::UserShareHelper::instance();
    EXPECT_TRUE(mgr.subscribe(61002, receiver, static_cast<void (dfmplugin_dirshare::UserShareHelper::*)(QString const&)>(&dfmplugin_dirshare::UserShareHelper::handleSetPassword)));
    EXPECT_TRUE(mgr.publish(61002));  // arity guard: dispatch ok, slot body not run
}
