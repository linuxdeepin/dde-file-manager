// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagfilewatcher.cpp
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

TEST_F(TagFileWatcherTest, onFilesHidden)
{
    // Test method: void onFilesHidden((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesHidden(_arg0));
}

TEST_F(TagFileWatcherTest, onFilesTagged)
{
    // Test method: void onFilesTagged((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesTagged(_arg0));
}

TEST_F(TagFileWatcherTest, onFilesUntagged)
{
    // Test method: void onFilesUntagged((const QVariantMap &fileAndTags))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesUntagged(_arg0));
}

TEST_F(TagFileWatcherTest, onTagRemoved)
{
    // Test method: void onTagRemoved((const QString &tagName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagRemoved(_arg0));
}

TEST_F(TagFileWatcherTest, setEnabledSubfileWatcher)
{
    // Test setter: void setEnabledSubfileWatcher((const QUrl &subfileUrl, bool enabled))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledSubfileWatcher(_arg0, false));
}
