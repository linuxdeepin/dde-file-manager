// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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

// 新增测试用例：测试 followEvents 方法
TEST_F(TrashCoreTest, FollowEvents_Basic)
{
    TrashCore plugin;
    
    // 只需确保方法不抛出异常
    EXPECT_NO_THROW(plugin.followEvents());
}

// 新增测试用例：测试 start 方法中的不同分支路径
TEST_F(TrashCoreTest, Start_WithPropertyDialogPlugin)
{
    TrashCore plugin;
    
    // 目前只测试基本的 start 方法
    EXPECT_TRUE(plugin.start());
}

// 新增测试用例：测试 regCustomPropertyDialog 方法
TEST_F(TrashCoreTest, RegCustomPropertyDialog_Basic)
{
    TrashCore plugin;
    
    // 确保方法存在且可调用
    EXPECT_NO_THROW(plugin.regCustomPropertyDialog());
}