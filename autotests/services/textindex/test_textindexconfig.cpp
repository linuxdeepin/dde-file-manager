// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexconfig.cpp
 * @brief Unit tests for TextIndexConfig (textindexconfig.cpp)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/textindexconfig.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(TextIndexConfigTest, InstanceReturnsRef)
{
    EXPECT_NO_FATAL_FAILURE({ (void)&TextIndexConfig::instance(); });
}

TEST(TextIndexConfigTest, AutoIndexUpdateIntervalPositive)
{
    EXPECT_GT(TextIndexConfig::instance().autoIndexUpdateInterval(), 0);
}

TEST(TextIndexConfigTest, MonitoringStartDelaySecondsNonNeg)
{
    EXPECT_GE(TextIndexConfig::instance().monitoringStartDelaySeconds(), 0);
}

TEST(TextIndexConfigTest, SilentIndexUpdateDelayNonNeg)
{
    EXPECT_GE(TextIndexConfig::instance().silentIndexUpdateDelay(), 0);
}

TEST(TextIndexConfigTest, InotifyResourceCleanupDelayMsNonNeg)
{
    EXPECT_GE(TextIndexConfig::instance().inotifyResourceCleanupDelayMs(), 0);
}

TEST(TextIndexConfigTest, MaxIndexTextFileSizeMBPositive)
{
    EXPECT_GT(TextIndexConfig::instance().maxIndexTextFileSizeMB(), 0);
}

TEST(TextIndexConfigTest, MaxIndexFileTruncationSizeMBPositive)
{
    EXPECT_GT(TextIndexConfig::instance().maxIndexFileTruncationSizeMB(), 0);
}

TEST(TextIndexConfigTest, SupportedTextFileExtensionsNonEmpty)
{
    EXPECT_FALSE(TextIndexConfig::instance().supportedTextFileExtensions().isEmpty());
}

TEST(TextIndexConfigTest, SupportedOcrImageExtensions)
{
    EXPECT_NO_FATAL_FAILURE({ (void)TextIndexConfig::instance().supportedOcrImageExtensions(); });
}

TEST(TextIndexConfigTest, MaxOcrImageSizeMBPositive)
{
    EXPECT_GT(TextIndexConfig::instance().maxOcrImageSizeMB(), 0);
}

TEST(TextIndexConfigTest, IndexHiddenFilesReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)TextIndexConfig::instance().indexHiddenFiles(); });
}

TEST(TextIndexConfigTest, FolderExcludeFilters)
{
    EXPECT_NO_FATAL_FAILURE({ (void)TextIndexConfig::instance().folderExcludeFilters(); });
}

TEST(TextIndexConfigTest, CpuUsageLimitPercent)
{
    EXPECT_NO_FATAL_FAILURE({ (void)TextIndexConfig::instance().cpuUsageLimitPercent(); });
}

TEST(TextIndexConfigTest, BatchCommitInterval)
{
    EXPECT_NO_FATAL_FAILURE({ (void)TextIndexConfig::instance().batchCommitInterval(); });
}

TEST(TextIndexConfigTest, ReloadConfigNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ TextIndexConfig::instance().reloadConfig(); });
}
