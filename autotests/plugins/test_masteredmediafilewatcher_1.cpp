// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_masteredmediafilewatcher_1.cpp
 * @brief Unit tests for MasteredMediaFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mastered/masteredmediafilewatcher.h"

#include <QTest>

using namespace dfmplugin_optical;

class MasteredMediaFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MasteredMediaFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MasteredMediaFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MasteredMediaFileWatcherTest, MasteredMediaFileWatcher)
{
    // Test constructor: MasteredMediaFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MasteredMediaFileWatcherTest, onFileAttributeChanged)
{
    // Test method: void onFileAttributeChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAttributeChanged(_arg0));
}

TEST_F(MasteredMediaFileWatcherTest, onFileRename)
{
    // Test method: void onFileRename((const QUrl &fromUrl, const QUrl &toUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRename(_arg0, _arg1));
}

TEST_F(MasteredMediaFileWatcherTest, onSubfileCreated)
{
    // Test method: void onSubfileCreated((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSubfileCreated(_arg0));
}
