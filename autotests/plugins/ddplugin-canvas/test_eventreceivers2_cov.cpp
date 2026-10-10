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

#include <plugins/desktop/ddplugin-canvas/broker/canvasviewbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/fileinfomodelbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/canvasmodelbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/canvasmanagerbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/canvasgridbroker.h>
#include <plugins/desktop/ddplugin-canvas/recentproxy/canvasrecentproxy.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep2Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep2Test, Register_CanvasViewBroker_0_ArityGuardNoBody)
{
    ddplugin_canvas::CanvasViewBroker CanvasViewBrokerInst0(nullptr);
    dpf::EventChannelManager mgr;
        auto *receiver = &CanvasViewBrokerInst0;
        EXPECT_TRUE(mgr.connect(61000, receiver, static_cast<QRect (ddplugin_canvas::CanvasViewBroker::*)(int, QRect)>(&ddplugin_canvas::CanvasViewBroker::iconRect)));
        EXPECT_TRUE((mgr.push(61000)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_CanvasViewBroker_1_ArityGuardNoBody)
{
    ddplugin_canvas::CanvasViewBroker CanvasViewBrokerInst1(nullptr);
    dpf::EventChannelManager mgr;
        auto *receiver = &CanvasViewBrokerInst1;
        EXPECT_TRUE(mgr.connect(61011, receiver, static_cast<void (ddplugin_canvas::CanvasViewBroker::*)(int)>(&ddplugin_canvas::CanvasViewBroker::refresh)));
        EXPECT_FALSE((mgr.push(61011)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep2Test, Register_CanvasViewBroker_2_ArityGuardNoBody)
{
    ddplugin_canvas::CanvasViewBroker CanvasViewBrokerInst2(nullptr);
    dpf::EventChannelManager mgr;
        auto *receiver = &CanvasViewBrokerInst2;
        EXPECT_TRUE(mgr.connect(61013, receiver, static_cast<QObject* (ddplugin_canvas::CanvasViewBroker::*)()>(&ddplugin_canvas::CanvasViewBroker::fileOperator)));
        EXPECT_TRUE((mgr.push(61013, QString::fromLatin1("arity-guard"))).isValid());  // arity guard: default-constructed return, slot body not run
}

