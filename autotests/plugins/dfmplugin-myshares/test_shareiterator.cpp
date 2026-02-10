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
#include <QScopedPointer>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

class UT_ShareIterator : public testing::Test
{
public:
    virtual void SetUp() override
    {
        it = new ShareIterator(QUrl("usershare:///"), {}, QDir::AllDirs, QDirIterator::NoIteratorFlags);
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

DFMBASE_USE_NAMESPACE

TEST_F(UT_ShareIterator, Next)
{
    EXPECT_FALSE(it->next().isValid());

    it->d->shares.append(QVariantMap());
    EXPECT_TRUE(it->next().path().isEmpty());

    it->d->shares.append({ { "path", "/hello/world" } });
    EXPECT_TRUE(it->next().path() == "/hello/world");
}

TEST_F(UT_ShareIterator, HasNext)
{
    it->d->shares.clear();
    EXPECT_FALSE(it->hasNext());
    it->d->shares.append(QVariantMap());
    EXPECT_TRUE(it->hasNext());
}

TEST_F(UT_ShareIterator, FileName)
{
    it->d->currentInfo.insert("shareName", "Test");
    EXPECT_TRUE(it->fileName() == "Test");
}

TEST_F(UT_ShareIterator, FileUrl)
{
    it->d->currentInfo = { { "", "" } };
    EXPECT_TRUE(it->fileUrl().path().isEmpty());

    it->d->currentInfo = { { "path", "/hello/world" } };
    EXPECT_TRUE(it->fileUrl().isValid());
    EXPECT_EQ(it->fileUrl().path(), QString("/hello/world"));
}

TEST_F(UT_ShareIterator, FileInfo)
{
    stub.set_lamda(InfoFactory::create<FileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_TRUE(it->fileInfo() == nullptr);
}

TEST_F(UT_ShareIterator, Url)
{
    EXPECT_TRUE(it->url().path() == "/");
    EXPECT_TRUE(it->url().scheme() == "usershare");
}
