// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run DirShare::initialize() with REAL bindEvents() (NOT stubbed)
// so production EventHelper<M> subscribe templates execute.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "dirshare.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_dirshare;

class DirShareRealEventsTest : public testing::Test
{
protected:
    void SetUp() override { dfmtest_hooks::registerAllHookEvents(); ins = new DirShare(); }
    void TearDown() override { delete ins; }
    DirShare *ins { nullptr };
};

TEST_F(DirShareRealEventsTest, Initialize_RunsRealBindEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->initialize());
}
