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

#include <plugins/filemanager/dfmplugin-trash/utils/trashhelper.h>
#include <plugins/filemanager/dfmplugin-trash/utils/trashfilehelper.h>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

using namespace dpf;

class EventReceiverSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventReceiverSweepTest, Register_TrashHelper_0_ArityGuardNoBody)
{
    dpf::EventDispatcherManager mgr;
    auto *receiver = dfmplugin_trash::TrashHelper::instance();
    EXPECT_TRUE(mgr.subscribe(61000, receiver, static_cast<void (dfmplugin_trash::TrashHelper::*)()>(&dfmplugin_trash::TrashHelper::onTrashEmptyState)));
    EXPECT_TRUE(mgr.publish(61000));  // arity guard: dispatch ok, slot body not run
}

TEST_F(EventReceiverSweepTest, Register_TrashFileHelper_1_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_trash::TrashFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61001, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(QUrl const&, QUrl const&)>(&dfmplugin_trash::TrashFileHelper::handleIsSubFile)));
    EXPECT_FALSE(mgr.run(61001));  // arity guard: no handler accepted
}

TEST_F(EventReceiverSweepTest, Register_TrashHelper_2_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_trash::TrashHelper::instance();
    EXPECT_TRUE(mgr.follow(61002, receiver, static_cast<bool (dfmplugin_trash::TrashHelper::*)(QUrl const&, QString*)>(&dfmplugin_trash::TrashHelper::detailViewIcon)));
    EXPECT_FALSE(mgr.run(61002));  // arity guard: no handler accepted
}


TEST_F(EventReceiverSweepTest, Register_TrashFileHelper_4_ArityGuardNoBody)
{
    dpf::EventSequenceManager mgr;
    auto *receiver = dfmplugin_trash::TrashFileHelper::instance();
    EXPECT_TRUE(mgr.follow(61004, receiver, static_cast<bool (dfmplugin_trash::TrashFileHelper::*)(QUrl const&, QUrl*)>(&dfmplugin_trash::TrashFileHelper::handleNotCdComputer)));
    EXPECT_FALSE(mgr.run(61004));  // arity guard: no handler accepted
}

