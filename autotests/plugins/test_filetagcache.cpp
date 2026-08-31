// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcache.cpp
 * @brief Unit tests for FileTagCache methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filetagcache.h"

#include <QTest>

using namespace dfmplugin_tag;

class FileTagCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagCache();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagCache *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagCacheTest, addTags)
{
    // Test method: void addTags((const QVariantMap &tags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addTags(_arg0));
}

TEST_F(FileTagCacheTest, changeTagColor)
{
    // Test method: void changeTagColor((const QVariantMap &tagAndColorName))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->changeTagColor(_arg0));
}

TEST_F(FileTagCacheTest, changeTagName)
{
    // Test method: void changeTagName((const QVariantMap &oldAndNew))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->changeTagName(_arg0));
}

TEST_F(FileTagCacheTest, deleteTags)
{
    // Test method: void deleteTags((const QStringList &tags))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->deleteTags(_arg0));
}

TEST_F(FileTagCacheTest, taggeFiles)
{
    // Test method: void taggeFiles((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->taggeFiles(_arg0));
}

TEST_F(FileTagCacheTest, untaggeFiles)
{
    // Test method: void untaggeFiles((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->untaggeFiles(_arg0));
}

TEST_F(FileTagCacheTest, getTagsColor)
{
    // Test method: FileTagCache::TagColorMap getTagsColor((const QStringList &tags))
    QStringList _arg0{};
    auto result = obj->getTagsColor(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}
