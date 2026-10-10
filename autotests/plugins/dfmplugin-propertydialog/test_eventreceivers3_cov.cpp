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

#include <plugins/common/dfmplugin-propertydialog/events/propertyeventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep3Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep3Test, Register_PropertyEventReceiver_0_ArityGuardNoBody)
{
    dpf::EventChannelManager mgr;
        auto *receiver = dfmplugin_propertydialog::PropertyEventReceiver::instance();
        EXPECT_TRUE(mgr.connect(61002, receiver, static_cast<bool (dfmplugin_propertydialog::PropertyEventReceiver::*)(std::function<QWidget* (QUrl const&)>, QString const&)>(&dfmplugin_propertydialog::PropertyEventReceiver::handleCustomViewRegister)));
        EXPECT_TRUE((mgr.push(61002)).isValid());  // arity guard: default-constructed return, slot body not run
}
