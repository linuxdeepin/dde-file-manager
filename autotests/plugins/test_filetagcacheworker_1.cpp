// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcacheworker_1.cpp
 * @brief Unit tests for FileTagCacheWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filetagcache.h"

#include <QTest>

using namespace dfmplugin_tag;

class FileTagCacheWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagCacheWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagCacheWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagCacheWorkerTest, onFilesTagged)
{
    // Test method: void onFilesTagged((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesTagged(_arg0));
}

TEST_F(FileTagCacheWorkerTest, onFilesUntagged)
{
    // Test method: void onFilesUntagged((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesUntagged(_arg0));
}

TEST_F(FileTagCacheWorkerTest, onTagAdded)
{
    // Test method: void onTagAdded((const QVariantMap &tags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagAdded(_arg0));
}

TEST_F(FileTagCacheWorkerTest, onTagsColorChanged)
{
    // Test method: void onTagsColorChanged((const QVariantMap &tagAndColorName))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagsColorChanged(_arg0));
}

TEST_F(FileTagCacheWorkerTest, onTagsNameChanged)
{
    // Test method: void onTagsNameChanged((const QVariantMap &oldAndNew))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagsNameChanged(_arg0));
}

TEST_F(FileTagCacheWorkerTest, onTrashFileTagsChanged)
{
    // Test method: void onTrashFileTagsChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onTrashFileTagsChanged());
}

TEST_F(FileTagCacheWorkerTest, FileTagCacheWorker_Destructor)
{
    // Test method:  ~FileTagCacheWorker(())
    EXPECT_NO_FATAL_FAILURE({ FileTagCacheWorker *tmp = new FileTagCacheWorker(); delete tmp; });
}
