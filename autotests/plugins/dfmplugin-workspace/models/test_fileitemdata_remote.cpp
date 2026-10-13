// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "models/fileitemdata.h"
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QVariant>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class FileItemDataRemoteTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        mockInfo = FileInfoPointer(new SyncFileInfo(QUrl("file:///tmp/ut_remote_stub.txt")));
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    FileInfoPointer mockInfo;
};

TEST_F(FileItemDataRemoteTest, RemoteFile_UpdateAttributesCalledOnCreateFileInfo)
{
    bool updateCalled = false;
    stub.set_lamda(VADDR(SyncFileInfo, updateAttributes),
                   [&updateCalled](SyncFileInfo *, const QList<FileInfo::FileInfoAttributeID> &) {
                       __DBG_STUB_INVOKE__
                       updateCalled = true;
                   });

    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [this](const QUrl &, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockInfo;
                   });

    QUrl remoteUrl("sftp://192.168.1.100/home/user/file.txt");
    FileItemData itemData(remoteUrl, nullptr);

    EXPECT_NO_THROW(itemData.data(kItemCreateFileInfoRole));
    EXPECT_TRUE(updateCalled);
}

TEST_F(FileItemDataRemoteTest, LocalFile_UpdateAttributesAlsoCalled)
{
    bool updateCalled = false;
    stub.set_lamda(VADDR(SyncFileInfo, updateAttributes),
                   [&updateCalled](SyncFileInfo *, const QList<FileInfo::FileInfoAttributeID> &) {
                       __DBG_STUB_INVOKE__
                       updateCalled = true;
                   });

    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [this](const QUrl &, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockInfo;
                   });

    QUrl localUrl("file:///home/user/local.txt");
    FileItemData itemData(localUrl, nullptr);

    EXPECT_NO_THROW(itemData.data(kItemCreateFileInfoRole));
    EXPECT_TRUE(updateCalled);
}

TEST_F(FileItemDataRemoteTest, CachedInfo_RemoteFile_UpdateAttributesCalledOnSecondAccess)
{
    bool updateCalled = false;
    stub.set_lamda(VADDR(SyncFileInfo, updateAttributes),
                   [&updateCalled](SyncFileInfo *, const QList<FileInfo::FileInfoAttributeID> &) {
                       __DBG_STUB_INVOKE__
                       updateCalled = true;
                   });

    mockInfo->setExtendedAttributes(ExtInfoType::kFileNeedUpdate, true);

    FileItemData itemData(QUrl("sftp://host/path"), mockInfo);

    EXPECT_NO_THROW(itemData.data(kItemCreateFileInfoRole));
    EXPECT_TRUE(updateCalled);
}
