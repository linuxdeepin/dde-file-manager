// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "fileinfo/sharefileinfo.h"
#include "private/sharefileinfo_p.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QVariantMap>
#include <QScopedPointer>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

class UT_ShareFileInfo : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override
    {
        conStub.set_lamda(&ShareFileInfo::setProxy, [] { __DBG_STUB_INVOKE__ });
        conStub.set_lamda(InfoFactory::create<FileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
        typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
        auto pushAddr = static_cast<Push>(&dpf::EventChannelManager::push);
        conStub.set_lamda(pushAddr, [] { __DBG_STUB_INVOKE__ return QVariant(); });
        info = new ShareFileInfo(QUrl("share:///test"));
        auto d = dynamic_cast<ShareFileInfoPrivate *>(info->d.data());
        d->info = { { "shareName", "test" },
                    { "path", "/test" },
                    { "comment", "" },
                    { "acl", "" },
                    { "guestEnable", "" },
                    { "writable", "" } };
    }

    virtual void TearDown() override
    {
        conStub.clear();
        delete info;
        info = nullptr;
    }

    stub_ext::StubExt conStub;
    ShareFileInfo *info { nullptr };
};

TEST_F(UT_ShareFileInfo, RedirectedFileUrl)
{
    EXPECT_TRUE(info->urlOf(UrlInfoType::kRedirectedFileUrl) == QUrl::fromLocalFile("/test"));
}

TEST_F(UT_ShareFileInfo, FileDisplayName)
{
    conStub.set_lamda(UrlRoute::isRootUrl, [] { return false; });
    EXPECT_EQ(info->displayOf(DisPlayInfoType::kFileDisplayName), "test");
}

TEST_F(UT_ShareFileInfo, FileName)
{
    EXPECT_TRUE(info->nameOf(NameInfoType::kFileName) == "test");
}

TEST_F(UT_ShareFileInfo, IsDir)
{
    EXPECT_NO_FATAL_FAILURE(info->isAttributes(OptInfoType::kIsDir));
}

TEST_F(UT_ShareFileInfo, CanRename)
{
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanRename));
}

TEST_F(UT_ShareFileInfo, CanDrag)
{
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanDrag));
}

TEST_F(UT_ShareFileInfo, IsWritable)
{
    EXPECT_NO_FATAL_FAILURE(info->isAttributes(OptInfoType::kIsWritable));
}

TEST_F(UT_ShareFileInfo, Refresh)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&ShareFileInfoPrivate::refresh, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(info->refresh());
}

class UT_ShareFileInfoPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        info = new ShareFileInfo(QUrl("share:///test"));
        d = dynamic_cast<ShareFileInfoPrivate *>(info->d.data());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
    }

private:
    stub_ext::StubExt stub;
    ShareFileInfo *info { nullptr };
    ShareFileInfoPrivate *d { nullptr };
};

TEST_F(UT_ShareFileInfoPrivate, Refresh)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    EXPECT_NO_FATAL_FAILURE(d->refresh());
}
