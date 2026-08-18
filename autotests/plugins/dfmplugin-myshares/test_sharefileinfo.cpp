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
#include <QSharedPointer>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

static const QUrl kTestUrl { "usershare:///test" };

class UT_ShareFileInfo : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override
    {
        conStub.set_lamda(&ShareFileInfo::setProxy, [] { __DBG_STUB_INVOKE__ });
        conStub.set_lamda(InfoFactory::create<FileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
        typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
        conStub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
        info = new ShareFileInfo(kTestUrl);
        d = info->d.data();
        d->info = { { ShareInfoKeys::kName, "test" },
                    { ShareInfoKeys::kPath, "/test" },
                    { ShareInfoKeys::kComment, "" },
                    { ShareInfoKeys::kAcl, "" },
                    { ShareInfoKeys::kGuestEnable, "" },
                    { ShareInfoKeys::kWritable, "" } };
    }

    virtual void TearDown() override
    {
        conStub.clear();
        delete info;
        info = nullptr;
        d = nullptr;
    }

    stub_ext::StubExt conStub;
    ShareFileInfo *info { nullptr };
    ShareFileInfoPrivate *d { nullptr };
};

TEST_F(UT_ShareFileInfo, UrlOf_RedirectedFileUrl_ReturnsLocalFileUrl)
{
    EXPECT_EQ(info->urlOf(UrlInfoType::kRedirectedFileUrl), QUrl::fromLocalFile("/test"));
}

TEST_F(UT_ShareFileInfo, UrlOf_Url_ReturnsOriginalShareUrl)
{
    EXPECT_EQ(info->urlOf(UrlInfoType::kUrl), kTestUrl);
}

TEST_F(UT_ShareFileInfo, UrlOf_UnknownType_FallsBackToProxyUrlOf)
{
    // null proxy -> FileInfo::urlOf(kOriginalUrl) returns the url itself.
    EXPECT_EQ(info->urlOf(UrlInfoType::kOriginalUrl), kTestUrl);
}

TEST_F(UT_ShareFileInfo, DisplayOf_NonRootUrl_ReturnsShareName)
{
    conStub.set_lamda(UrlRoute::isRootUrl, [] { return false; });
    EXPECT_EQ(info->displayOf(DisPlayInfoType::kFileDisplayName), QString("test"));
}

TEST_F(UT_ShareFileInfo, DisplayOf_RootUrl_ReturnsMyShares)
{
    conStub.set_lamda(UrlRoute::isRootUrl, [] { return true; });
    EXPECT_EQ(info->displayOf(DisPlayInfoType::kFileDisplayName), QObject::tr("My Shares"));
}

TEST_F(UT_ShareFileInfo, DisplayOf_EmptyShareName_FallsBackToProxyDisplay)
{
    conStub.set_lamda(UrlRoute::isRootUrl, [] { return false; });
    d->info.clear();
    // null proxy -> FileInfo::displayOf(kFileDisplayName) hits default branch: empty string.
    EXPECT_TRUE(info->displayOf(DisPlayInfoType::kFileDisplayName).isNull());
}

TEST_F(UT_ShareFileInfo, DisplayOf_UnknownType_FallsBackToProxyDisplay)
{
    EXPECT_TRUE(info->displayOf(DisPlayInfoType::kSizeDisplayName).isNull()
                || !info->displayOf(DisPlayInfoType::kSizeDisplayName).isEmpty());
}

TEST_F(UT_ShareFileInfo, NameOf_FileName_ReturnsShareName)
{
    EXPECT_EQ(info->nameOf(NameInfoType::kFileName), QString("test"));
}

TEST_F(UT_ShareFileInfo, NameOf_FileCopyName_ReturnsShareName)
{
    EXPECT_EQ(info->nameOf(NameInfoType::kFileCopyName), QString("test"));
}

TEST_F(UT_ShareFileInfo, NameOf_UnknownType_FallsBackToProxyNameOf)
{
    // null proxy -> FileInfo::nameOf(kMimeTypeName) hits default branch: empty string.
    EXPECT_TRUE(info->nameOf(NameInfoType::kMimeTypeName).isNull());
}

TEST_F(UT_ShareFileInfo, IsAttributes_DirWithNullProxy_ReturnsFalse)
{
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsDir));
}

TEST_F(UT_ShareFileInfo, IsAttributes_WritableWithNullProxy_ReturnsFalse)
{
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsWritable));
}

TEST_F(UT_ShareFileInfo, CanAttributes_CanRename_ReturnsFalse)
{
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanRename));
}

TEST_F(UT_ShareFileInfo, CanAttributes_CanDrag_ReturnsFalse)
{
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanDrag));
}

TEST_F(UT_ShareFileInfo, CanAttributes_CanRedirectionWithNullProxy_ReturnsFalse)
{
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanRedirectionFileUrl));
}

TEST_F(UT_ShareFileInfo, CanAttributes_UnknownType_FallsBackToProxyCanAttributes)
{
    // null proxy -> FileInfo::canAttributes(kCanHidden) returns true.
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanHidden));
}

TEST_F(UT_ShareFileInfo, Refresh_CallsBothProxyAndPrivateRefresh)
{
    bool privateRefreshed = false;
    conStub.set_lamda(&ShareFileInfoPrivate::refresh, [&privateRefreshed] { privateRefreshed = true; });
    EXPECT_NO_FATAL_FAILURE(info->refresh());
    EXPECT_TRUE(privateRefreshed);
}

TEST_F(UT_ShareFileInfo, UpdateAttributes_CallsBothProxyAndPrivateRefresh)
{
    bool privateRefreshed = false;
    conStub.set_lamda(&ShareFileInfoPrivate::refresh, [&privateRefreshed] { privateRefreshed = true; });
    EXPECT_NO_FATAL_FAILURE(info->updateAttributes());
    EXPECT_TRUE(privateRefreshed);
    EXPECT_NO_FATAL_FAILURE(info->updateAttributes({ FileInfo::FileInfoAttributeID::kStandardName }));
    EXPECT_TRUE(privateRefreshed);
}

class UT_ShareFileInfoWithProxy : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override
    {
        // setProxy is NOT stubbed here: a valid proxy is set from the factory result.
        proxyStub.set_lamda(InfoFactory::create<FileInfo>, [] {
            __DBG_STUB_INVOKE__
            return FileInfoPointer(new FileInfo(QUrl::fromLocalFile("/tmp")));
        });
        typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
        proxyStub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
        info = new ShareFileInfo(kTestUrl);
    }

    virtual void TearDown() override
    {
        proxyStub.clear();
        delete info;
        info = nullptr;
    }

    stub_ext::StubExt proxyStub;
    ShareFileInfo *info { nullptr };
};

TEST_F(UT_ShareFileInfoWithProxy, CanAttributes_CanRedirectionWithProxy_ReturnsTrue)
{
    EXPECT_FALSE(info->proxy.isNull());
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanRedirectionFileUrl));
}

TEST_F(UT_ShareFileInfoWithProxy, DisplayOf_EmptyNameDelegatesToProxyDisplay)
{
    proxyStub.set_lamda(UrlRoute::isRootUrl, [] { return false; });
    auto d = info->d.data();
    ASSERT_TRUE(d != nullptr);
    d->info.clear();
    // proxy is a plain FileInfo: displayOf(kFileDisplayName) hits default branch, empty string.
    EXPECT_TRUE(info->displayOf(DisPlayInfoType::kFileDisplayName).isNull());
}

class UT_ShareFileInfoPrivate : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&ShareFileInfo::setProxy, [] { __DBG_STUB_INVOKE__ });
        stub.set_lamda(InfoFactory::create<FileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
        typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
        stub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
        info = new ShareFileInfo(kTestUrl);
        d = info->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete info;
        info = nullptr;
        d = nullptr;
    }

    stub_ext::StubExt stub;
    ShareFileInfo *info { nullptr };
    ShareFileInfoPrivate *d { nullptr };
};

TEST_F(UT_ShareFileInfoPrivate, Ctor_NonRootUrl_LoadsInfoFromSlot)
{
    stub_ext::StubExt local;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    local.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] {
        return QVariant(QVariantMap { { ShareInfoKeys::kName, "fromSlot" } });
    });

    ShareFileInfo fresh(kTestUrl);
    auto *fd = fresh.d.data();
    ASSERT_TRUE(fd != nullptr);
    EXPECT_EQ(fd->fileName(), QString("fromSlot"));
}

TEST_F(UT_ShareFileInfoPrivate, Refresh_NonRootUrl_QueriesSlotAndUpdatesInfo)
{
    int pushed = 0;
    stub_ext::StubExt local;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    local.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [&pushed]() -> QVariant {
        ++pushed;
        return QVariant(QVariantMap { { ShareInfoKeys::kName, "refreshed" } });
    });

    d->info.clear();
    EXPECT_NO_FATAL_FAILURE(d->refresh());
    EXPECT_EQ(pushed, 1);
    EXPECT_EQ(d->fileName(), QString("refreshed"));
}

TEST_F(UT_ShareFileInfoPrivate, Refresh_RootUrl_SkipsSlotQuery)
{
    ShareFileInfo rootInfo(QUrl("usershare:///"));
    auto *rd = rootInfo.d.data();
    ASSERT_TRUE(rd != nullptr);

    bool pushed = false;
    stub_ext::StubExt local;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    local.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [&pushed]() -> QVariant {
        pushed = true;
        return QVariant();
    });

    EXPECT_NO_FATAL_FAILURE(rd->refresh());
    EXPECT_FALSE(pushed);
    EXPECT_TRUE(rd->fileName().isEmpty());
}

TEST_F(UT_ShareFileInfoPrivate, FileName_ReturnsShareNameOrEmpty)
{
    d->info = { { ShareInfoKeys::kName, "abc" } };
    EXPECT_EQ(d->fileName(), QString("abc"));

    d->info.clear();
    EXPECT_TRUE(d->fileName().isEmpty());
}
