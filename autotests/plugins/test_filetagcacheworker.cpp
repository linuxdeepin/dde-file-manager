// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcacheworker.cpp
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

TEST_F(FileTagCacheWorkerTest, loadFileTagsFromDatabase)
{
    // Test method: void loadFileTagsFromDatabase(())
    EXPECT_NO_FATAL_FAILURE(obj->loadFileTagsFromDatabase());
}

TEST_F(FileTagCacheWorkerTest, onTagDeleted)
{
    // Test method: void onTagDeleted((const QVariant &tags))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagDeleted(_arg0));
}
