// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"
#include "fileoperations/copyfiles/docopyfilesworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class SetExpectedSizeTest : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        worker = new DoCopyFilesWorker();
        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);

        using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
        stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                       [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
            delete worker;
            worker = nullptr;
        }
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    DoCopyFilesWorker *worker { nullptr };
};

TEST_F(SetExpectedSizeTest, SetExpectedSizeForTarget_UsesAutoNotSyncAndCache)
{
    Global::CreateFileInfoType capturedType = Global::CreateFileInfoType::kCreateFileInfoSyncAndCache;

    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [&capturedType](const QUrl &, Global::CreateFileInfoType type, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       capturedType = type;
                       return FileInfoPointer(new SyncFileInfo(QUrl("file:///tmp/ut_expected_size.txt")));
                   });

    QUrl targetUrl = QUrl::fromLocalFile(tempDir->path() + "/target.txt");
    worker->setExpectedSizeForTarget(targetUrl, 1024);

    EXPECT_EQ(capturedType, Global::CreateFileInfoType::kCreateFileInfoAuto);
    EXPECT_NE(capturedType, Global::CreateFileInfoType::kCreateFileInfoSyncAndCache);
}

TEST_F(SetExpectedSizeTest, SetExpectedSizeForTarget_SftpUrl_UsesAuto)
{
    Global::CreateFileInfoType capturedType = Global::CreateFileInfoType::kCreateFileInfoSyncAndCache;

    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [&capturedType](const QUrl &url, Global::CreateFileInfoType type, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       capturedType = type;
                       return FileInfoPointer(new SyncFileInfo(url));
                   });

    QUrl sftpUrl("sftp://192.168.1.100/home/user/target.txt");
    worker->setExpectedSizeForTarget(sftpUrl, 4096);

    EXPECT_EQ(capturedType, Global::CreateFileInfoType::kCreateFileInfoAuto);
}

TEST_F(SetExpectedSizeTest, SetExpectedSizeForTarget_SetsExpectedSizeAttribute)
{
    bool attributeSet = false;
    qint64 capturedSize = 0;

    auto mockInfo = FileInfoPointer(new SyncFileInfo(QUrl("file:///tmp/ut_expected_size_attr.txt")));

    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [mockInfo](const QUrl &, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, setExtendedAttributes),
                   [&attributeSet, &capturedSize](FileInfo *, const FileInfo::FileExtendedInfoType &key, const QVariant &value) {
                       __DBG_STUB_INVOKE__
                       if (key == FileInfo::FileExtendedInfoType::kExpectedSize) {
                           attributeSet = true;
                           capturedSize = value.toLongLong();
                       }
                   });

    QUrl targetUrl = QUrl::fromLocalFile(tempDir->path() + "/target_attr.txt");
    worker->setExpectedSizeForTarget(targetUrl, 8192);

    EXPECT_TRUE(attributeSet);
    EXPECT_EQ(capturedSize, 8192);
}
