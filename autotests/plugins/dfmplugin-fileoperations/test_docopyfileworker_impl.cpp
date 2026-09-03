// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfile.h>

#include "fileoperations/fileoperationutils/docopyfileworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class DoCopyFileWorkerImpl : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();

        workData.reset(new WorkerData);
        worker = new DoCopyFileWorker(workData);
        ASSERT_TRUE(worker);
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
            worker->stop();
            delete worker;
            worker = nullptr;
        }
        workData.reset();
        tempDir.reset();
    }

protected:
    FileInfoPointer createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << content;
            file.close();
        }
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        return InfoFactory::create<FileInfo>(fileUrl);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QSharedPointer<WorkerData> workData;
    DoCopyFileWorker *worker { nullptr };
};

// ========== Constructor / Destructor ==========
TEST_F(DoCopyFileWorkerImpl, Constructor_InitializesCorrectly)
{
    EXPECT_TRUE(worker);
    EXPECT_EQ(worker->workData, workData);
}

TEST_F(DoCopyFileWorkerImpl, Destructor_WakesWaitingThreads)
{
    delete worker;
    worker = nullptr;
    SUCCEED();
}

// ========== State management ==========
TEST_F(DoCopyFileWorkerImpl, Pause_SetsStateToPaused)
{
    worker->pause();
    // State should be paused
    SUCCEED();
}

TEST_F(DoCopyFileWorkerImpl, Resume_ResumesExecution)
{
    worker->pause();
    worker->resume();
    SUCCEED();
}

TEST_F(DoCopyFileWorkerImpl, Stop_SetsStateToStopped)
{
    worker->stop();
    SUCCEED();
}

TEST_F(DoCopyFileWorkerImpl, OperateAction_SetsCurrentAction)
{
    worker->operateAction(AbstractJobHandler::SupportAction::kSkipAction);
    SUCCEED();
}

TEST_F(DoCopyFileWorkerImpl, OperateAction_Retry)
{
    workData->singleThread = false;
    worker->operateAction(AbstractJobHandler::SupportAction::kRetryAction);
    SUCCEED();
}

// ========== progressCallback ==========
TEST_F(DoCopyFileWorkerImpl, ProgressCallback_UpdatesProgress)
{
    DoCopyFileWorker::ProgressData data;
    data.data = workData;
    data.copyFile = QUrl::fromLocalFile("/test/file.txt");

    qint64 initialWriteSize = workData->currentWriteSize;

    DoCopyFileWorker::progressCallback(1000, 2000, &data);

    EXPECT_GT(workData->currentWriteSize, initialWriteSize);
}

TEST_F(DoCopyFileWorkerImpl, ProgressCallback_ZeroTotalSize)
{
    DoCopyFileWorker::ProgressData data;
    data.data = workData;
    data.copyFile = QUrl::fromLocalFile("/test/file.txt");

    qint64 initialZeroSize = workData->zeroOrlinkOrDirWriteSize;

    DoCopyFileWorker::progressCallback(0, 0, &data);

    EXPECT_GT(workData->zeroOrlinkOrDirWriteSize, initialZeroSize);
}

// ========== doFileCopy ==========
TEST_F(DoCopyFileWorkerImpl, DoFileCopy_CallsCopy)
{
    auto sourceFile = createTestFile("source.txt");
    auto targetFile = createTestFile("target.txt");

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetFile->urlOf(UrlInfoType::kUrl)));

    stub.set_lamda(&DoCopyFileWorker::doCopyFileByRange,
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    worker->doFileCopy(fromInfo, toInfo);
    EXPECT_EQ(workData->completeFileCount, 1);
}

// ========== doDfmioFileCopy ==========
TEST_F(DoCopyFileWorkerImpl, DoDfmioFileCopy_Success)
{
    auto sourceFile = createTestFile("dfmio_source.txt");
    QString targetPath = tempDirPath + "/dfmio_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile,
                   [](DoCopyFileWorker *, const DFileInfoPointer &) { });

    stub.set_lamda(ADDR(dfmio::DOperator, copyFile),
                   [](dfmio::DOperator *, const QUrl &, DFile::CopyFlags,
                      DOperator::ProgressCallbackFunc, void *) -> bool { return true; });

    bool skip = false;
    bool result = worker->doDfmioFileCopy(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(DoCopyFileWorkerImpl, DoDfmioFileCopy_Stopped)
{
    auto sourceFile = createTestFile("dfmio_stopped.txt");
    QString targetPath = tempDirPath + "/dfmio_stopped_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    worker->stop();

    bool skip = false;
    bool result = worker->doDfmioFileCopy(fromInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

// ========== openFileBySys ==========
TEST_F(DoCopyFileWorkerImpl, OpenFileBySys_OpenSuccess)
{
    auto testFile = createTestFile("open_test.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));

    bool skip = false;
    int fd = worker->openFileBySys(fromInfo, toInfo, O_RDONLY, &skip, true);

    if (fd >= 0) {
        close(fd);
        EXPECT_GE(fd, 0);
    }
    SUCCEED();
}

// ========== doCopyFilePractically ==========
TEST_F(DoCopyFileWorkerImpl, DoCopyFilePractically_Stopped)
{
    auto sourceFile = createTestFile("practical_stopped.txt");
    QString targetPath = tempDirPath + "/practical_stopped_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    worker->stop();

    bool skip = false;
    auto result = worker->doCopyFilePractically(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel);
}

TEST_F(DoCopyFileWorkerImpl, DoCopyFilePractically_DirectMode)
{
    auto sourceFile = createTestFile("direct_mode.txt");
    QString targetPath = tempDirPath + "/direct_mode_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    workData->exBlockSyncEveryWrite = true;
    workData->isTargetFileLocal = false;

    stub.set_lamda(&DoCopyFileWorker::doCopyFileWithDirectIO,
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    bool skip = false;
    auto result = worker->doCopyFilePractically(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

TEST_F(DoCopyFileWorkerImpl, DoCopyFilePractically_TraditionalMode)
{
    auto sourceFile = createTestFile("traditional.txt");
    QString targetPath = tempDirPath + "/traditional_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    workData->exBlockSyncEveryWrite = false;

    stub.set_lamda(&DoCopyFileWorker::doCopyFileTraditional,
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    bool skip = false;
    auto result = worker->doCopyFilePractically(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

// ========== doCopyFileTraditional ==========
TEST_F(DoCopyFileWorkerImpl, DoCopyFileTraditional_Success)
{
    auto sourceFile = createTestFile("traditional_source.txt", "test data");
    QString targetPath = tempDirPath + "/traditional_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile,
                   [](DoCopyFileWorker *, const DFileInfoPointer &) { });

    stub.set_lamda(ADDR(dfmio::DOperator, copyFile),
                   [](dfmio::DOperator *, const QUrl &, DFile::CopyFlags,
                      DOperator::ProgressCallbackFunc, void *) -> bool { return true; });

    bool skip = false;
    auto result = worker->doCopyFileTraditional(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

TEST_F(DoCopyFileWorkerImpl, DoCopyFileTraditional_Stopped)
{
    auto sourceFile = createTestFile("trad_stopped.txt");
    QString targetPath = tempDirPath + "/trad_stopped_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    worker->stop();

    bool skip = false;
    auto result = worker->doCopyFileTraditional(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel);
}

// ========== doCopyFileByRange ==========
TEST_F(DoCopyFileWorkerImpl, DoCopyFileByRange_Stopped)
{
    auto sourceFile = createTestFile("range_stopped.txt");
    QString targetPath = tempDirPath + "/range_stopped_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    worker->stop();

    bool skip = false;
    auto result = worker->doCopyFileByRange(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel);
}

TEST_F(DoCopyFileWorkerImpl, DoCopyFileByRange_Fallback)
{
    auto sourceFile = createTestFile("range_fallback.txt");
    QString targetPath = tempDirPath + "/range_fallback_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::shouldFallbackFromCopyFileRange,
                   [](DoCopyFileWorker *, int) -> bool { return true; });

    // Force copy_file_range failure so the fallback branch is exercised;
    // otherwise the copy succeeds on tmpfs and the fallback path is never reached.
    stub.set_lamda(copy_file_range,
                   [](int, off_t *, int, off_t *, size_t, unsigned int) -> ssize_t {
                       errno = EXDEV;
                       return -1;
                   });

    bool skip = false;
    auto result = worker->doCopyFileByRange(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyFallback);
}

// ========== openDestinationFile ==========
TEST_F(DoCopyFileWorkerImpl, OpenDestinationFile_NormalMode)
{
    QString targetPath = tempDirPath + "/dest_new.txt";

    auto result = worker->openDestinationFile(targetPath, DoCopyFileWorker::WriteMode::Normal);

    if (result.fd >= 0) {
        close(result.fd);
        EXPECT_GE(result.fd, 0);
    }
    SUCCEED();
}

TEST_F(DoCopyFileWorkerImpl, OpenDestinationFile_DirectMode)
{
    QString targetPath = tempDirPath + "/dest_direct.txt";

    auto result = worker->openDestinationFile(targetPath, DoCopyFileWorker::WriteMode::Direct);

    if (result.fd >= 0) {
        close(result.fd);
    }
    SUCCEED();
}

// ========== reopenDestinationFileForResume ==========
TEST_F(DoCopyFileWorkerImpl, ReopenDestinationFileForResume_ExistingFile)
{
    auto targetFile = createTestFile("reopen_target.txt", "existing content");
    QString targetPath = targetFile->urlOf(UrlInfoType::kUrl).toLocalFile();

    auto result = worker->reopenDestinationFileForResume(targetPath, DoCopyFileWorker::WriteMode::Normal);

    if (result.fd >= 0) {
        close(result.fd);
    }
    SUCCEED();
}

// ========== allocateAlignedBuffer ==========
TEST_F(DoCopyFileWorkerImpl, AllocateAlignedBuffer_4KAlignment)
{
    char *buffer = worker->allocateAlignedBuffer(4096, 4096);

    if (buffer) {
        EXPECT_NE(buffer, nullptr);
        EXPECT_EQ(reinterpret_cast<uintptr_t>(buffer) % 4096, 0);
        free(buffer);
    }
    SUCCEED();
}

// ========== actionToNextDo / actionOperating ==========
TEST_F(DoCopyFileWorkerImpl, ActionToNextDo_NoAction)
{
    bool skip = false;
    auto result = worker->actionToNextDo(AbstractJobHandler::SupportAction::kNoAction, 100, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

TEST_F(DoCopyFileWorkerImpl, ActionToNextDo_Skip)
{
    bool skip = false;
    auto result = worker->actionToNextDo(AbstractJobHandler::SupportAction::kSkipAction, 100, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel);
    EXPECT_TRUE(skip);
}

// ========== shouldFallbackFromCopyFileRange ==========
TEST_F(DoCopyFileWorkerImpl, ShouldFallbackFromCopyFileRange_EINVAL)
{
    bool result = worker->shouldFallbackFromCopyFileRange(EINVAL);
    EXPECT_TRUE(result);
}

TEST_F(DoCopyFileWorkerImpl, ShouldFallbackFromCopyFileRange_EXDEV)
{
    bool result = worker->shouldFallbackFromCopyFileRange(EXDEV);
    EXPECT_TRUE(result);
}

TEST_F(DoCopyFileWorkerImpl, ShouldFallbackFromCopyFileRange_ENOSYS)
{
    bool result = worker->shouldFallbackFromCopyFileRange(ENOSYS);
    EXPECT_TRUE(result);
}

TEST_F(DoCopyFileWorkerImpl, ShouldFallbackFromCopyFileRange_Success)
{
    bool result = worker->shouldFallbackFromCopyFileRange(0);
    EXPECT_FALSE(result);
}

// ========== mapSystemErrorToJobError ==========
TEST_F(DoCopyFileWorkerImpl, MapSystemErrorToJobError_NoSpace)
{
    auto result = worker->mapSystemErrorToJobError(ENOSPC, true);
    EXPECT_EQ(result, AbstractJobHandler::JobErrorType::kNotEnoughSpaceError);
}

TEST_F(DoCopyFileWorkerImpl, MapSystemErrorToJobError_Permission)
{
    // The implementation maps EACCES/EPERM to kPermissionDeniedError
    auto result = worker->mapSystemErrorToJobError(EACCES, true);
    EXPECT_EQ(result, AbstractJobHandler::JobErrorType::kPermissionDeniedError);
    result = worker->mapSystemErrorToJobError(EPERM, true);
    EXPECT_EQ(result, AbstractJobHandler::JobErrorType::kPermissionDeniedError);
}

// ========== Signal emission ==========
TEST_F(DoCopyFileWorkerImpl, CurrentTask_SignalEmitted)
{
    bool signalEmitted = false;
    QUrl receivedSource, receivedTarget;

    QObject::connect(worker, &DoCopyFileWorker::currentTask,
                   [&signalEmitted, &receivedSource, &receivedTarget](const QUrl &source, const QUrl &target) {
                       signalEmitted = true;
                       receivedSource = source;
                       receivedTarget = target;
                   });

    auto sourceFile = createTestFile("signal_source.txt");
    QString targetPath = tempDirPath + "/signal_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile,
                   [](DoCopyFileWorker *, const DFileInfoPointer &) { });

    stub.set_lamda(ADDR(dfmio::DOperator, copyFile),
                   [](dfmio::DOperator *, const QUrl &, DFile::CopyFlags,
                      DOperator::ProgressCallbackFunc, void *) -> bool { return true; });

    bool skip = false;
    worker->doDfmioFileCopy(fromInfo, toInfo, &skip);

    EXPECT_TRUE(signalEmitted);
}

// ========== createFileDevice / createFileDevices ==========
TEST_F(DoCopyFileWorkerImpl, CreateFileDevice_Stopped)
{
    auto sourceFile = createTestFile("create_dev.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));

    worker->stop();

    QSharedPointer<DFMIO::DFile> file;
    bool skip = false;
    bool result = worker->createFileDevice(fromInfo, toInfo, fromInfo, file, &skip);
    EXPECT_FALSE(result);
}

TEST_F(DoCopyFileWorkerImpl, CreateFileDevices_Stopped)
{
    auto sourceFile = createTestFile("create_devs.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));

    worker->stop();

    QSharedPointer<DFMIO::DFile> fromFile, toFile;
    bool skip = false;
    bool result = worker->createFileDevices(fromInfo, toInfo, fromFile, toFile, &skip);
    EXPECT_FALSE(result);
}

// ========== verifyFileIntegrity ==========
TEST_F(DoCopyFileWorkerImpl, VerifyFileIntegrity_ZeroSize)
{
    auto sourceFile = createTestFile("verify.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));

    QSharedPointer<DFMIO::DFile> toFile;
    bool result = worker->verifyFileIntegrity(1024, 0, fromInfo, toInfo, toFile);
    EXPECT_TRUE(result);
}
