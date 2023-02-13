// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/dfmplugin-myshares/watcher/sharewatcher.h"
#include "plugins/filemanager/dfmplugin-myshares/private/sharewatcher_p.h"

#include <dfm-framework/event/eventdispatcher.h>

#include <gtest/gtest.h>

using namespace dfmplugin_myshares;
using namespace dpf;

class UT_ShareWatcher : public testing::Test
{
public:
    virtual void SetUp() override
    {
        watcher = new ShareWatcher(QUrl("usershare:///"));
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

    stub_ext::StubExt stub;
    ShareWatcher *watcher { nullptr };
};

TEST_F(UT_ShareWatcher, ShareAdded)
{
    EXPECT_NO_FATAL_FAILURE(watcher->shareAdded("hello"));
    EXPECT_NO_FATAL_FAILURE(watcher->shareAdded("world"));
}

TEST_F(UT_ShareWatcher, ShareRemoved)
{
    EXPECT_NO_FATAL_FAILURE(watcher->shareRemoved("hello"));
    EXPECT_NO_FATAL_FAILURE(watcher->shareRemoved("world"));
}

class UT_ShareWatcherPrivate : public testing::Test
{
public:
    virtual void SetUp() override
    {
        watcher = new ShareWatcher(QUrl("usershare:///"));
        d = dynamic_cast<ShareWatcherPrivate *>(watcher->d.data());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

    stub_ext::StubExt stub;
    ShareWatcher *watcher { nullptr };
    ShareWatcherPrivate *d { nullptr };
};

TEST_F(UT_ShareWatcherPrivate, Start)
{
    typedef void (ShareWatcher::*Callback)(const QString &);
    typedef bool (EventDispatcherManager::*Subscribe)(const QString &, const QString &, ShareWatcher *, Callback);
    auto subscribe = static_cast<Subscribe>(&EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_TRUE(d->start());
    EXPECT_NO_FATAL_FAILURE(d->start());
}

TEST_F(UT_ShareWatcherPrivate, Stop)
{
    typedef void (ShareWatcher::*Callback)(const QString &);
    typedef bool (EventDispatcherManager::*Unsubscribe)(const QString &, const QString &, ShareWatcher *, Callback);
    auto unsubscribe = static_cast<Unsubscribe>(&EventDispatcherManager::unsubscribe);
    stub.set_lamda(unsubscribe, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_TRUE(d->stop());
    EXPECT_NO_FATAL_FAILURE(d->stop());
}
