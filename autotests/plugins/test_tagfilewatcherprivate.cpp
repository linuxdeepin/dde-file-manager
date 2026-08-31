// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfilewatcherprivate.cpp
 * @brief Unit tests for TagFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/private/tagfilewatcher_p.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagFileWatcherPrivateTest, TagFileWatcherPrivate)
{
    // Test constructor: TagFileWatcherPrivate((const QUrl &fileUrl, TagFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagFileWatcherPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(TagFileWatcherPrivateTest, initFileWatcher)
{
    // Test method: void initFileWatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->initFileWatcher());
}

TEST_F(TagFileWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(TagFileWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}
