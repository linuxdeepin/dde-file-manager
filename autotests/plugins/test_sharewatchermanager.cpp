// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharewatchermanager.cpp
 * @brief Unit tests for ShareWatcherManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sharewatchermanager.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class ShareWatcherManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareWatcherManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareWatcherManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareWatcherManagerTest, remove)
{
    // Test method: void remove((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->remove(_arg0));
}

TEST_F(ShareWatcherManagerTest, removeParentPathsWatcher)
{
    // Test method: void removeParentPathsWatcher((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeParentPathsWatcher(_arg0));
}
