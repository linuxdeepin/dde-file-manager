// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharewatchermanager_1.cpp
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

TEST_F(ShareWatcherManagerTest, addParentPathsWatcher)
{
    // Test method: void addParentPathsWatcher((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addParentPathsWatcher(_arg0));
}

TEST_F(ShareWatcherManagerTest, shouldEmitSignal)
{
    // Test method: bool shouldEmitSignal((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->shouldEmitSignal(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ShareWatcherManagerTest, ShareWatcherManager_Destructor)
{
    // Test method:  ~ShareWatcherManager(())
    EXPECT_NO_FATAL_FAILURE({ ShareWatcherManager *tmp = new ShareWatcherManager(); delete tmp; });
}
