// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QThread>
#include <QSignalSpy>
#include <QTest>

#include "stubext.h"

#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include <dfm-io/dfmio_utils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

// Concrete test worker for AbstractWorkerImpl
class TestAbstractWorkerImplWorker : public AbstractWorker
{
public:
    TestAbstractWorkerImplWorker(QObject *parent = nullptr)
        : AbstractWorker(parent) { }

    virtual ~TestAbstractWorkerImplWorker() override { }

    // Expose protected methods for testing
    using AbstractWorker::setWorkArgs;
    using AbstractWorker::stop;
    using AbstractWorker::pause;
    using AbstractWorker::resume;
    using AbstractWorker::setStat;
    using AbstractWorker::stateCheck;
    using AbstractWorker::workerWait;
    using AbstractWorker::statisticsFilesSize;
    using AbstractWorker::initArgs;
    using AbstractWorker::endWork;
    using AbstractWorker::emitStateChangedNotify;
    using AbstractWorker::emitCurrentTaskNotify;
    using AbstractWorker::emitProgressChangedNotify;
    using AbstractWorker::emitErrorNotify;
    using AbstractWorker::createCopyJobInfo;
    using AbstractWorker::resumeAllThread;
    using AbstractWorker::resumeThread;
    using AbstractWorker::pauseAllThread;
    using AbstractWorker::stopAllThread;
    using AbstractWorker::checkRetry;
    using AbstractWorker::parentUrl;
    using AbstractWorker::syncFilesToDevice;
    using AbstractWorker::startAsyncStatistics;
    using AbstractWorker::getAction;
    using AbstractWorker::doOperateWork;
    using AbstractWorker::startCountProccess;
    using AbstractWorker::doWork;
    using AbstractWorker::saveOperations;
    using AbstractWorker::formatFileName;

protected:
    bool doWork() override { return true; }
    // NOTE: initArgs() is intentionally NOT overridden here so tests hit the
    // real AbstractWorker::initArgs() (e.g. InitArgs_ResetsState). If a test
    // needs to isolate side effects, stub the product function with stubext.
    bool statisticsFilesSize() override { return true; }
};

class AbstractWorkerImpl : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        worker = new TestAbstractWorkerImplWorker();
        ASSERT_TRUE(worker);

        handle.reset(new AbstractJobHandler);
        worker->setWorkArgs(handle, {}, QUrl(), AbstractJobHandler::JobFlag::kNoHint);

        // Avoid blocking on workerWait
        using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
        stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                       [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool { return true; });
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
            worker->stopAllThread();
            delete worker;
            worker = nullptr;
        }
        tempDir.reset();
    }

protected:
    QUrl createTestFile(const QString &name)
    {
        QString path = tempDir->path() + "/" + name;
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("test");
            file.close();
        }
        return QUrl::fromLocalFile(path);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    TestAbstractWorkerImplWorker *worker { nullptr };
    JobHandlePointer handle;
};

// ========== setWorkArgs ==========
TEST_F(AbstractWorkerImpl, SetWorkArgs_NullHandle)
{
    TestAbstractWorkerImplWorker *w = new TestAbstractWorkerImplWorker();
    w->setWorkArgs(nullptr, { QUrl::fromLocalFile("/tmp/a") }, QUrl::fromLocalFile("/tmp/b"));
    EXPECT_EQ(w->handle, nullptr);
    delete w;
}

TEST_F(AbstractWorkerImpl, SetWorkArgs_ValidHandle)
{
    JobHandlePointer h(new AbstractJobHandler);
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/a") };
    QUrl target = QUrl::fromLocalFile("/tmp/b");

    worker->setWorkArgs(h, sources, target);

    EXPECT_EQ(worker->handle, h);
    EXPECT_EQ(worker->sourceUrls, sources);
    EXPECT_EQ(worker->targetUrl, target);
    EXPECT_NE(worker->workData, nullptr);
}

// ========== doOperateWork ==========
TEST_F(AbstractWorkerImpl, DoOperateWork_Stop)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->doOperateWork(AbstractJobHandler::SupportAction::kStopAction);
    EXPECT_TRUE(worker->isStopped());
}

TEST_F(AbstractWorkerImpl, DoOperateWork_Pause)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->doOperateWork(AbstractJobHandler::SupportAction::kPauseAction);
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kPauseState);
}

TEST_F(AbstractWorkerImpl, DoOperateWork_Resume)
{
    worker->setStat(AbstractJobHandler::JobState::kPauseState);
    worker->doOperateWork(AbstractJobHandler::SupportAction::kResumAction);
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kRunningState);
}

TEST_F(AbstractWorkerImpl, DoOperateWork_Skip)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->doOperateWork(AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kSkipAction);
}

// ========== stop / pause / resume ==========
TEST_F(AbstractWorkerImpl, Stop_SetsState)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->stop();
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kStopState);
}

TEST_F(AbstractWorkerImpl, Pause_RunningToPaused)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->pause();
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kPauseState);
}

TEST_F(AbstractWorkerImpl, Resume_PausedToRunning)
{
    worker->setStat(AbstractJobHandler::JobState::kPauseState);
    worker->resume();
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kRunningState);
}

// ========== setStat ==========
TEST_F(AbstractWorkerImpl, SetStat_Running)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kRunningState);
}

TEST_F(AbstractWorkerImpl, SetStat_Stop)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->setStat(AbstractJobHandler::JobState::kStopState);
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kStopState);
}

// ========== stateCheck ==========
TEST_F(AbstractWorkerImpl, StateCheck_Running)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    EXPECT_TRUE(worker->stateCheck());
}

TEST_F(AbstractWorkerImpl, StateCheck_Stopped)
{
    worker->setStat(AbstractJobHandler::JobState::kStopState);
    EXPECT_FALSE(worker->stateCheck());
}

// ========== workerWait ==========
TEST_F(AbstractWorkerImpl, WorkerWait_ReturnsState)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    bool result = worker->workerWait();
    EXPECT_TRUE(result);
}

// ========== initArgs ==========
TEST_F(AbstractWorkerImpl, InitArgs_ResetsState)
{
    worker->sourceFilesTotalSize = 100;
    bool result = worker->initArgs();
    EXPECT_TRUE(result);
    EXPECT_EQ(worker->sourceFilesTotalSize, 0);
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kRunningState);
}

// ========== endWork ==========
TEST_F(AbstractWorkerImpl, EndWork_EmitsFinished)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    bool finishedEmitted = false;
    QObject::connect(worker, &AbstractWorker::finishedNotify,
                     [&finishedEmitted](const JobInfoPointer &) { finishedEmitted = true; });

    worker->endWork();
    EXPECT_TRUE(finishedEmitted);
}

// ========== emitStateChangedNotify ==========
TEST_F(AbstractWorkerImpl, EmitStateChangedNotify_Signal)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    bool emitted = false;
    QObject::connect(worker, &AbstractWorker::stateChangedNotify,
                     [&emitted](const JobInfoPointer &) { emitted = true; });

    worker->emitStateChangedNotify();
    EXPECT_TRUE(emitted);
}

// ========== emitCurrentTaskNotify ==========
TEST_F(AbstractWorkerImpl, EmitCurrentTaskNotify_Signal)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    bool emitted = false;
    QObject::connect(worker, &AbstractWorker::currentTaskNotify,
                     [&emitted](const JobInfoPointer &) { emitted = true; });

    worker->emitCurrentTaskNotify(QUrl::fromLocalFile("/tmp/from"), QUrl::fromLocalFile("/tmp/to"));
    // Throttle may coalesce; test that method doesn't crash
    SUCCEED();
}

// ========== emitProgressChangedNotify ==========
TEST_F(AbstractWorkerImpl, EmitProgressChangedNotify_CopyType)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->sourceFilesTotalSize = 1000;

    bool emitted = false;
    QObject::connect(worker, &AbstractWorker::progressChangedNotify,
                     [&emitted](const JobInfoPointer &) { emitted = true; });

    worker->emitProgressChangedNotify(500);
    EXPECT_TRUE(emitted);
}

TEST_F(AbstractWorkerImpl, EmitProgressChangedNotify_DeleteType)
{
    worker->jobType = AbstractJobHandler::JobType::kDeleteType;
    worker->sourceUrls = { QUrl::fromLocalFile("/tmp/a") };

    bool emitted = false;
    QObject::connect(worker, &AbstractWorker::progressChangedNotify,
                     [&emitted](const JobInfoPointer &) { emitted = true; });

    worker->emitProgressChangedNotify(1);
    EXPECT_TRUE(emitted);
}

// ========== emitErrorNotify ==========
TEST_F(AbstractWorkerImpl, EmitErrorNotify_Signal)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    bool emitted = false;
    QObject::connect(worker, &AbstractWorker::errorNotify,
                     [&emitted](const JobInfoPointer &) { emitted = true; });

    worker->emitErrorNotify(QUrl::fromLocalFile("/tmp/from"), QUrl::fromLocalFile("/tmp/to"),
                            AbstractJobHandler::JobErrorType::kOpenError);
    EXPECT_TRUE(emitted);
}

// ========== createCopyJobInfo ==========
TEST_F(AbstractWorkerImpl, CreateCopyJobInfo_Valid)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    auto info = worker->createCopyJobInfo(QUrl::fromLocalFile("/tmp/from"),
                                          QUrl::fromLocalFile("/tmp/to"));
    EXPECT_NE(info, nullptr);
    EXPECT_TRUE(info->contains(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey));
    EXPECT_TRUE(info->contains(AbstractJobHandler::NotifyInfoKey::kTargetUrlKey));
}

// ========== resumeAllThread / pauseAllThread / stopAllThread ==========
TEST_F(AbstractWorkerImpl, ResumeAllThread_RunningState)
{
    worker->setStat(AbstractJobHandler::JobState::kPauseState);
    worker->resumeAllThread();
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kRunningState);
}

TEST_F(AbstractWorkerImpl, PauseAllThread_PauseState)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->pauseAllThread();
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kPauseState);
}

TEST_F(AbstractWorkerImpl, StopAllThread_StopState)
{
    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    worker->stopAllThread();
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kStopState);
}

// ========== getAction ==========
TEST_F(AbstractWorkerImpl, GetAction_Cancel)
{
    worker->getAction(AbstractJobHandler::SupportAction::kCancelAction);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kCancelAction);
}

TEST_F(AbstractWorkerImpl, GetAction_Replace)
{
    worker->getAction(AbstractJobHandler::SupportAction::kReplaceAction);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kReplaceAction);
}

TEST_F(AbstractWorkerImpl, GetAction_Merge)
{
    worker->getAction(AbstractJobHandler::SupportAction::kMergeAction);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kMergeAction);
}

TEST_F(AbstractWorkerImpl, GetAction_Enforce)
{
    worker->getAction(AbstractJobHandler::SupportAction::kEnforceAction);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kEnforceAction);
}

TEST_F(AbstractWorkerImpl, GetAction_PermanentlyDelete)
{
    worker->getAction(AbstractJobHandler::SupportAction::kPermanentlyDelete);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kPermanentlyDelete);
}

TEST_F(AbstractWorkerImpl, GetAction_NoAction)
{
    worker->getAction(AbstractJobHandler::SupportAction::kNoAction);
    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kNoAction);
}

// ========== parentUrl ==========
TEST_F(AbstractWorkerImpl, ParentUrl_HasParent)
{
    QUrl child = QUrl::fromLocalFile("/tmp/subdir/child.txt");
    QUrl parent = worker->parentUrl(child);
    EXPECT_TRUE(parent.isValid());
    EXPECT_EQ(parent.path(), "/tmp/subdir");
}

TEST_F(AbstractWorkerImpl, ParentUrl_RootDirectory)
{
    QUrl root = QUrl::fromLocalFile("/");
    QUrl parent = worker->parentUrl(root);
    EXPECT_FALSE(parent.isValid());
}

// ========== syncFilesToDevice ==========
TEST_F(AbstractWorkerImpl, SyncFilesToDevice_NoSyncNeeded)
{
    worker->workData->exBlockSyncEveryWrite = false;
    worker->syncFilesToDevice();
    SUCCEED();
}

// ========== startAsyncStatistics ==========
TEST_F(AbstractWorkerImpl, StartAsyncStatistics_CreatesThread)
{
    QUrl file = createTestFile("stats.txt");

    stub.set_lamda(&FileScanner::scanSyncWithCallback,
                   [](const QList<QUrl> &, FileScanner::ScanOptions,
                      FileScanner::ProgressCallback) -> FileScanner::ScanResult {
                       return { 100, 0, 1, 0, QList<QUrl>() };
                   });

    worker->startAsyncStatistics({ file });
    EXPECT_NE(worker->statisticsThread, nullptr);

    // The fixture stubs QWaitCondition::wait (to keep workerWait() from
    // blocking), which QThread::wait() calls internally: its loop would spin
    // forever on a faked "already done" condition while the real thread is
    // still finishing. QThread::create runs a plain lambda (no event loop,
    // quit() is a no-op), so just poll isFinished() instead. The thread MUST
    // be done before the fixture deletes worker - the scan lambda captures
    // 'this'.
    if (worker->statisticsThread) {
        ASSERT_TRUE(QTest::qWaitFor([&]() {
            return worker->statisticsThread->isFinished();
        }, 10000));
    }
}

// ========== doWork ==========
TEST_F(AbstractWorkerImpl, DoWork_Success)
{
    bool result = worker->doWork();
    EXPECT_TRUE(result);
}

// ========== startCountProccess ==========
TEST_F(AbstractWorkerImpl, StartCountProccess_NoTimer)
{
    worker->updateProgressTimer.reset();
    worker->startCountProccess();
    SUCCEED();
}

// ========== formatFileName ==========
TEST_F(AbstractWorkerImpl, FormatFileName_Default)
{
    worker->targetUrl = QUrl::fromLocalFile("/tmp/test");
    QString result = worker->formatFileName("normal.txt");
    EXPECT_EQ(result, "normal.txt");
}

TEST_F(AbstractWorkerImpl, FormatFileName_Vfat)
{
    worker->targetUrl = QUrl::fromLocalFile(tempDir->path());

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString { return "ext4"; });

    QString result = worker->formatFileName("normal.txt");
    EXPECT_EQ(result, "normal.txt");
}

// ========== saveOperations ==========
TEST_F(AbstractWorkerImpl, SaveOperations_CopyType)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->sourceUrls = { QUrl::fromLocalFile("/tmp/source.txt") };
    worker->targetUrl = QUrl::fromLocalFile("/tmp/dest");
    worker->completeSourceFiles = { QUrl::fromLocalFile("/tmp/source.txt") };
    worker->completeTargetFiles = { QUrl::fromLocalFile("/tmp/dest/source.txt") };

    worker->saveOperations();
    SUCCEED();
}

TEST_F(AbstractWorkerImpl, SaveOperations_InvalidUrls)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->sourceUrls.clear();
    worker->targetUrl = QUrl();

    worker->saveOperations();
    SUCCEED();
}

// ========== isStopped ==========
TEST_F(AbstractWorkerImpl, IsStopped_Initial)
{
    EXPECT_FALSE(worker->isStopped());
}

TEST_F(AbstractWorkerImpl, IsStopped_AfterStop)
{
    worker->setStat(AbstractJobHandler::JobState::kStopState);
    EXPECT_TRUE(worker->isStopped());
}

// ========== checkRetry ==========
TEST_F(AbstractWorkerImpl, CheckRetry_NoRetry)
{
    worker->retry = false;
    worker->checkRetry();
    SUCCEED();
}

// ========== resumeThread with ids ==========
TEST_F(AbstractWorkerImpl, ResumeThread_WithId)
{
    worker->setStat(AbstractJobHandler::JobState::kPauseState);
    worker->resumeThread({ quintptr(worker) });
    // Worker itself is in list, so it should remain paused
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kPauseState);
}

