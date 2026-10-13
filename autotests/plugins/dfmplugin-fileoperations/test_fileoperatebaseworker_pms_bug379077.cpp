// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QDir>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/file/local/localdiriterator.h>

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"
#include "fileoperations/copyfiles/docopyfilesworker.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestFileOperateBaseWorkerBug379077 : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        worker = new DoCopyFilesWorker();
        ASSERT_TRUE(worker);
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

// PMS:379077 SFTP 路径同步 FileInfo 缓存修复
// 修复 (07a9e10a): setExpectedSizeForTarget 中将 kCreateFileInfoSyncAndCache
// 改为 kCreateFileInfoAuto，避免远程路径缓存中毒
TEST_F(TestFileOperateBaseWorkerBug379077, SetExpectedSizeForTarget_UsesAutoCreateType)
{
    Global::CreateFileInfoType capturedType = Global::CreateFileInfoType::kCreateFileInfoSyncAndCache;

    // Stub InfoFactory::create to capture the CreateFileInfoType parameter
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [&capturedType](const QUrl &url, Global::CreateFileInfoType type, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       capturedType = type;
                       // Return a valid FileInfo for the local test file
                       return InfoFactory::create<SyncFileInfo>(url);
                   });

    QString filePath = tempDir->path() + "/target.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("test");
    f.close();
    QUrl targetUrl = QUrl::fromLocalFile(filePath);

    EXPECT_NO_FATAL_FAILURE(worker->setExpectedSizeForTarget(targetUrl, 1024));

    // The fix changed from kCreateFileInfoSyncAndCache to kCreateFileInfoAuto
    EXPECT_EQ(capturedType, Global::CreateFileInfoType::kCreateFileInfoAuto);
}

// Verify the method completes without crash for a valid URL
TEST_F(TestFileOperateBaseWorkerBug379077, SetExpectedSizeForTarget_DoesNotCrash)
{
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl &, Global::CreateFileInfoType, QString *)>(
                       &InfoFactory::create<FileInfo>),
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return InfoFactory::create<SyncFileInfo>(url);
                   });

    QString filePath = tempDir->path() + "/target2.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("test");
    f.close();
    QUrl targetUrl = QUrl::fromLocalFile(filePath);

    EXPECT_NO_FATAL_FAILURE(worker->setExpectedSizeForTarget(targetUrl, 2048));
}
