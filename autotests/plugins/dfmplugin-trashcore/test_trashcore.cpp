// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#include "dfmplugin_trashcore_global.h"
#include "stubext.h"
#include "trashcore.h"
#include "events/trashcoreeventreceiver.h"
#include "events/trashcoreeventsender.h"
#include "utils/trashcorehelper.h"
#include <dfm-framework/dpf.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_event_defines.h>

using namespace dfmplugin_trashcore;

class TrashCoreTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TrashCoreTest, Initialize_Basic)
{
    TrashCore plugin;
    
    EXPECT_NO_THROW(plugin.initialize());
}

TEST_F(TrashCoreTest, Start_Basic)
{
    TrashCore plugin;
    
    EXPECT_TRUE(plugin.start());
}