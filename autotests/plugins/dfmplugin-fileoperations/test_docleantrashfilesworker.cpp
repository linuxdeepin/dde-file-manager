// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QSignalSpy>
#include <QWaitCondition>

#include "stubext.h"

#include "fileoperations/cleantrash/docleantrashfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoCleanTrashFilesWorker : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        worker = new DoCleanTrashFilesWorker();
        ASSERT_TRUE(worker);

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
        file.write(content.toUtf8());
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
    DoCleanTrashFilesWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kCleanTrashType);
}

TEST_F(TestDoCleanTrashFilesWorker, Destructor_CallsStop)
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

// ========== initArgs Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, InitArgs_Success)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, initArgs), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&FileUtils::trashRootUrl, []() {
        __DBG_STUB_INVOKE__
        return QUrl("trash:///");
    });

    bool result = worker->initArgs();

    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, InitArgs_BaseInitFails)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, initArgs), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->initArgs();

    EXPECT_FALSE(result);
}

// ========== statisticsFilesSize Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, StatisticsFilesSize_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, StatisticsFilesSize_WithSources)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);
}

// ========== onUpdateProgress Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, OnUpdateProgress_EmitsSignal)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCleanTrashType;
    worker->cleanTrashFilesCount = 5;
    worker->sourceFilesCount = 10;

    bool progressEmitted = false;
    QObject::connect(worker, &DoCleanTrashFilesWorker::progressChangedNotify,
                     [&progressEmitted](const JobInfoPointer &) {
                         progressEmitted = true;
                     });

    worker->onUpdateProgress();
    EXPECT_TRUE(progressEmitted);
}

// ========== doWork Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, DoWork_Success)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCleanTrashFilesWorker::cleanAllTrashFiles,
                   [](DoCleanTrashFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractWorker, endWork), []() {
        __DBG_STUB_INVOKE__
    });

    bool result = worker->doWork();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, DoWork_BaseWorkFails)
{
    stub.set_lamda(VADDR(FileOperateBaseWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

// ========== cleanAllTrashFiles Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, CleanAllTrashFiles_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->cleanAllTrashFiles();

    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, CleanAllTrashFiles_WithFiles)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCleanTrashFilesWorker::clearTrashFile,
                   [](DoCleanTrashFilesWorker *, const FileInfoPointer &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->cleanAllTrashFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, CleanAllTrashFiles_StateCheckFails)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->cleanAllTrashFiles();
    EXPECT_FALSE(result);
}

// ========== clearTrashFile Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, ClearTrashFile_RegularFile)
{
    auto testFile = createTestFile("trash_file.txt");
    FileInfoPointer fileInfo = testFile;

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCleanTrashFilesWorker::deleteFile,
                   [](DoCleanTrashFilesWorker *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->clearTrashFile(fileInfo);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, ClearTrashFile_Directory)
{
    auto testDir = createTestDir("trash_dir");
    FileInfoPointer dirInfo = testDir;

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCleanTrashFilesWorker::clearTrashFile,
                   [](DoCleanTrashFilesWorker *, const FileInfoPointer &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&DoCleanTrashFilesWorker::deleteFile,
                   [](DoCleanTrashFilesWorker *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->clearTrashFile(dirInfo);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, ClearTrashFile_StateCheckFails)
{
    auto testFile = createTestFile("state_fail.txt");
    FileInfoPointer fileInfo = testFile;

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->clearTrashFile(fileInfo);
    EXPECT_FALSE(result);
}

// ========== deleteFile Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, DeleteFile_Success)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/delete_test.txt");

    stub.set_lamda(&LocalFileHandler::deleteFile,
                   [](LocalFileHandler *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->deleteFile(fileUrl);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, DeleteFile_Failure)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/delete_fail.txt");

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

    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait,
                   [](DoCleanTrashFilesWorker *, const QUrl &,
                      const AbstractJobHandler::JobErrorType &,
                      const bool, const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool result = worker->deleteFile(fileUrl);
    EXPECT_TRUE(result);   // Should return true when skipped
}

// ========== doHandleErrorAndWait Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, DoHandleErrorAndWait_SkipAction)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/error_test.txt");

    stub.set_lamda(VADDR(AbstractWorker, setStat),
                   [](AbstractWorker *, const AbstractJobHandler::JobState &state) {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(state, AbstractJobHandler::JobState::kPauseState);
                   });

    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    worker->currentAction = AbstractJobHandler::SupportAction::kSkipAction;

    auto action = worker->doHandleErrorAndWait(testUrl,
                                               AbstractJobHandler::JobErrorType::kDeleteFileError);

    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kSkipAction);
}

TEST_F(TestDoCleanTrashFilesWorker, DoHandleErrorAndWait_RetryAction)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/retry_test.txt");

    stub.set_lamda(VADDR(AbstractWorker, setStat),
                   [](AbstractWorker *, const AbstractJobHandler::JobState &) {
                       __DBG_STUB_INVOKE__
                   });

    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    worker->currentAction = AbstractJobHandler::SupportAction::kRetryAction;

    auto action = worker->doHandleErrorAndWait(testUrl,
                                               AbstractJobHandler::JobErrorType::kDeleteFileError);

    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kRetryAction);
}

// ========== Signal Emission Tests ==========

TEST_F(TestDoCleanTrashFilesWorker, SignalEmission_ErrorNotify)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCleanTrashType;

    QUrl testUrl = QUrl::fromLocalFile("/tmp/error_signal.txt");

    bool errorEmitted = false;
    QObject::connect(worker, &DoCleanTrashFilesWorker::errorNotify,
                     [&errorEmitted](const JobInfoPointer &) {
                         errorEmitted = true;
                     });

    stub.set_lamda(VADDR(AbstractWorker, setStat),
                   [](AbstractWorker *, const AbstractJobHandler::JobState &) {
                       __DBG_STUB_INVOKE__
                   });

    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    worker->currentAction = AbstractJobHandler::SupportAction::kSkipAction;

    worker->doHandleErrorAndWait(testUrl, AbstractJobHandler::JobErrorType::kDeleteFileError);

    EXPECT_TRUE(errorEmitted);
}

// ========== Edge Cases ==========

TEST_F(TestDoCleanTrashFilesWorker, EdgeCase_MultipleTrashDirectories)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash1"));
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash2"));
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash3"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCleanTrashFilesWorker::clearTrashFile,
                   [](DoCleanTrashFilesWorker *, const FileInfoPointer &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->cleanAllTrashFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, EdgeCase_EmptyTrash)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->cleanAllTrashFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, EdgeCase_DeleteFileFails_ThenSkip)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/fail_skip.txt");

    stub.set_lamda(&LocalFileHandler::deleteFile,
                   [](LocalFileHandler *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&LocalFileHandler::errorString,
                   [](LocalFileHandler *) -> QString {
                       __DBG_STUB_INVOKE__
                       return "Error";
                   });

    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait,
                   [](DoCleanTrashFilesWorker *, const QUrl &,
                      const AbstractJobHandler::JobErrorType &,
                      const bool, const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool result = worker->deleteFile(fileUrl);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, EdgeCase_DeleteFileFails_ThenCancel)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/fail_cancel.txt");

    stub.set_lamda(&LocalFileHandler::deleteFile,
                   [](LocalFileHandler *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&LocalFileHandler::errorString,
                   [](LocalFileHandler *) -> QString {
                       __DBG_STUB_INVOKE__
                       return "Error";
                   });

    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait,
                   [](DoCleanTrashFilesWorker *, const QUrl &,
                      const AbstractJobHandler::JobErrorType &,
                      const bool, const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kCancelAction;
                   });

    bool result = worker->deleteFile(fileUrl);
    EXPECT_FALSE(result);
}

TEST_F(TestDoCleanTrashFilesWorker, EdgeCase_ClearTrashFile_NullFileInfo)
{
    FileInfoPointer nullInfo = nullptr;

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Should handle gracefully without crashing
    bool result = worker->clearTrashFile(nullInfo);
    EXPECT_FALSE(result);
}
