// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>

#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/state/indexstatestore.h"
#include "services/textindex/task/backlogtracker.h"
#include "services/textindex/utils/indexutility.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class FilenameBacklogTrackerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(directory.isValid());
        profile = IndexProfile({ IndexProfile::Type::Filename,
                                 "backlog-test",
                                 "backlog_status.json",
                                 "backlog_version",
                                 1 },
                               { [this]() { return directory.path(); },
                                 []() { return true; },
                                 [](const QString &) { return true; },
                                 [](const QString &) { return true; } });
        store = std::make_unique<IndexStateStore>(profile);
        tracker = std::make_unique<FilenameBacklogTracker>(store.get());
    }

    QTemporaryDir directory;
    IndexProfile profile;
    std::unique_ptr<IndexStateStore> store;
    std::unique_ptr<FilenameBacklogTracker> tracker;
};

TEST_F(FilenameBacklogTrackerTest, SetsAtDefaultThresholdAndClearsWhenDrained)
{
    tracker->update(4999, 0, true);
    EXPECT_FALSE(store->isBacklogExceeded());

    tracker->update(5000, 0, true);
    EXPECT_TRUE(store->isBacklogExceeded());

    tracker->update(0, 0, true);
    EXPECT_TRUE(store->isBacklogExceeded());

    tracker->update(0, 0, false);
    EXPECT_FALSE(store->isBacklogExceeded());
}

TEST_F(FilenameBacklogTrackerTest, StartupKeepsDirtyBacklogAndResetsCleanBacklog)
{
    store->setBacklogExceeded(true);
    store->setIndexState(IndexUtility::IndexState::Dirty);
    tracker->onStartup();
    EXPECT_TRUE(store->isBacklogExceeded());

    store->setIndexState(IndexUtility::IndexState::Clean);
    tracker->onStartup();
    EXPECT_FALSE(store->isBacklogExceeded());
}

TEST_F(FilenameBacklogTrackerTest, FullScanCompletionClearsBacklog)
{
    store->setBacklogExceeded(true);
    tracker->onFullScanFinished();
    EXPECT_FALSE(store->isBacklogExceeded());
}
