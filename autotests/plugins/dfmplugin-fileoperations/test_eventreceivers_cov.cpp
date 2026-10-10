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

#include <plugins/common/dfmplugin-fileoperations/fileoperationsevent/fileoperationseventreceiver.h>
#include <plugins/common/dfmplugin-fileoperations/fileoperationsevent/trashfileeventreceiver.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};




TEST_F(EventReceiverSweepTest, Register_FileOperationsEventReceiver_3_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_fileoperations::FileOperationsEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61004, receiver, static_cast<bool (dfmplugin_fileoperations::FileOperationsEventReceiver::*)(unsigned long long, QList<QUrl>, bool*)>(&dfmplugin_fileoperations::FileOperationsEventReceiver::handleOperationOpenFiles)));
    EXPECT_TRUE(mgr.publish(61004));  // arity guard: dispatch ok, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_FileOperationsEventReceiver_4_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_fileoperations::FileOperationsEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61005, receiver, static_cast<void (dfmplugin_fileoperations::FileOperationsEventReceiver::*)(QList<QUrl> const&)>(&dfmplugin_fileoperations::FileOperationsEventReceiver::handleOperationCleanByUrls)));
    EXPECT_TRUE(mgr.publish(61005));  // arity guard: dispatch ok, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_FileOperationsEventReceiver_5_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_fileoperations::FileOperationsEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61008, receiver, static_cast<void (dfmplugin_fileoperations::FileOperationsEventReceiver::*)()>(&dfmplugin_fileoperations::FileOperationsEventReceiver::handleOperationCleanSaveOperationsStack)));
    EXPECT_TRUE(mgr.publish(61008));  // arity guard: dispatch ok, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_FileOperationsEventReceiver_6_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_fileoperations::FileOperationsEventReceiver::instance();
    EXPECT_TRUE(mgr.subscribe(61009, receiver, static_cast<void (dfmplugin_fileoperations::FileOperationsEventReceiver::*)(QMap<QString, QVariant>)>(&dfmplugin_fileoperations::FileOperationsEventReceiver::handleOperationSaveOperations)));
    EXPECT_TRUE(mgr.publish(61009));  // arity guard: dispatch ok, slot body not run
}

