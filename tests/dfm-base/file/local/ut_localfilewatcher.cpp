// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_ABSTRACTFILEWATCHER
#define UT_ABSTRACTFILEWATCHER

#include <dfm-base/file/local/localfilewatcher.h>

#include <QDir>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_LocalFileWatcher : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    ~UT_LocalFileWatcher() override;

    AbstractFileWatcherPointer watcher{ nullptr };
};

UT_LocalFileWatcher::~UT_LocalFileWatcher() {

}

TEST_F(UT_LocalFileWatcher, testLocalFileWatcher)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    watcher.reset(new LocalFileWatcher(url));
    EXPECT_TRUE(0 == watcher->getCacheInfoConnectSize());
    watcher->addCacheInfoConnectSize();
    EXPECT_EQ(1, watcher->getCacheInfoConnectSize());
    watcher->reduceCacheInfoConnectSize();
    EXPECT_TRUE(0 == watcher->getCacheInfoConnectSize());
    watcher->notifyFileAdded(url);
    watcher->notifyFileChanged(url);
    watcher->notifyFileDeleted(url);
    watcher->setEnabledSubfileWatcher(url);
    EXPECT_FALSE(watcher->stopWatcher());
    EXPECT_TRUE(watcher->startWatcher());
    EXPECT_TRUE(watcher->startWatcher());
    EXPECT_TRUE(watcher->stopWatcher());
    EXPECT_FALSE(watcher->stopWatcher());

}

#endif
