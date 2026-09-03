// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/recentfilewatcher.h"
#include "private/recentfilewatcher_p.h"
#include "utils/recentmanager.h"

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

class RecentFileWatcherTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // RecentFileWatcher now just wraps AbstractFileWatcher with start/stop
        watcher = new RecentFileWatcher(RecentHelper::rootUrl());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

protected:
    RecentFileWatcher *watcher = { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(RecentFileWatcherTest, Constructor_CreatesSuccessfully)
{
    EXPECT_NE(watcher, nullptr);
}

TEST_F(RecentFileWatcherTest, Destructor_CleansUp)
{
    RecentFileWatcher *tempWatcher = new RecentFileWatcher(RecentHelper::rootUrl());
    EXPECT_NO_THROW(delete tempWatcher);
}

TEST_F(RecentFileWatcherTest, Start_ReturnsTrue)
{
    // RecentFileWatcherPrivate::start just sets started = true
    EXPECT_TRUE(watcher->dptr->start());
}

TEST_F(RecentFileWatcherTest, Stop_ReturnsTrue)
{
    // RecentFileWatcherPrivate::stop just sets started = false
    watcher->dptr->start();   // start first
    EXPECT_TRUE(watcher->dptr->stop());
}

TEST_F(RecentFileWatcherTest, Inherits_AbstractFileWatcher)
{
    auto *base = dynamic_cast<AbstractFileWatcher *>(watcher);
    EXPECT_NE(base, nullptr);
}
