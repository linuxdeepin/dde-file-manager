// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfmio_utils.h>

#include "fileoperations/trashfiles/docopyfromtrashfilesworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoCopyFromTrashFilesWorker : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        worker = new DoCopyFromTrashFilesWorker();
        ASSERT_TRUE(worker);

        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);
        worker->targetUrl = QUrl::fromLocalFile(tempDirPath);
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

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    DoCopyFromTrashFilesWorker *worker;
};

TEST_F(TestDoCopyFromTrashFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kCopyType);
}

TEST_F(TestDoCopyFromTrashFilesWorker, Destructor_CallsStop)
{
    bool stopCalled = false;
    stub.set_lamda(VADDR(AbstractWorker, stop), [&stopCalled]() {
        __DBG_STUB_INVOKE__
        stopCalled = true;
    });

    delete worker;
    worker = nullptr;
    EXPECT_TRUE(stopCalled);
}

TEST_F(TestDoCopyFromTrashFilesWorker, StatisticsFilesSize_EmptySourceUrls)
{
    worker->sourceUrls.clear();
    bool result = worker->statisticsFilesSize();
    EXPECT_FALSE(result);
}

TEST_F(TestDoCopyFromTrashFilesWorker, StatisticsFilesSize_Success)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/test/file1.txt"));
    worker->sourceUrls.append(QUrl::fromLocalFile("/test/file2.txt"));

    bool result = worker->statisticsFilesSize();
    EXPECT_TRUE(result);
    EXPECT_EQ(worker->sourceFilesCount, 2);
}

TEST_F(TestDoCopyFromTrashFilesWorker, InitArgs_ClearsCompleteTargetFiles)
{
    worker->completeTargetFiles.append(QUrl());

    stub.set_lamda(VADDR(AbstractWorker, initArgs), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->initArgs();
    EXPECT_TRUE(result);
    EXPECT_TRUE(worker->completeTargetFiles.isEmpty());
}

TEST_F(TestDoCopyFromTrashFilesWorker, DoWork_Success)
{
    stub.set_lamda(VADDR(AbstractWorker, doWork), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCopyFromTrashFilesWorker::doOperate,
                   [](DoCopyFromTrashFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractWorker, endWork),
                   []() {
                       __DBG_STUB_INVOKE__
                   });

    bool result = worker->doWork();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCopyFromTrashFilesWorker, CreateParentDir_ParentExists)
{
    QString testPath = tempDirPath + "/test_file.txt";
    QUrl testUrl = QUrl::fromLocalFile(testPath);

    auto trashInfo = DFileInfoPointer(new DFileInfo(testUrl));
    auto restoreInfo = DFileInfoPointer(new DFileInfo(testUrl));

    bool result = false;
    auto parentInfo = worker->createParentDir(trashInfo, restoreInfo, &result);
    EXPECT_TRUE(parentInfo);
}

TEST_F(TestDoCopyFromTrashFilesWorker, CreateParentDir_CreateNew)
{
    QString newDir = tempDirPath + "/new_dir";
    QString testPath = newDir + "/test_file.txt";
    QUrl testUrl = QUrl::fromLocalFile(testPath);

    auto trashInfo = DFileInfoPointer(new DFileInfo(testUrl));
    auto restoreInfo = DFileInfoPointer(new DFileInfo(testUrl));

    bool result = false;
    auto parentInfo = worker->createParentDir(trashInfo, restoreInfo, &result);
    EXPECT_TRUE(parentInfo || !result);
}

TEST_F(TestDoCopyFromTrashFilesWorker, DoOperate_EmptySourceUrls)
{
    worker->sourceUrls.clear();
    bool result = worker->doOperate();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCopyFromTrashFilesWorker, DoOperate_WithValidFile)
{
    QString testFile = tempDirPath + "/trash_test.txt";
    QFile file(testFile);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    worker->sourceUrls.append(QUrl::fromLocalFile(testFile));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(QUrl()));
                   });

    stub.set_lamda(&FileOperateBaseWorker::copyFileFromTrash,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &, DFile::CopyFlag) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doOperate();
    EXPECT_TRUE(result);
}
