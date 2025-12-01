// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>
#include <QSignalSpy>
#include <QWaitCondition>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>

#include "fileoperations/deletefiles/dodeletefilesworker.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoDeleteFilesWorker : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        // Create worker instance
        worker = new DoDeleteFilesWorker();
        ASSERT_TRUE(worker);

        // Initialize workData
        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);
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

    FileInfoPointer createTestDir(const QString &dirName)
    {
        QString dirPath = tempDirPath + QDir::separator() + dirName;
        QDir().mkpath(dirPath);

        QUrl dirUrl = QUrl::fromLocalFile(dirPath);
        return InfoFactory::create<FileInfo>(dirUrl);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QUrl tempDirUrl;
    DoDeleteFilesWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoDeleteFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kDeleteType);
}

TEST_F(TestDoDeleteFilesWorker, Destructor_CallsStop)
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

// ========== stop Tests ==========

TEST_F(TestDoDeleteFilesWorker, Stop_CallsBaseClass)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractWorker, stop), [&baseCalled](AbstractWorker *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    worker->stop();
    EXPECT_TRUE(baseCalled);
}

// ========== doHandleErrorAndWait Tests ==========

TEST_F(TestDoDeleteFilesWorker, DoHandleErrorAndWait_ErrorHandling)
{
    QUrl testUrl = QUrl::fromLocalFile("/test/file.txt");

    stub.set_lamda(VADDR(AbstractWorker, setStat),
                   [](AbstractWorker *, const AbstractJobHandler::JobState &state) {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(state, AbstractJobHandler::JobState::kPauseState);
                   });

    // Stub QWaitCondition::wait to prevent blocking
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool errorEmitted = false;
    QObject::connect(worker, &DoDeleteFilesWorker::errorNotify,
                     [&errorEmitted](const JobInfoPointer &) {
                         errorEmitted = true;
                     });

    // Set the expected action before calling doHandleErrorAndWait
    worker->currentAction = AbstractJobHandler::SupportAction::kSkipAction;

    auto action = worker->doHandleErrorAndWait(testUrl,
                                               AbstractJobHandler::JobErrorType::kDeleteFileError,
                                               "Test error");
    EXPECT_TRUE(errorEmitted);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kSkipAction);
}

// ========== deleteFileOnOtherDevice Tests ==========

TEST_F(TestDoDeleteFilesWorker, DeleteFileOnOtherDevice_Success)
{
    auto testFile = createTestFile("delete_me.txt");
    ASSERT_TRUE(testFile);

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->deleteFileOnOtherDevice(testFile->urlOf(UrlInfoType::kUrl));
    EXPECT_TRUE(result);
    EXPECT_EQ(worker->deleteFilesCount, 1);
}

TEST_F(TestDoDeleteFilesWorker, DeleteFileOnOtherDevice_Failure)
{
    auto testFile = createTestFile("fail_delete.txt");
    ASSERT_TRUE(testFile);

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&LocalFileHandler::deleteFile,
                   [](LocalFileHandler *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&LocalFileHandler::errorString,
                   [](LocalFileHandler *) -> QString {
                       __DBG_STUB_INVOKE__
                       return "Permission denied";
                   });

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait,
                   [](DoDeleteFilesWorker *, const QUrl &,
                      const AbstractJobHandler::JobErrorType &,
                      const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool result = worker->deleteFileOnOtherDevice(testFile->urlOf(UrlInfoType::kUrl));
    EXPECT_TRUE(result);   // Should return true when skipped
}

TEST_F(TestDoDeleteFilesWorker, DeleteFileOnOtherDevice_StateCheckFails)
{
    auto testFile = createTestFile("state_fail.txt");
    ASSERT_TRUE(testFile);

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->deleteFileOnOtherDevice(testFile->urlOf(UrlInfoType::kUrl));
    EXPECT_FALSE(result);
}

// ========== deleteDirOnOtherDevice Tests ==========

TEST_F(TestDoDeleteFilesWorker, DeleteDirOnOtherDevice_EmptyDirectory)
{
    auto testDir = createTestDir("empty_dir");
    ASSERT_TRUE(testDir);

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->deleteDirOnOtherDevice(testDir);
    EXPECT_TRUE(result);
}

TEST_F(TestDoDeleteFilesWorker, DeleteDirOnOtherDevice_WithFiles)
{
    auto testDir = createTestDir("dir_with_files");
    QString filePath = tempDirPath + "/dir_with_files/file.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->deleteDirOnOtherDevice(testDir);
    EXPECT_TRUE(result);
}

TEST_F(TestDoDeleteFilesWorker, DeleteDirOnOtherDevice_NoChildren)
{
    auto testDir = createTestDir("no_child_dir");
    ASSERT_TRUE(testDir);

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, countChildFile), [](FileInfo *) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,
                   [](DoDeleteFilesWorker *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->deleteDirOnOtherDevice(testDir);
    EXPECT_TRUE(result);
}

// ========== deleteFilesOnOtherDevice Tests ==========

TEST_F(TestDoDeleteFilesWorker, DeleteFilesOnOtherDevice_SingleFile)
{
    auto testFile = createTestFile("single_delete.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));
    worker->isConvert = true;

    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,
                   [](DoDeleteFilesWorker *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool signalEmitted = false;
    QObject::connect(worker, &DoDeleteFilesWorker::fileDeleted,
                     [&signalEmitted](const QUrl &) {
                         signalEmitted = true;
                     });

    bool result = worker->deleteFilesOnOtherDevice();
    EXPECT_TRUE(result);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestDoDeleteFilesWorker, DeleteFilesOnOtherDevice_Directory)
{
    auto testDir = createTestDir("delete_dir");
    worker->sourceUrls.append(testDir->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(&DoDeleteFilesWorker::deleteDirOnOtherDevice,
                   [](DoDeleteFilesWorker *, const FileInfoPointer &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->deleteFilesOnOtherDevice();
    EXPECT_TRUE(result);
}

TEST_F(TestDoDeleteFilesWorker, DeleteFilesOnOtherDevice_MultipleFiles)
{
    worker->sourceUrls.append(createTestFile("file1.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("file2.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("file3.txt")->urlOf(UrlInfoType::kUrl));

    int deleteCount = 0;
    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,
                   [&deleteCount](DoDeleteFilesWorker *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       deleteCount++;
                       return true;
                   });

    bool result = worker->deleteFilesOnOtherDevice();
    EXPECT_TRUE(result);
    EXPECT_EQ(deleteCount, 3);
}

// ========== deleteFilesOnCanNotRemoveDevice Tests ==========

TEST_F(TestDoDeleteFilesWorker, DeleteFilesOnCanNotRemoveDevice_SingleFile)
{
    auto testFile = createTestFile("local_delete.txt");
    worker->allFilesList.append(testFile->urlOf(UrlInfoType::kUrl));
    worker->isConvert = true;

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->deleteFilesOnCanNotRemoveDevice();
    EXPECT_TRUE(result);
}

TEST_F(TestDoDeleteFilesWorker, DeleteFilesOnCanNotRemoveDevice_MultipleFiles)
{
    worker->allFilesList.append(createTestFile("local1.txt")->urlOf(UrlInfoType::kUrl));
    worker->allFilesList.append(createTestFile("local2.txt")->urlOf(UrlInfoType::kUrl));
    worker->allFilesList.append(createTestFile("local3.txt")->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool signalEmitted = false;
    QObject::connect(worker, &DoDeleteFilesWorker::fileDeleted,
                     [&signalEmitted](const QUrl &) {
                         signalEmitted = true;
                     });

    bool result = worker->deleteFilesOnCanNotRemoveDevice();
    EXPECT_TRUE(result);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestDoDeleteFilesWorker, DeleteFilesOnCanNotRemoveDevice_WithSkip)
{
    auto testFile = createTestFile("skip_delete.txt");
    worker->allFilesList.append(testFile->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&LocalFileHandler::deleteFile,
                   [](LocalFileHandler *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait,
                   [](DoDeleteFilesWorker *, const QUrl &,
                      const AbstractJobHandler::JobErrorType &,
                      const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool result = worker->deleteFilesOnCanNotRemoveDevice();
    EXPECT_TRUE(result);
}

// ========== deleteAllFiles Tests ==========

TEST_F(TestDoDeleteFilesWorker, DeleteAllFiles_LocalDevice)
{
    worker->isSourceFileLocal = true;

    stub.set_lamda(&DoDeleteFilesWorker::deleteFilesOnCanNotRemoveDevice,
                   [](DoDeleteFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->deleteAllFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoDeleteFilesWorker, DeleteAllFiles_OtherDevice)
{
    worker->isSourceFileLocal = false;

    stub.set_lamda(&DoDeleteFilesWorker::deleteFilesOnOtherDevice,
                   [](DoDeleteFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->deleteAllFiles();
    EXPECT_TRUE(result);
}

// ========== doWork Tests ==========

TEST_F(TestDoDeleteFilesWorker, DoWork_Success)
{
    stub.set_lamda(VADDR(AbstractWorker, doWork), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoDeleteFilesWorker::deleteAllFiles,
                   [](DoDeleteFilesWorker *) -> bool {
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

TEST_F(TestDoDeleteFilesWorker, DoWork_BaseWorkFails)
{
    stub.set_lamda(VADDR(AbstractWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

// ========== onUpdateProgress Tests ==========

TEST_F(TestDoDeleteFilesWorker, OnUpdateProgress_EmitsSignal)
{
    worker->deleteFilesCount = 5;
    worker->sourceFilesTotalSize = 1000;

    bool progressEmitted = false;
    QObject::connect(worker, &DoDeleteFilesWorker::progressChangedNotify,
                     [&progressEmitted](const JobInfoPointer &) {
                         progressEmitted = true;
                     });

    worker->onUpdateProgress();
    EXPECT_TRUE(progressEmitted);
}
