// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "mastered/masteredmediafileinfo.h"
#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QUrl>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;
using namespace GlobalServerDefines;

class TestMasteredMediaFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        testUrl = QUrl("burn:///dev/sr0/staging/test.txt");
        rootUrl = QUrl("burn:///dev/sr0");
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    QUrl testUrl;
    QUrl rootUrl;
    stub_ext::StubExt stub;
};

TEST_F(TestMasteredMediaFileInfo, Constructor_WithUrl_CreatesFileInfo)
{
    MasteredMediaFileInfo fileInfo(testUrl);
    EXPECT_EQ(fileInfo.urlOf(FileInfo::FileUrlInfoType::kUrl), testUrl);
}

TEST_F(TestMasteredMediaFileInfo, Constructor_WithUrlAndProxy_CreatesFileInfoWithProxy)
{
    FileInfoPointer proxy(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    MasteredMediaFileInfo fileInfo(testUrl, proxy);
    EXPECT_EQ(fileInfo.urlOf(FileInfo::FileUrlInfoType::kUrl), testUrl);
}

TEST_F(TestMasteredMediaFileInfo, UrlOf_Url_ReturnsOriginalUrl)
{
    MasteredMediaFileInfo fileInfo(testUrl);
    QUrl result = fileInfo.urlOf(FileInfo::FileUrlInfoType::kUrl);
    EXPECT_EQ(result, testUrl);
}

TEST_F(TestMasteredMediaFileInfo, IsAttributes_IsReadable_NoProxy_ReturnsTrue)
{
    MasteredMediaFileInfo fileInfo(testUrl);
    bool result = fileInfo.isAttributes(FileInfo::FileIsType::kIsReadable);
    EXPECT_TRUE(result);
}

TEST_F(TestMasteredMediaFileInfo, IsAttributes_IsWritable_NoProxy_ReturnsFalse)
{
    MasteredMediaFileInfo fileInfo(testUrl);
    bool result = fileInfo.isAttributes(FileInfo::FileIsType::kIsWritable);
    EXPECT_FALSE(result);
}

TEST_F(TestMasteredMediaFileInfo, IsAttributes_IsWritable_WithProxy_ReturnsProxyResult)
{
    FileInfoPointer proxy(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    bool proxyIsAttributesCalled = false;

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [&](FileInfo *obj, OptInfoType type) {
        __DBG_STUB_INVOKE__
        proxyIsAttributesCalled = true;
        return true;
    });

    MasteredMediaFileInfo fileInfo(testUrl, proxy);
    bool result = fileInfo.isAttributes(FileInfo::FileIsType::kIsWritable);

    EXPECT_TRUE(proxyIsAttributesCalled);
    EXPECT_TRUE(result);
}

TEST_F(TestMasteredMediaFileInfo, SupportedOfAttributes_Drop_BurnDisabled_ReturnsIgnoreAction)
{
    stub.set_lamda(ADDR(OpticalHelper, isBurnEnabled), []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    MasteredMediaFileInfo fileInfo(testUrl);
    Qt::DropActions result = fileInfo.supportedOfAttributes(FileInfo::SupportType::kDrop);

    EXPECT_EQ(result, Qt::IgnoreAction);
}

TEST_F(TestMasteredMediaFileInfo, SupportedOfAttributes_Drop_IsWritable_ReturnsAllActions)
{
    stub.set_lamda(ADDR(OpticalHelper, isBurnEnabled), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [](FileInfo *obj, OptInfoType type) {
        __DBG_STUB_INVOKE__
        return true;
    });

    MasteredMediaFileInfo fileInfo(testUrl);
    Qt::DropActions result = fileInfo.supportedOfAttributes(FileInfo::SupportType::kDrop);

    EXPECT_EQ(result, Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
}

TEST_F(TestMasteredMediaFileInfo, ViewOfTip_EmptyDir_ReturnsLocalizedString)
{
    // Test the public interface without relying on private methods
    MasteredMediaFileInfo fileInfo(rootUrl);
    QString result = fileInfo.viewOfTip(FileInfo::ViewType::kEmptyDir);

    // Just verify it returns a non-empty string for empty directories
    EXPECT_FALSE(result.isEmpty());
}
