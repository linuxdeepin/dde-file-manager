// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/syncfileinfo.h"

#include "stubext.h"

#include <QUrl>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class TestFileInfo : public FileInfo
{
public:
    explicit TestFileInfo(const QUrl &url)
        : FileInfo(url) {}
    virtual ~TestFileInfo() {}

    virtual bool canAttributes(const FileCanType) const { return true; }
    virtual QUrl urlOf(const FileUrlInfoType) const { return url; }
};

TEST(UT_UniversalUtils, testUrlsTransformToLocal_1)
{
    QList<QUrl> srcUrls { QUrl::fromLocalFile("/tmp") };
    QList<QUrl> targetUrls;
    bool ret = UniversalUtils::urlsTransformToLocal(srcUrls, &targetUrls);

    EXPECT_FALSE(ret);
    EXPECT_EQ(srcUrls.size(), targetUrls.size());
    EXPECT_TRUE(targetUrls.first().isLocalFile());
}

TEST(UT_UniversalUtils, testUrlsTransformToLocal_2)
{
    stub_ext::StubExt st;
    st.set_lamda(InfoFactory::create<FileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });

    QList<QUrl> srcUrls { QUrl::fromUserInput("test:///tmp") };
    QList<QUrl> targetUrls;
    bool ret = UniversalUtils::urlsTransformToLocal(srcUrls, &targetUrls);

    EXPECT_FALSE(ret);
    EXPECT_EQ(srcUrls.size(), targetUrls.size());
    EXPECT_FALSE(targetUrls.first().isLocalFile());
}

TEST(UT_UniversalUtils, testUrlsTransformToLocal_3)
{
    stub_ext::StubExt st;
    st.set_lamda(InfoFactory::create<FileInfo>, [] {
        __DBG_STUB_INVOKE__
        QSharedPointer<FileInfo> info { new TestFileInfo(QUrl::fromLocalFile("/tmp")) };
        return info;
    });

    QList<QUrl> srcUrls { QUrl::fromUserInput("test:///tmp") };
    QList<QUrl> targetUrls;
    bool ret = UniversalUtils::urlsTransformToLocal(srcUrls, &targetUrls);

    EXPECT_TRUE(ret);
    EXPECT_EQ(srcUrls.size(), targetUrls.size());
    EXPECT_TRUE(targetUrls.first().isLocalFile());
}
