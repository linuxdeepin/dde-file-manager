// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run Computer::initialize() with REAL bindEvents()/followEvents()
// (NOT stubbed) so production EventHelper<M> subscribe/follow templates execute.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "computer.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_computer;

class ComputerRealEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();
        stub.set_lamda(&Computer::bindWindows, [] { __DBG_STUB_INVOKE__ });
        ins = new Computer();
    }
    void TearDown() override { stub.clear(); delete ins; }
    stub_ext::StubExt stub;
    Computer *ins { nullptr };
};

TEST_F(ComputerRealEventsTest, Initialize_RunsRealBindAndFollowEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->initialize());
}
