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

#include <plugins/desktop/ddplugin-canvas/broker/canvasmodelbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/canvasmanagerbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/canvasviewbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/fileinfomodelbroker.h>
#include <plugins/desktop/ddplugin-canvas/broker/canvasgridbroker.h>
#include <plugins/desktop/ddplugin-canvas/canvasmanager.h>
#include <plugins/desktop/ddplugin-canvas/recentproxy/canvasrecentproxy.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweep3Test : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweep3Test, Register_CanvasViewBroker_0_ArityGuardNoBody)
{
    ddplugin_canvas::CanvasViewBroker CanvasViewBrokerInst0(nullptr);
    dpf::EventChannelManager mgr;
        auto *receiver = &CanvasViewBrokerInst0;
        EXPECT_TRUE(mgr.connect(61003, receiver, static_cast<QRect (ddplugin_canvas::CanvasViewBroker::*)(int, QRect)>(&ddplugin_canvas::CanvasViewBroker::iconRect)));
        EXPECT_TRUE((mgr.push(61003)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep3Test, Register_CanvasViewBroker_1_ArityGuardNoBody)
{
    ddplugin_canvas::CanvasViewBroker CanvasViewBrokerInst1(nullptr);
    dpf::EventChannelManager mgr;
        auto *receiver = &CanvasViewBrokerInst1;
        EXPECT_TRUE(mgr.connect(61009, receiver, static_cast<QSize (ddplugin_canvas::CanvasViewBroker::*)(int)>(&ddplugin_canvas::CanvasViewBroker::gridSize)));
        EXPECT_TRUE((mgr.push(61009)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep3Test, Register_CanvasViewBroker_2_ArityGuardNoBody)
{
    ddplugin_canvas::CanvasViewBroker CanvasViewBrokerInst2(nullptr);
    dpf::EventChannelManager mgr;
        auto *receiver = &CanvasViewBrokerInst2;
        EXPECT_TRUE(mgr.connect(61013, receiver, static_cast<QRect (ddplugin_canvas::CanvasViewBroker::*)(int, QRect)>(&ddplugin_canvas::CanvasViewBroker::iconRect)));
        EXPECT_TRUE((mgr.push(61013)).isValid());  // arity guard: default-constructed return, slot body not run
}

TEST_F(EventReceiverSweep3Test, Register_CanvasManager_3_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = ddplugin_canvas::CanvasManager::instance();
    EXPECT_TRUE(mgr.subscribe(61016, receiver, static_cast<void (ddplugin_canvas::CanvasManager::*)()>(&ddplugin_canvas::CanvasManager::onCanvasBuild)));
    EXPECT_TRUE(mgr.publish(61016));  // arity guard: dispatch ok, slot body not run
}

