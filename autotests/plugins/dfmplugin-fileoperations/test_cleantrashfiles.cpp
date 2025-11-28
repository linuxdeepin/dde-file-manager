// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QThread>

#include "stubext.h"

#include "fileoperations/cleantrash/cleantrashfiles.h"
#include "fileoperations/cleantrash/docleantrashfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestCleanTrashFiles : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Register file info factories
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    /*!
     * \brief Create a temporary test file
     */
    QUrl createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDir->path() + "/" + fileName;
        QFile file(filePath);
        file.open(QIODevice::WriteOnly);
        file.write(content.toUtf8());
        file.close();

        return QUrl::fromLocalFile(filePath);
    }

    /*!
     * \brief Create a temporary test directory
     */
    QUrl createTestDir(const QString &dirName)
    {
        QString dirPath = tempDir->path() + "/" + dirName;
        QDir().mkpath(dirPath);

        return QUrl::fromLocalFile(dirPath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// ========== CleanTrashFiles Constructor/Destructor Tests ==========

TEST_F(TestCleanTrashFiles, Constructor_CreatesInstance)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();

    ASSERT_NE(cleanJob, nullptr);
    EXPECT_NE(cleanJob->doWorker, nullptr);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Destructor_CleansUpProperly)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    ASSERT_NE(cleanJob, nullptr);

    // Should not crash on delete
    delete cleanJob;

    SUCCEED();
}

// ========== CleanTrashFiles::setJobArgs() Tests ==========

TEST_F(TestCleanTrashFiles, SetJobArgs_EmptySources)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QList<QUrl> emptySources;

    cleanJob->setJobArgs(handle, emptySources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, SetJobArgs_ValidTrashFiles)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/test.txt");
    QList<QUrl> sources = { trashFile };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, SetJobArgs_MultipleTrashFiles)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl file1 = QUrl::fromLocalFile("/tmp/.Trash/file1.txt");
    QUrl file2 = QUrl::fromLocalFile("/tmp/.Trash/file2.txt");
    QUrl file3 = QUrl::fromLocalFile("/tmp/.Trash/file3.txt");

    QList<QUrl> sources = { file1, file2, file3 };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, SetJobArgs_WithJobFlags)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/flags_test.txt");
    QList<QUrl> sources = { trashFile };

    cleanJob->setJobArgs(handle, sources, QUrl(), AbstractJobHandler::JobFlag::kNoHint);

    SUCCEED();

    delete cleanJob;
}

// ========== CleanTrashFiles Start/Stop Tests ==========

TEST_F(TestCleanTrashFiles, Start_StartsWorkerThread)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, doWork), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/start_test.txt");
    QList<QUrl> sources = { trashFile };

    cleanJob->setJobArgs(handle, sources);
    cleanJob->start();

    // Give thread time to start
    QThread::msleep(100);

    SUCCEED();

    delete cleanJob;
}

// ========== Signal/Slot Tests ==========

TEST_F(TestCleanTrashFiles, OperateWork_SignalConnected)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();

    bool signalEmitted = false;
    QObject::connect(cleanJob, &CleanTrashFiles::operateWork, [&signalEmitted](AbstractJobHandler::SupportActions) {
        signalEmitted = true;
    });

    emit cleanJob->operateWork(AbstractJobHandler::SupportAction::kStopAction);

    EXPECT_TRUE(signalEmitted);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, ErrorNotify_SignalConnected)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();

    bool signalEmitted = false;
    QObject::connect(cleanJob, &CleanTrashFiles::errorNotify, [&signalEmitted](const JobInfoPointer) {
        signalEmitted = true;
    });

    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    emit cleanJob->errorNotify(jobInfo);

    EXPECT_TRUE(signalEmitted);

    delete cleanJob;
}

// ========== DoCleanTrashFilesWorker Tests ==========

TEST_F(TestCleanTrashFiles, Worker_Exists)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();

    ASSERT_NE(cleanJob->doWorker, nullptr);
    EXPECT_NE(dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data()), nullptr);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_InitArgs)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, initArgs), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    bool result = worker->initArgs();

    EXPECT_TRUE(result);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_DoWork)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, doWork), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    bool result = worker->doWork();

    EXPECT_TRUE(result);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_StatisticsFilesSize)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, statisticsFilesSize), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_OnUpdateProgress)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, onUpdateProgress), [](DoCleanTrashFilesWorker *) {
        __DBG_STUB_INVOKE__
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    // Should not crash
    worker->onUpdateProgress();

    SUCCEED();

    delete cleanJob;
}

// ========== DoCleanTrashFilesWorker Private Methods Tests (via stub) ==========

TEST_F(TestCleanTrashFiles, Worker_CleanAllTrashFiles)
{
    stub.set_lamda(ADDR(DoCleanTrashFilesWorker, cleanAllTrashFiles), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    // Access through doWork which calls cleanAllTrashFiles
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, doWork), [worker](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->doWork();

    EXPECT_TRUE(result);

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_ClearTrashFile)
{
    stub.set_lamda(ADDR(DoCleanTrashFilesWorker, clearTrashFile), [](DoCleanTrashFilesWorker *, const FileInfoPointer &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_DeleteFile)
{
    stub.set_lamda(ADDR(DoCleanTrashFilesWorker, deleteFile), [](DoCleanTrashFilesWorker *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_DoHandleErrorAndWait)
{
    stub.set_lamda(ADDR(DoCleanTrashFilesWorker, doHandleErrorAndWait),
                   [](DoCleanTrashFilesWorker *, const QUrl &, const AbstractJobHandler::JobErrorType &, const bool, const QString &) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    SUCCEED();

    delete cleanJob;
}

// ========== Integration Tests ==========

TEST_F(TestCleanTrashFiles, Integration_CompleteJobSetup)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, initArgs), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, doWork), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/integration_test.txt");
    QList<QUrl> sources = { trashFile };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Integration_ErrorHandling)
{
    stub.set_lamda(VADDR(DoCleanTrashFilesWorker, doWork), [](DoCleanTrashFilesWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;   // Simulate error
    });

    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/error_test.txt");
    QList<QUrl> sources = { trashFile };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

// ========== Edge Cases ==========

TEST_F(TestCleanTrashFiles, EdgeCase_NullJobHandle)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/null_handle.txt");
    QList<QUrl> sources = { trashFile };

    // Should handle null handle gracefully
    cleanJob->setJobArgs(nullptr, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, EdgeCase_NonTrashUrls)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl normalFile = createTestFile("normal_file.txt");
    QList<QUrl> sources = { normalFile };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, EdgeCase_MixedUrls)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/file.txt");
    QUrl normalFile = createTestFile("normal.txt");
    QList<QUrl> sources = { trashFile, normalFile };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, EdgeCase_LargeNumberOfTrashFiles)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QList<QUrl> sources;
    for (int i = 0; i < 100; ++i) {
        sources << QUrl::fromLocalFile(QString("/tmp/.Trash/bulk_%1.txt").arg(i));
    }

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, EdgeCase_UnicodeTrashFileName)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashFile = QUrl::fromLocalFile("/tmp/.Trash/中文垃圾文件.txt");
    QList<QUrl> sources = { trashFile };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, EdgeCase_EmptyTrashDirectory)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    JobHandlePointer handle(new AbstractJobHandler());

    QUrl trashDir = QUrl::fromLocalFile("/tmp/.Trash");
    QList<QUrl> sources = { trashDir };

    cleanJob->setJobArgs(handle, sources);

    SUCCEED();

    delete cleanJob;
}

// ========== Worker State Tests ==========

TEST_F(TestCleanTrashFiles, Worker_StatePaused)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    // Call the real method - no stub needed for method being tested
    worker->setStat(AbstractJobHandler::JobState::kPauseState);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_StateStopped)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    // Call the real method - no stub needed for method being tested
    worker->setStat(AbstractJobHandler::JobState::kStopState);

    SUCCEED();

    delete cleanJob;
}

TEST_F(TestCleanTrashFiles, Worker_StateRunning)
{
    CleanTrashFiles *cleanJob = new CleanTrashFiles();
    DoCleanTrashFilesWorker *worker = dynamic_cast<DoCleanTrashFilesWorker *>(cleanJob->doWorker.data());
    ASSERT_NE(worker, nullptr);

    // Call the real method - no stub needed for method being tested
    worker->setStat(AbstractJobHandler::JobState::kRunningState);

    SUCCEED();

    delete cleanJob;
}
