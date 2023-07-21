// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/tagfilewatcher.h"
#include "files/private/tagfilewatcher_p.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagFileWatcherTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
        stub.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [&w] {
            return w;
        });
        //        stub.set_lamda(&TagFileWatcherPrivate::initConnect, [] {});
        //        stub.set_lamda(&TagFileWatcherPrivate::initFileWatcher, [] {});
        watcher = new TagFileWatcher(TagHelper::rootUrl());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

protected:
    TagFileWatcher *watcher = { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TagFileWatcherTest, onTagRemoved)
{
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, []() {
        return TagHelper::rootUrl();
    });
    bool isRun = false;
    QObject::connect(watcher, &AbstractFileWatcher::fileDeleted, [&isRun](const QUrl url) {
        isRun = true;
        EXPECT_EQ(url, TagHelper::rootUrl());
    });
    watcher->onTagRemoved("红色");
    EXPECT_TRUE(isRun);
}

TEST_F(TagFileWatcherTest, onFilesTagged)
{
    stub.set_lamda(&TagHelper::getTagNameFromUrl, []() {
        return QString("红色");
    });
    bool isRun = false;
    QObject::connect(watcher, &AbstractFileWatcher::subfileCreated, [&isRun]() {
        isRun = true;
    });
    QVariantMap fileAndTags;
    fileAndTags.insert(TagHelper::rootUrl().toString(), QVariant(QString("红色")));
    watcher->onFilesTagged(fileAndTags);
    EXPECT_TRUE(isRun);
}

TEST_F(TagFileWatcherTest, onFilesUntagged)
{
    stub.set_lamda(&TagHelper::getTagNameFromUrl, []() {
        return QString("红色");
    });
    bool isRun = false;
    QObject::connect(watcher, &AbstractFileWatcher::fileDeleted, [&isRun]() {
        isRun = true;
    });
    QVariantMap fileAndTags;
    fileAndTags.insert(TagHelper::rootUrl().toString(), QVariant(QString("红色")));
    watcher->onFilesUntagged(fileAndTags);
    EXPECT_TRUE(isRun);
}

TEST_F(TagFileWatcherTest, onFilesHidden)
{
    stub.set_lamda(&TagHelper::getTagNameFromUrl, []() {
        return QString("红色");
    });
    bool isRun = false;
    QObject::connect(watcher, &AbstractFileWatcher::fileAttributeChanged, [&isRun]() {
        isRun = true;
    });
    QVariantMap fileAndTags;
    fileAndTags.insert(TagHelper::rootUrl().toString(), QVariant(QString("红色")));
    watcher->onFilesHidden(fileAndTags);
    EXPECT_TRUE(isRun);
}

TEST_F(TagFileWatcherTest, start)
{
    EXPECT_TRUE(watcher->dptr->start());
}

TEST_F(TagFileWatcherTest, stop)
{
    EXPECT_FALSE(watcher->dptr->stop());
}
