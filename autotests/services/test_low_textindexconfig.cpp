// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_textindexconfig.cpp
 * @brief Unit tests for TextIndexConfig methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/utils/textindexconfig.h"

#include <QTest>

using namespace src;

class TextIndexConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexConfig();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexConfig *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexConfigTest, TextIndexConfig)
{
    // Test constructor: TextIndexConfig((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TextIndexConfigTest, autoIndexUpdateInterval)
{
    // Test getter: int autoIndexUpdateInterval()
    auto result = obj->autoIndexUpdateInterval();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, batchCommitInterval)
{
    // Test getter: int batchCommitInterval()
    auto result = obj->batchCommitInterval();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, cpuLoadThresholdPercent)
{
    // Test getter: int cpuLoadThresholdPercent()
    auto result = obj->cpuLoadThresholdPercent();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, cpuUsageLimitPercent)
{
    // Test getter: int cpuUsageLimitPercent()
    auto result = obj->cpuUsageLimitPercent();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, diskBusyThresholdPercent)
{
    // Test getter: int diskBusyThresholdPercent()
    auto result = obj->diskBusyThresholdPercent();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, folderExcludeFilters)
{
    // Test getter: QStringList folderExcludeFilters()
    auto result = obj->folderExcludeFilters();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TextIndexConfigTest, idleThresholdSeconds)
{
    // Test getter: int idleThresholdSeconds()
    auto result = obj->idleThresholdSeconds();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, indexHiddenFiles)
{
    // Test bool getter: indexHiddenFiles()
    bool result = obj->indexHiddenFiles();
    EXPECT_FALSE(result);

}

TEST_F(TextIndexConfigTest, inotifyResourceCleanupDelayMs)
{
    // Test getter: qint64 inotifyResourceCleanupDelayMs()
    auto result = obj->inotifyResourceCleanupDelayMs();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, inotifyWatchesCoefficient)
{
    // Test getter: double inotifyWatchesCoefficient()
    auto result = obj->inotifyWatchesCoefficient();
    EXPECT_EQ(result, 0.0);

}

TEST_F(TextIndexConfigTest, loadSampleIntervalSeconds)
{
    // Test getter: int loadSampleIntervalSeconds()
    auto result = obj->loadSampleIntervalSeconds();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, maxIndexFileTruncationSizeMB)
{
    // Test getter: int maxIndexFileTruncationSizeMB()
    auto result = obj->maxIndexFileTruncationSizeMB();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, maxIndexTextFileSizeMB)
{
    // Test getter: int maxIndexTextFileSizeMB()
    auto result = obj->maxIndexTextFileSizeMB();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, maxOcrImageSizeMB)
{
    // Test getter: int maxOcrImageSizeMB()
    auto result = obj->maxOcrImageSizeMB();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, monitoringStartDelaySeconds)
{
    // Test getter: int monitoringStartDelaySeconds()
    auto result = obj->monitoringStartDelaySeconds();
    EXPECT_EQ(result, 0);

}

TEST_F(TextIndexConfigTest, reloadConfig)
{
    // Test method: void reloadConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->reloadConfig());
}

TEST_F(TextIndexConfigTest, setupConnections)
{
    // Test method: void setupConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->setupConnections());
}

TEST_F(TextIndexConfigTest, silentIndexUpdateDelay)
{
    // Test getter: int silentIndexUpdateDelay()
    auto result = obj->silentIndexUpdateDelay();
    EXPECT_EQ(result, 0);

}
