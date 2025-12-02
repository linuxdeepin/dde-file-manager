// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextmenucache.h"

#include <QAction>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_DFMExtMenuCache : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtMenuCache, instance_ReturnsSingleton)
{
    DFMExtMenuCache &instance1 = DFMExtMenuCache::instance();
    DFMExtMenuCache &instance2 = DFMExtMenuCache::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(UT_DFMExtMenuCache, extMenuSortRules_InitiallyEmpty)
{
    DFMExtMenuCache &cache = DFMExtMenuCache::instance();

    cache.extMenuSortRules.clear();

    EXPECT_TRUE(cache.extMenuSortRules.isEmpty());
}

TEST_F(UT_DFMExtMenuCache, extMenuSortRules_CanAddRules)
{
    DFMExtMenuCache &cache = DFMExtMenuCache::instance();
    cache.extMenuSortRules.clear();

    QAction action1;
    QAction action2;
    cache.extMenuSortRules.append(qMakePair(&action1, &action2));

    EXPECT_EQ(cache.extMenuSortRules.size(), 1);
    cache.extMenuSortRules.clear();
}
