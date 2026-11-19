// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "watcher/sharewatcher.h"
#include "private/sharewatcher_p.h"
#include "utils/shareutils.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QStringList>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_ShareWatcher : public testing::Test
{
    // Test interface
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

TEST_F(UT_ShareWatcher, ShareAdded_NullInfo_EmitsSubfileCreated)
{
    stub.set_lamda(static_cast<QSharedPointer<FileInfo> (*)(const QUrl &, bool *, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>), [] { __DBG_STUB_INVOKE__ return nullptr; });
    QSignalSpy spy(watcher, &AbstractFileWatcher::subfileCreated);

    watcher->shareAdded("/hello/world");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toUrl(), ShareUtils::makeShareUrl("/hello/world"));
}

TEST_F(UT_ShareWatcher, ShareAdded_ValidInfo_RefreshesInfoAndEmits)
{
    bool infoCreated = false;
    stub.set_lamda(static_cast<QSharedPointer<FileInfo> (*)(const QUrl &, bool *, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>), [&infoCreated]() -> FileInfoPointer {
        infoCreated = true;
        return FileInfoPointer(new FileInfo(QUrl::fromLocalFile("/tmp")));
    });
    QSignalSpy spy(watcher, &AbstractFileWatcher::subfileCreated);

    watcher->shareAdded("/data/share");
    EXPECT_TRUE(infoCreated);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toUrl(), ShareUtils::makeShareUrl("/data/share"));
}

TEST_F(UT_ShareWatcher, ShareRemoved_EmitsFileDeleted)
{
    QSignalSpy spy(watcher, &AbstractFileWatcher::fileDeleted);

    watcher->shareRemoved("/bye/share");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toUrl(), ShareUtils::makeShareUrl("/bye/share"));
}

class UT_ShareWatcherPrivate : public testing::Test
{
    // Test interface
public:
    virtual void SetUp() override
    {
        watcher = new ShareWatcher(QUrl("usershare:///"));
        d = dynamic_cast<ShareWatcherPrivate *>(watcher->d.data());
        ASSERT_TRUE(d != nullptr);
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (d)
            d->stop();
        delete watcher;
        watcher = nullptr;
        d = nullptr;
    }

    static void ensureSignals()
    {
        const QStringList topics {
            "signal_Share_ShareAdded",
            "signal_Share_ShareRemoved",
            "signal_Share_ShareChanged"
        };
        for (const QString &topic : topics) {
            if (Event::instance()->eventType("dfmplugin_dirshare", topic) == EventTypeScope::kInValid)
                Event::instance()->registerEventType(EventStratege::kSignal, "dfmplugin_dirshare", topic);
        }
    }

    stub_ext::StubExt stub;
    ShareWatcher *watcher { nullptr };
    ShareWatcherPrivate *d { nullptr };
};

TEST_F(UT_ShareWatcherPrivate, Start_WhenSubscribeSucceeds_ReturnsTrue)
{
    ensureSignals();

    EXPECT_TRUE(d->start());
    EXPECT_NO_FATAL_FAILURE(d->start());
}

TEST_F(UT_ShareWatcherPrivate, Start_WhenSubscribeFails_ReturnsFalse)
{
    const auto savedConverter = EventConverter::convertFunc;
    EventConverter::convertFunc = [](const QString &, const QString &) { return EventTypeScope::kInValid; };
    const bool started = d->start();
    EventConverter::convertFunc = savedConverter;

    EXPECT_FALSE(started);
}

TEST_F(UT_ShareWatcherPrivate, Stop_WhenUnsubscribeSucceeds_ReturnsTrue)
{
    ensureSignals();
    ASSERT_TRUE(d->start());

    EXPECT_TRUE(d->stop());
    EXPECT_NO_FATAL_FAILURE(d->stop());
}

TEST_F(UT_ShareWatcherPrivate, Stop_WhenUnsubscribeFails_ReturnsFalse)
{
    const auto savedConverter = EventConverter::convertFunc;
    EventConverter::convertFunc = [](const QString &, const QString &) { return EventTypeScope::kInValid; };
    const bool stopped = d->stop();
    EventConverter::convertFunc = savedConverter;

    EXPECT_FALSE(stopped);
}

TEST_F(UT_ShareWatcherPrivate, StartWatcher_PublicEntry_StartsAndIsIdempotent)
{
    ensureSignals();

    EXPECT_TRUE(watcher->startWatcher());
    // second call early-returns true because it is already started
    EXPECT_TRUE(watcher->startWatcher());
}

TEST_F(UT_ShareWatcherPrivate, StopWatcher_PublicEntry_StopsAndIsIdempotent)
{
    ensureSignals();

    ASSERT_TRUE(watcher->startWatcher());
    EXPECT_TRUE(watcher->stopWatcher());
    // not started any more: early-returns true
    EXPECT_TRUE(watcher->stopWatcher());
}
