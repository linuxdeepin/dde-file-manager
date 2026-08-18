// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "iterator/shareiterator.h"
#include "private/shareiterator_p.h"
#include "utils/shareutils.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QVariantMap>
#include <QTemporaryDir>
#include <QFile>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

class UT_ShareIterator : public testing::Test
{
    // Test interface
public:
    virtual void SetUp() override
    {
        typedef QVariant (dpf::EventChannelManager::*PushAll)(const QString &, const QString &);
        stub.set_lamda(static_cast<PushAll>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
        it = new ShareIterator(QUrl("usershare:///"), {}, QDir::NoFilter, QDirIterator::NoIteratorFlags);
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete it;
        it = nullptr;
    }

    stub_ext::StubExt stub;
    ShareIterator *it { nullptr };
};

TEST_F(UT_ShareIterator, Ctor_RootUrl_LoadsSharesFromSlot)
{
    stub_ext::StubExt local;
    typedef QVariant (dpf::EventChannelManager::*PushAll)(const QString &, const QString &);
    local.set_lamda(static_cast<PushAll>(&dpf::EventChannelManager::push), [] {
        const QList<QVariantMap> shares {
            { { ShareInfoKeys::kName, "a" }, { ShareInfoKeys::kPath, "/a" } },
            { { ShareInfoKeys::kName, "b" }, { ShareInfoKeys::kPath, "/b" } }
        };
        return QVariant::fromValue(shares);
    });

    ShareIterator seeded(QUrl("usershare:///"));
    EXPECT_TRUE(seeded.d->proxy == nullptr);
    EXPECT_TRUE(seeded.hasNext());
    EXPECT_EQ(seeded.next().path(), QString("/a"));
    EXPECT_TRUE(seeded.hasNext());
    EXPECT_EQ(seeded.next().path(), QString("/b"));
    EXPECT_FALSE(seeded.hasNext());
    EXPECT_FALSE(seeded.next().isValid());
}

TEST_F(UT_ShareIterator, Ctor_RootUrl_CreatesNoProxy)
{
    EXPECT_TRUE(it->d->proxy == nullptr);
}

TEST_F(UT_ShareIterator, Next_EmptyShares_ReturnsInvalidUrl)
{
    EXPECT_FALSE(it->next().isValid());
}

TEST_F(UT_ShareIterator, Next_TakesFirstShareAndReturnsItsUrl)
{
    it->d->shares.append({ { ShareInfoKeys::kPath, "/hello/world" } });
    EXPECT_EQ(it->next().path(), QString("/hello/world"));
    EXPECT_TRUE(it->d->shares.isEmpty());
    EXPECT_FALSE(it->next().isValid());
}

TEST_F(UT_ShareIterator, Next_ShareWithoutPath_ReturnsEmptyPathUrl)
{
    it->d->shares.append(QVariantMap());
    const QUrl url = it->next();
    EXPECT_TRUE(url.path().isEmpty());
}

TEST_F(UT_ShareIterator, HasNext_ReflectsSharesEmptiness)
{
    it->d->shares.clear();
    EXPECT_FALSE(it->hasNext());
    it->d->shares.append(QVariantMap());
    EXPECT_TRUE(it->hasNext());
}

TEST_F(UT_ShareIterator, FileName_ReturnsCurrentShareName)
{
    it->d->currentInfo = { { ShareInfoKeys::kName, "Test" } };
    EXPECT_EQ(it->fileName(), QString("Test"));

    it->d->currentInfo.clear();
    EXPECT_TRUE(it->fileName().isEmpty());
}

TEST_F(UT_ShareIterator, FileUrl_MakesShareUrlFromCurrentSharePath)
{
    it->d->currentInfo.clear();
    EXPECT_TRUE(it->fileUrl().path().isEmpty());

    it->d->currentInfo = { { ShareInfoKeys::kPath, "/hello/world" } };
    const QUrl url = it->fileUrl();
    EXPECT_EQ(url.scheme(), QString("usershare"));
    EXPECT_EQ(url.path(), QString("/hello/world"));
}

TEST_F(UT_ShareIterator, FileInfo_ReturnsInfoFactoryResult)
{
    stub.set_lamda(InfoFactory::create<FileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
    it->d->currentInfo = { { ShareInfoKeys::kPath, "/hello/world" } };
    EXPECT_TRUE(it->fileInfo() == nullptr);
}

TEST_F(UT_ShareIterator, Url_ValidRootUrl_ReturnsRootUrl)
{
    EXPECT_EQ(it->url(), QUrl("usershare:///"));
    EXPECT_EQ(it->url().scheme(), QString("usershare"));
    EXPECT_EQ(it->url().path(), QString("/"));
}

TEST_F(UT_ShareIterator, Url_InvalidRootUrl_FallsBackToShareRootUrl)
{
    EXPECT_EQ(it->url(), QUrl("usershare:///"));
    it->d->rootUrl = QUrl();
    EXPECT_FALSE(it->d->rootUrl.isValid());
    EXPECT_EQ(it->url().scheme(), ShareUtils::rootUrl().scheme());
    EXPECT_EQ(it->url().path(), ShareUtils::rootUrl().path());
}

class UT_ShareIteratorProxy : public testing::Test
{
    // Test interface
public:
    virtual void SetUp() override
    {
        UrlRoute::regScheme("file", "/");

        tmpDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tmpDir->isValid());
        QFile helloFile(tmpDir->path() + "/hello.txt");
        ASSERT_TRUE(helloFile.open(QIODevice::WriteOnly));
        ASSERT_TRUE(helloFile.write("hello") > 0);
        helloFile.close();

        typedef QVariant (dpf::EventChannelManager::*PushAll)(const QString &, const QString &);
        stub.set_lamda(static_cast<PushAll>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });

        const QUrl shareUrl = ShareUtils::makeShareUrl(tmpDir->path());
        it = new ShareIterator(shareUrl, {}, QDir::Files, QDirIterator::NoIteratorFlags);
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete it;
        it = nullptr;
        tmpDir.reset();
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tmpDir;
    ShareIterator *it { nullptr };
};

TEST_F(UT_ShareIteratorProxy, Ctor_NonRootUrl_CreatesLocalDirIteratorProxy)
{
    EXPECT_TRUE(it->d->proxy != nullptr);
}

TEST_F(UT_ShareIteratorProxy, Next_WithProxy_WrapsLocalUrlsAsShareUrls)
{
    QStringList paths;
    int guard = 0;
    while (it->hasNext() && guard++ < 64) {
        const QUrl url = it->next();
        if (url.path().isEmpty())
            continue;
        EXPECT_EQ(url.scheme(), QString("usershare"));
        paths << url.path();
    }
    EXPECT_TRUE(paths.contains(tmpDir->path() + "/hello.txt"));
}

TEST_F(UT_ShareIteratorProxy, HasNext_WithProxy_ExhaustsAfterDraining)
{
    int guard = 0;
    while (it->hasNext() && guard++ < 64)
        it->next();
    EXPECT_FALSE(it->hasNext());
}

TEST_F(UT_ShareIteratorProxy, WithProxy_ReturnsLocalNameAndShareFileUrl)
{
    bool found = false;
    int guard = 0;
    while (it->hasNext() && guard++ < 64) {
        it->next();
        if (it->fileName() == "hello.txt") {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    EXPECT_EQ(it->fileName(), QString("hello.txt"));

    const QUrl url = it->fileUrl();
    EXPECT_EQ(url.scheme(), QString("usershare"));
    EXPECT_EQ(url.path(), tmpDir->path() + "/hello.txt");
}
