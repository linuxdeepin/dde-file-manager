// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run Optical::initialize() with REAL bindEvents() and
// bindFileOperations() (NOT stubbed) so production subscribe/follow templates
// execute. bindWindows remains stubbed (UI-heavy). The DevProxyMng connect and
// LifeCycle check are safe (existing test runs them).
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "optical.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_optical;

class OpticalRealEventsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dfmtest_hooks::registerAllHookEvents();
        stub.set_lamda(ADDR(Optical, bindWindows), [] { __DBG_STUB_INVOKE__ });
        ins = new Optical();
    }
    void TearDown() override { stub.clear(); delete ins; }
    stub_ext::StubExt stub;
    Optical *ins { nullptr };
};

TEST_F(OpticalRealEventsTest, Initialize_RunsRealBindEventsAndFileOps_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->initialize());
}
