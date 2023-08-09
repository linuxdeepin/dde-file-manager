// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_LOCALFILEWATCHER
#define UT_LOCALFILEWATCHER

#include <stubext.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>

#include <dfm-io/dwatcher.h>

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

    LocalFileWatcherPrivate * watherDptr = static_cast<LocalFileWatcherPrivate *>(watcher->d.data());
    stub_ext::StubExt stub;
    stub.set_lamda(&dfmio::DWatcher::start, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(watherDptr->start());
    watherDptr->watcher = nullptr;

    EXPECT_FALSE(watherDptr->start());
    EXPECT_FALSE(watherDptr->stop());
}

#endif
