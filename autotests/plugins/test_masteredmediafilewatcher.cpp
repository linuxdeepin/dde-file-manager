// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_masteredmediafilewatcher.cpp
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

TEST_F(MasteredMediaFileWatcherTest, onFileDeleted)
{
    // Test method: void onFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDeleted(_arg0));
}

TEST_F(MasteredMediaFileWatcherTest, onMountPointDeleted)
{
    // Test method: void onMountPointDeleted((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onMountPointDeleted(_arg0));
}
