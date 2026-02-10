// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "utils/sharewatchermanager.h"

#include <dfm-base/file/local/localfilewatcher.h>

using namespace dfmplugin_dirshare;

class UT_ShareWatcherManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        manager = new ShareWatcherManager;
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete manager;
    }

private:
    stub_ext::StubExt stub;

    ShareWatcherManager *manager { nullptr };
};

TEST_F(UT_ShareWatcherManager, Add)
{
    EXPECT_TRUE(manager->watchers.count() == 0);
    EXPECT_TRUE(manager->add("/"));
    EXPECT_NO_FATAL_FAILURE(manager->add("/"));
    EXPECT_TRUE(manager->watchers.count() == 1);
}

TEST_F(UT_ShareWatcherManager, Remove)
{
    EXPECT_NO_FATAL_FAILURE(manager->add("/"));
    EXPECT_TRUE(manager->watchers.count() == 1);
    EXPECT_NO_FATAL_FAILURE(manager->remove("/"));
    EXPECT_TRUE(manager->watchers.count() == 0);
}
