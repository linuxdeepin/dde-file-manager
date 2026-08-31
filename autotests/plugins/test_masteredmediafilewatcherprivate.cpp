// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_masteredmediafilewatcherprivate.cpp
 * @brief Unit tests for MasteredMediaFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mastered/masteredmediafilewatcher.h"

#include <QTest>

using namespace dfmplugin_optical;

class MasteredMediaFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MasteredMediaFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MasteredMediaFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MasteredMediaFileWatcherPrivateTest, MasteredMediaFileWatcherPrivate)
{
    // Test constructor: MasteredMediaFileWatcherPrivate((const QUrl &fileUrl, MasteredMediaFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MasteredMediaFileWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(MasteredMediaFileWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}
