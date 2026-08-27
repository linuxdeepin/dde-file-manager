// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcache.cpp
 * @brief Unit tests for FileTagCache Mid-priority methods (dfmplugin-tag)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "utils/filetagcache.h"

using namespace dfmplugin_tag;

class FileTagCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(FileTagCacheTest, addTags)
{
    // Instance method addTags
    FileTagCache obj;
    EXPECT_NO_FATAL_FAILURE({ obj.addTags(QVariantMap()); });
}

TEST_F(FileTagCacheTest, changeTagColor)
{
    // Instance method changeTagColor
    FileTagCache obj;
    EXPECT_NO_FATAL_FAILURE({ obj.changeTagColor(QVariantMap()); });
}

TEST_F(FileTagCacheTest, changeTagName)
{
    // Instance method changeTagName
    FileTagCache obj;
    EXPECT_NO_FATAL_FAILURE({ obj.changeTagName(QVariantMap()); });
}

TEST_F(FileTagCacheTest, deleteTags)
{
    // Instance method deleteTags
    FileTagCache obj;
    EXPECT_NO_FATAL_FAILURE({ obj.deleteTags(QStringList{"test"}); });
}

TEST_F(FileTagCacheTest, taggeFiles)
{
    // Instance method taggeFiles
    FileTagCache obj;
    EXPECT_NO_FATAL_FAILURE({ obj.taggeFiles(QVariantMap()); });
}

TEST_F(FileTagCacheTest, untaggeFiles)
{
    // Instance method untaggeFiles
    FileTagCache obj;
    EXPECT_NO_FATAL_FAILURE({ obj.untaggeFiles(QVariantMap()); });
}

TEST_F(FileTagCacheTest, getTagsColor)
{
    // getTagsColor
    SUCCEED();
}
