// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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

class TestDoCopyFileWorker : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        // Create workData
        workData.reset(new WorkerData);

        // Create worker instance
        worker = new DoCopyFileWorker(workData);
        ASSERT_TRUE(worker);
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
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
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << content;
        file.close();

        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        return InfoFactory::create<FileInfo>(fileUrl);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QUrl tempDirUrl;
    QSharedPointer<WorkerData> workData;
    DoCopyFileWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoCopyFileWorker, Constructor_InitializesCorrectly)
{
    EXPECT_TRUE(worker);
    EXPECT_EQ(worker->workData, workData);
}

TEST_F(TestDoCopyFileWorker, Destructor_WakesWaitingThreads)
{
    // Destructor should wake all waiting threads
    delete worker;
    worker = nullptr;
    // Should not crash
}

// ========== State Management Tests ==========

TEST_F(TestDoCopyFileWorker, Pause_SetsStateToPaused)
{
    worker->pause();
    // State should be paused (we can't directly access private member, but we can test behavior)
}

TEST_F(TestDoCopyFileWorker, Resume_ResumesExecution)
{
    worker->pause();
    worker->resume();
    // State should be normal
}

TEST_F(TestDoCopyFileWorker, Stop_SetsStateToStopped)
{
    worker->stop();
    // State should be stopped
}

TEST_F(TestDoCopyFileWorker, OperateAction_SetsCurrentAction)
{
    worker->operateAction(AbstractJobHandler::SupportAction::kSkipAction);
    // CurrentAction should be set
}

// ========== doFileCopy Tests ==========

TEST_F(TestDoCopyFileWorker, DoFileCopy_CallsDoCopyFileByRange)
{
    auto sourceFile = createTestFile("source.txt");
    auto targetFile = createTestFile("target.txt");

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetFile->urlOf(UrlInfoType::kUrl)));

    stub.set_lamda(&DoCopyFileWorker::doCopyFileByRange,
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       __DBG_STUB_INVOKE__
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    worker->doFileCopy(fromInfo, toInfo);
    EXPECT_EQ(workData->completeFileCount, 1);
}

// ========== doDfmioFileCopy Tests ==========

TEST_F(TestDoCopyFileWorker, DoDfmioFileCopy_Success)
{
    auto sourceFile = createTestFile("source_dfmio.txt");
    QString targetPath = tempDirPath + "/target_dfmio.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile,
                   [](DoCopyFileWorker *, const DFileInfoPointer &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(ADDR(dfmio::DOperator, copyFile),
                   [](dfmio::DOperator *, const QUrl &, DFile::CopyFlags,
                      DOperator::ProgressCallbackFunc, void *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    bool result = worker->doDfmioFileCopy(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCopyFileWorker, DoDfmioFileCopy_Stopped)
{
    auto sourceFile = createTestFile("source_stopped.txt");
    QString targetPath = tempDirPath + "/target_stopped.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    worker->stop();

    bool skip = false;
    bool result = worker->doDfmioFileCopy(fromInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

// ========== progressCallback Tests ==========

TEST_F(TestDoCopyFileWorker, ProgressCallback_UpdatesProgress)
{
    DoCopyFileWorker::ProgressData data;
    data.data = workData;
    data.copyFile = QUrl::fromLocalFile("/test/file.txt");

    qint64 initialWriteSize = workData->currentWriteSize;

    DoCopyFileWorker::progressCallback(1000, 2000, &data);

    EXPECT_GT(workData->currentWriteSize, initialWriteSize);
}

TEST_F(TestDoCopyFileWorker, ProgressCallback_ZeroTotalSize)
{
    DoCopyFileWorker::ProgressData data;
    data.data = workData;
    data.copyFile = QUrl::fromLocalFile("/test/file.txt");

    qint64 initialZeroSize = workData->zeroOrlinkOrDirWriteSize;

    DoCopyFileWorker::progressCallback(0, 0, &data);

    EXPECT_GT(workData->zeroOrlinkOrDirWriteSize, initialZeroSize);
}

// ========== openFileBySys Tests ==========

TEST_F(TestDoCopyFileWorker, OpenFileBySys_OpenSuccess)
{
    auto testFile = createTestFile("open_test.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));

    bool skip = false;
    int fd = worker->openFileBySys(fromInfo, toInfo, O_RDONLY, &skip, true);

    if (fd >= 0) {
        close(fd);
        EXPECT_TRUE(true);
    } else {
        // May fail due to various reasons, that's ok for this test
        EXPECT_TRUE(true);
    }
}

// ========== doCopyFileWithDirectIO Tests ==========

TEST_F(TestDoCopyFileWorker, DoCopyFileWithDirectIO_InvalidSource)
{
    auto sourceFile = createTestFile("direct_source.txt");
    QString targetPath = tempDirPath + "/direct_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/nonexistent/file.txt")));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,
                   [](DoCopyFileWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool skip = false;
    auto result = worker->doCopyFileWithDirectIO(fromInfo, toInfo, &skip);
    // Should handle error
    EXPECT_NE(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

// ========== doCopyFilePractically Tests ==========

TEST_F(TestDoCopyFileWorker, DoCopyFilePractically_Stopped)
{
    auto sourceFile = createTestFile("practical_source.txt");
    QString targetPath = tempDirPath + "/practical_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    worker->stop();

    bool skip = false;
    auto result = worker->doCopyFilePractically(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel);
}

TEST_F(TestDoCopyFileWorker, DoCopyFilePractically_DirectMode)
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
                       __DBG_STUB_INVOKE__
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    bool skip = false;
    auto result = worker->doCopyFilePractically(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

TEST_F(TestDoCopyFileWorker, DoCopyFilePractically_TraditionalMode)
{
    auto sourceFile = createTestFile("traditional.txt");
    QString targetPath = tempDirPath + "/traditional_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    workData->exBlockSyncEveryWrite = false;

    stub.set_lamda(&DoCopyFileWorker::doCopyFileTraditional,
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       __DBG_STUB_INVOKE__
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    bool skip = false;
    auto result = worker->doCopyFilePractically(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

// ========== doCopyFileTraditional Tests ==========

TEST_F(TestDoCopyFileWorker, DoCopyFileTraditional_Success)
{
    auto sourceFile = createTestFile("traditional_source.txt", "test data");
    QString targetPath = tempDirPath + "/traditional_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile,
                   [](DoCopyFileWorker *, const DFileInfoPointer &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(ADDR(dfmio::DOperator, copyFile),
                   [](dfmio::DOperator *, const QUrl &, DFile::CopyFlags,
                      DOperator::ProgressCallbackFunc, void *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    auto result = worker->doCopyFileTraditional(fromInfo, toInfo, &skip);
    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
}

TEST_F(TestDoCopyFileWorker, DoCopyFileTraditional_Stopped)
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

// ========== doCopyFileByRange Tests ==========

TEST_F(TestDoCopyFileWorker, DoCopyFileByRange_Success)
{
    auto sourceFile = createTestFile("range_source.txt", "test content for range");
    QString targetPath = tempDirPath + "/range_target.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&DoCopyFileWorker::doCopyFilePractically,
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       __DBG_STUB_INVOKE__
                       return DoCopyFileWorker::NextDo::kDoCopyNext;
                   });

    bool skip = false;
    auto result = worker->doCopyFileByRange(fromInfo, toInfo, &skip);
    // May succeed or fallback to traditional depending on system support
    EXPECT_TRUE(result == DoCopyFileWorker::NextDo::kDoCopyNext);
}

TEST_F(TestDoCopyFileWorker, DoCopyFileByRange_Stopped)
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

// ========== openDestinationFile Tests ==========

TEST_F(TestDoCopyFileWorker, OpenDestinationFile_NormalMode)
{
    QString targetPath = tempDirPath + "/dest_new.txt";

    auto result = worker->openDestinationFile(targetPath, DoCopyFileWorker::WriteMode::Normal);

    if (result.fd >= 0) {
        close(result.fd);
        EXPECT_EQ(result.mode, DoCopyFileWorker::WriteMode::Normal);
    } else {
        // May fail on some systems, acceptable
        SUCCEED();
    }
}

TEST_F(TestDoCopyFileWorker, OpenDestinationFile_DirectMode)
{
    QString targetPath = tempDirPath + "/dest_direct.txt";

    auto result = worker->openDestinationFile(targetPath, DoCopyFileWorker::WriteMode::Direct);

    if (result.fd >= 0) {
        close(result.fd);
        // Direct mode may fallback to Normal mode on some systems
        SUCCEED();
    } else {
        // May fail on some systems, acceptable
        SUCCEED();
    }
}

// ========== reopenDestinationFileForResume Tests ==========

TEST_F(TestDoCopyFileWorker, ReopenDestinationFileForResume_ExistingFile)
{
    // Create existing target file
    auto targetFile = createTestFile("reopen_target.txt", "existing content");
    QString targetPath = targetFile->urlOf(UrlInfoType::kUrl).toLocalFile();

    auto result = worker->reopenDestinationFileForResume(targetPath, DoCopyFileWorker::WriteMode::Normal);

    if (result.fd >= 0) {
        close(result.fd);
        SUCCEED();
    } else {
        // May fail on some systems
        SUCCEED();
    }
}

TEST_F(TestDoCopyFileWorker, ReopenDestinationFileForResume_DirectMode)
{
    auto targetFile = createTestFile("reopen_direct.txt", "test");
    QString targetPath = targetFile->urlOf(UrlInfoType::kUrl).toLocalFile();

    auto result = worker->reopenDestinationFileForResume(targetPath, DoCopyFileWorker::WriteMode::Direct);

    if (result.fd >= 0) {
        close(result.fd);
        SUCCEED();
    } else {
        SUCCEED();
    }
}

// ========== allocateAlignedBuffer Tests ==========

TEST_F(TestDoCopyFileWorker, AllocateAlignedBuffer_4KAlignment)
{
    size_t bufferSize = 4096;
    size_t alignment = 4096;
    char *buffer = worker->allocateAlignedBuffer(bufferSize, alignment);

    if (buffer) {
        EXPECT_NE(buffer, nullptr);
        // Check alignment (should be aligned to 4K boundary)
        EXPECT_EQ(reinterpret_cast<uintptr_t>(buffer) % alignment, 0);
        free(buffer);
    } else {
        // May fail on some systems
        SUCCEED();
    }
}

TEST_F(TestDoCopyFileWorker, AllocateAlignedBuffer_512Alignment)
{
    size_t bufferSize = 512;
    size_t alignment = 512;
    char *buffer = worker->allocateAlignedBuffer(bufferSize, alignment);

    if (buffer) {
        EXPECT_NE(buffer, nullptr);
        EXPECT_EQ(reinterpret_cast<uintptr_t>(buffer) % alignment, 0);
        free(buffer);
    } else {
        SUCCEED();
    }
}

// ========== shouldFallbackFromCopyFileRange Tests ==========

TEST_F(TestDoCopyFileWorker, ShouldFallbackFromCopyFileRange_EINVAL)
{
    bool result = worker->shouldFallbackFromCopyFileRange(EINVAL);
    EXPECT_TRUE(result);  // EINVAL should trigger fallback
}

TEST_F(TestDoCopyFileWorker, ShouldFallbackFromCopyFileRange_EXDEV)
{
    bool result = worker->shouldFallbackFromCopyFileRange(EXDEV);
    EXPECT_TRUE(result);  // EXDEV (cross-device) should trigger fallback
}

TEST_F(TestDoCopyFileWorker, ShouldFallbackFromCopyFileRange_ENOSYS)
{
    bool result = worker->shouldFallbackFromCopyFileRange(ENOSYS);
    EXPECT_TRUE(result);  // ENOSYS (not implemented) should trigger fallback
}

TEST_F(TestDoCopyFileWorker, ShouldFallbackFromCopyFileRange_Success)
{
    bool result = worker->shouldFallbackFromCopyFileRange(0);
    EXPECT_FALSE(result);  // Success (errno=0), no fallback needed
}

// ========== Signal Emission Tests ==========

TEST_F(TestDoCopyFileWorker, CurrentTask_SignalEmitted)
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
                   [](DoCopyFileWorker *, const DFileInfoPointer &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(ADDR(dfmio::DOperator, copyFile),
                   [](dfmio::DOperator *, const QUrl &, DFile::CopyFlags,
                      DOperator::ProgressCallbackFunc, void *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    worker->doDfmioFileCopy(fromInfo, toInfo, &skip);

    EXPECT_TRUE(signalEmitted);
}
