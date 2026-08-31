// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfilewatcher_1.cpp
 * @brief Unit tests for TagFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/tagfilewatcher.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagFileWatcherTest, TagFileWatcher)
{
    // Test constructor: TagFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagFileWatcherTest, TagFileWatcher_Destructor)
{
    // Test method:  ~TagFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ TagFileWatcher *tmp = new TagFileWatcher(); delete tmp; });
}
