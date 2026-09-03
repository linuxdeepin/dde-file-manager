// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Real-events test: run Search::initialize() with REAL bindEvents() (NOT stubbed)
// so production EventHelper<M> subscribe/follow templates execute.
// The existing SearchTest.ut_initialize already runs the rest of initialize()
// safely; we only un-stub bindEvents.
#include <gtest/gtest.h>
#include "stubext.h"
#include "dfm_hookreg.h"
#include "search.h"
#include <dfm-framework/dpf.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_search;

TEST(SearchRealEventsTest, Initialize_RunsRealBindEvents_NoCrash)
{
    dfmtest_hooks::registerAllHookEvents();
    Search search;
    EXPECT_NO_FATAL_FAILURE(search.initialize());
}
