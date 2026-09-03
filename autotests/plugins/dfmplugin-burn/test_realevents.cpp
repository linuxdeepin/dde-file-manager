// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run Burn::initialize() with REAL bindEvents() (NOT stubbed)
// so production EventHelper<M> subscribe template instantiations execute.
// bindEvents uses GlobalEventType (always valid, no registration needed).
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "burn.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_burn;

class BurnRealEventsTest : public testing::Test
{
protected:
    void SetUp() override { dfmtest_hooks::registerAllHookEvents(); ins = new Burn(); }
    void TearDown() override { delete ins; }
    Burn *ins { nullptr };
};

TEST_F(BurnRealEventsTest, Initialize_RunsRealBindEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->initialize());
}
