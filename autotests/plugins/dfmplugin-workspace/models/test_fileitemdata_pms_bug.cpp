// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "models/fileitemdata.h"
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QVariant>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class TestFileItemDataBug : public ::testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        ASSERT_TRUE(tempDir.isValid());
        QString filePath = tempDir.filePath("remote_test.txt");
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("test");
        f.close();
        testUrl = QUrl::fromLocalFile(filePath);

        // Pre-create a valid FileInfo to return from the stubbed factory
        cachedInfo = InfoFactory::create<FileInfo>(testUrl);
        ASSERT_TRUE(cachedInfo);
    }

    void TearDown() override
    {
        stub.clear();
    }

    QTemporaryDir tempDir;
    QUrl testUrl;
    FileInfoPointer cachedInfo;
    stub_ext::StubExt stub;
};

// PMS:379291(口述) 远程文件缓存命中时仍调用 updateAttributes()
// 修复 (88f865d3): 在 data(kItemCreateFileInfoRole) 中，当 isCacheMiss 为 false
// 但 ProtocolUtils::isRemoteFile(url) 为 true 时，仍调用 updateAttributes()
TEST_F(TestFileItemDataBug, RemoteFileCacheHit_StillCallsUpdateAttributes)
{
    bool updateAttributesCalled = false;

    // Stub InfoFactory::create to return cached info with isCacheMiss=false (cache hit)
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl &, bool *, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [this](const QUrl &, bool *isCacheMiss, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       if (isCacheMiss)
                           *isCacheMiss = false;   // cache hit
                       return cachedInfo;
                   });

    // Stub ProtocolUtils::isRemoteFile to return true for our test URL
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Stub updateAttributes (virtual, on SyncFileInfo) to set flag
    stub.set_lamda(VADDR(SyncFileInfo, updateAttributes), [&updateAttributesCalled](SyncFileInfo *, const QList<FileInfo::FileInfoAttributeID> &) {
        __DBG_STUB_INVOKE__
        updateAttributesCalled = true;
    });

    // Create FileItemData with no initial info (forces data() to call InfoFactory::create)
    FileItemData itemData(testUrl, nullptr);

    // Trigger kItemCreateFileInfoRole which exercises the fix
    EXPECT_NO_FATAL_FAILURE(itemData.data(kItemCreateFileInfoRole));

    // The fix ensures updateAttributes() is called even on cache hit for remote files
    EXPECT_TRUE(updateAttributesCalled);
}

// Verify that for local files (non-remote) with cache hit, updateAttributes is NOT called
TEST_F(TestFileItemDataBug, LocalFileCacheHit_DoesNotCallUpdateAttributes)
{
    bool updateAttributesCalled = false;

    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl &, bool *, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [this](const QUrl &, bool *isCacheMiss, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       if (isCacheMiss)
                           *isCacheMiss = false;
                       return cachedInfo;
                   });

    // ProtocolUtils::isRemoteFile returns false (default for file:// scheme)
    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(SyncFileInfo, updateAttributes), [&updateAttributesCalled](SyncFileInfo *, const QList<FileInfo::FileInfoAttributeID> &) {
        __DBG_STUB_INVOKE__
        updateAttributesCalled = true;
    });

    FileItemData itemData(testUrl, nullptr);

    EXPECT_NO_FATAL_FAILURE(itemData.data(kItemCreateFileInfoRole));

    // For local files with cache hit, updateAttributes should NOT be called
    EXPECT_FALSE(updateAttributesCalled);
}
