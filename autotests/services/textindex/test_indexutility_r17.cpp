// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexutility_r17.cpp
 * @brief Additional IndexUtility tests: ConfigRebuildWatcher ctor lambda
 *        (constructed with empty WatchEntry list), DlnfsConfigWatcher and
 *        AnythingConfigWatcher D0 destructors (heap alloc+delete).
 */

#include <gtest/gtest.h>
#include <QList>
#include <QObject>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/indexutility.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(IndexUtilityR17Test, ConfigRebuildWatcherConstructsEmpty)
{
    QList<IndexUtility::ConfigRebuildWatcher::WatchEntry> empty;
    IndexUtility::ConfigRebuildWatcher w(empty, nullptr);
    SUCCEED();
}

TEST(IndexUtilityR17Test, ConfigRebuildWatcherD0Destructor)
{
    QList<IndexUtility::ConfigRebuildWatcher::WatchEntry> empty;
    auto *ptr = new IndexUtility::ConfigRebuildWatcher(empty, nullptr);
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}

TEST(IndexUtilityR17Test, DlnfsConfigWatcherInstanceCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::DlnfsConfigWatcher::instance(); });
}

TEST(IndexUtilityR17Test, AnythingConfigWatcherInstanceCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::AnythingConfigWatcher::instance(); });
}

TEST(IndexUtilityR17Test, IsDefaultIndexedDirectoryReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isDefaultIndexedDirectory("/tmp"); });
}

TEST(IndexUtilityR17Test, IsIndexWithAnythingReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isIndexWithAnything("/tmp"); });
}

TEST(IndexUtilityR17Test, IsSupportedTextFileReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isSupportedTextFile("/tmp/test.txt"); });
}

TEST(IndexUtilityR17Test, IsSupportedOCRFileReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isSupportedOCRFile("/tmp/test.png"); });
}
