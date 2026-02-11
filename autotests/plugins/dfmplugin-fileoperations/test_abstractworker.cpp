// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryDir>
#include <QFile>
#include <QSignalSpy>
#include <QWaitCondition>

#include "stubext.h"

#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/operationsstackproxy.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

// Concrete test worker implementation
class TestWorker : public AbstractWorker
{
    Q_OBJECT
public:
    TestWorker()
        : AbstractWorker() { }
    virtual ~TestWorker() override { }

    // Public wrappers for protected methods
    using AbstractWorker::checkRetry;
    using AbstractWorker::createCopyJobInfo;
    using AbstractWorker::doWork;
    using AbstractWorker::emitCurrentTaskNotify;
    using AbstractWorker::emitErrorNotify;
    using AbstractWorker::emitProgressChangedNotify;
    using AbstractWorker::emitStateChangedNotify;
    using AbstractWorker::endWork;
    using AbstractWorker::formatFileName;
    using AbstractWorker::getAction;
    using AbstractWorker::initArgs;
    using AbstractWorker::isStopped;
    using AbstractWorker::onStatisticsFilesSizeFinish;
    using AbstractWorker::onStatisticsFilesSizeUpdate;
    using AbstractWorker::onUpdateProgress;
    using AbstractWorker::parentUrl;
    using AbstractWorker::pause;
    using AbstractWorker::pauseAllThread;
    using AbstractWorker::resume;
    using AbstractWorker::resumeAllThread;
    using AbstractWorker::saveOperations;
    using AbstractWorker::setStat;
    using AbstractWorker::stateCheck;
    using AbstractWorker::statisticsFilesSize;
    using AbstractWorker::stopAllThread;
    using AbstractWorker::syncFilesToDevice;
    using AbstractWorker::workerWait;

protected:
    bool doWork() override { return true; }
    bool initArgs() override { return true; }
    bool statisticsFilesSize() override { return true; }
};

class TestAbstractWorker : public testing::Test
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

        worker = new TestWorker();
        ASSERT_TRUE(worker);

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
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    TestWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestAbstractWorker, Constructor_Initializes)
{
    EXPECT_TRUE(worker);
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kUnknow);
    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kUnknowState);
}

TEST_F(TestAbstractWorker, Destructor_Cleanup)
{
    delete worker;
    worker = nullptr;
    SUCCEED();
}

// ========== setWorkArgs Tests ==========

TEST_F(TestAbstractWorker, SetWorkArgs_ValidArgs)
{
    JobHandlePointer handle(new AbstractJobHandler);
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    stub.set_lamda(VADDR(AbstractWorker, initArgs), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    worker->setWorkArgs(handle, sources, target);

    EXPECT_EQ(worker->sourceUrls, sources);
    EXPECT_EQ(worker->targetUrl, target);
}

// ========== doOperateWork Tests ==========

TEST_F(TestAbstractWorker, DoOperateWork_StartAction)
{
    stub.set_lamda(VADDR(AbstractWorker, doWork), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    worker->doOperateWork(AbstractJobHandler::SupportAction::kStartAction);
    SUCCEED();
}

TEST_F(TestAbstractWorker, DoOperateWork_PauseAction)
{
    stub.set_lamda(VADDR(AbstractWorker, pause), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->doOperateWork(AbstractJobHandler::SupportAction::kPauseAction);
    SUCCEED();
}

TEST_F(TestAbstractWorker, DoOperateWork_ResumeAction)
{
    stub.set_lamda(VADDR(AbstractWorker, resume), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->doOperateWork(AbstractJobHandler::SupportAction::kResumAction);
    SUCCEED();
}

TEST_F(TestAbstractWorker, DoOperateWork_StopAction)
{
    stub.set_lamda(VADDR(AbstractWorker, stop), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->doOperateWork(AbstractJobHandler::SupportAction::kStopAction);
    SUCCEED();
}

TEST_F(TestAbstractWorker, DoOperateWork_CancelAction)
{
    stub.set_lamda(VADDR(AbstractWorker, stop), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->doOperateWork(AbstractJobHandler::SupportAction::kCancelAction);
    SUCCEED();
}

// ========== stop Tests ==========

TEST_F(TestAbstractWorker, Stop_SetsStopWork)
{
    worker->stopWork = false;

    worker->stop();

    EXPECT_TRUE(worker->stopWork);
}

// ========== statisticsFilesSize Tests ==========

TEST_F(TestAbstractWorker, StatisticsFilesSize_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);
}

// ========== stateCheck Tests ==========

TEST_F(TestAbstractWorker, StateCheck_NormalState)
{
    worker->currentState = AbstractJobHandler::JobState::kRunningState;

    bool result = worker->stateCheck();

    EXPECT_TRUE(result);
}

TEST_F(TestAbstractWorker, StateCheck_Stopped)
{
    worker->stopWork = true;

    bool result = worker->stateCheck();

    EXPECT_FALSE(result);
}

TEST_F(TestAbstractWorker, StateCheck_PausedState)
{
    worker->currentState = AbstractJobHandler::JobState::kPauseState;

    stub.set_lamda(VADDR(AbstractWorker, workerWait), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->stateCheck();

    EXPECT_TRUE(result);
}

// ========== workerWait Tests ==========

TEST_F(TestAbstractWorker, WorkerWait_ResumeAction)
{
    worker->currentAction = AbstractJobHandler::SupportAction::kResumAction;

    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->workerWait();

    EXPECT_TRUE(result);
}

// ========== setStat Tests ==========

TEST_F(TestAbstractWorker, SetStat_ChangesState)
{
    worker->currentState = AbstractJobHandler::JobState::kRunningState;

    stub.set_lamda(VADDR(AbstractWorker, emitStateChangedNotify), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->setStat(AbstractJobHandler::JobState::kPauseState);

    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kPauseState);
}

// ========== initArgs Tests ==========

TEST_F(TestAbstractWorker, InitArgs_Success)
{
    bool result = worker->initArgs();

    EXPECT_TRUE(result);
}

// ========== endWork Tests ==========

TEST_F(TestAbstractWorker, EndWork_EmitsSignal)
{
    bool signalEmitted = false;
    QObject::connect(worker, &AbstractWorker::workerFinish,
                     [&signalEmitted]() {
                         signalEmitted = true;
                     });

    worker->endWork();

    EXPECT_TRUE(signalEmitted);
}

// ========== Signal Emission Tests ==========

TEST_F(TestAbstractWorker, EmitStateChangedNotify_Success)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->currentState = AbstractJobHandler::JobState::kRunningState;

    bool signalEmitted = false;
    QObject::connect(worker, &AbstractWorker::stateChangedNotify,
                     [&signalEmitted](const JobInfoPointer &) {
                         signalEmitted = true;
                     });

    worker->emitStateChangedNotify();

    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestAbstractWorker, EmitCurrentTaskNotify_Success)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    bool signalEmitted = false;
    QObject::connect(worker, &AbstractWorker::currentTaskNotify,
                     [&signalEmitted](const JobInfoPointer &) {
                         signalEmitted = true;
                     });

    worker->emitCurrentTaskNotify(from, to);

    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestAbstractWorker, EmitProgressChangedNotify_Success)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->sourceFilesTotalSize = 10000;

    bool signalEmitted = false;
    QObject::connect(worker, &AbstractWorker::progressChangedNotify,
                     [&signalEmitted](const JobInfoPointer &) {
                         signalEmitted = true;
                     });

    worker->emitProgressChangedNotify(5000);

    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestAbstractWorker, EmitErrorNotify_Success)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    bool signalEmitted = false;
    QObject::connect(worker, &AbstractWorker::errorNotify,
                     [&signalEmitted](const JobInfoPointer &) {
                         signalEmitted = true;
                     });

    worker->emitErrorNotify(from, to, AbstractJobHandler::JobErrorType::kCancelError);

    EXPECT_TRUE(signalEmitted);
}

// ========== Thread Control Tests ==========

TEST_F(TestAbstractWorker, Pause_ChangesState)
{
    stub.set_lamda(VADDR(AbstractWorker, setStat), [](AbstractWorker *, const AbstractJobHandler::JobState &state) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(state, AbstractJobHandler::JobState::kPauseState);
    });

    worker->pause();
}

TEST_F(TestAbstractWorker, Resume_ChangesState)
{
    stub.set_lamda(VADDR(AbstractWorker, setStat), [](AbstractWorker *, const AbstractJobHandler::JobState &state) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(state, AbstractJobHandler::JobState::kRunningState);
    });

    worker->resume();
}

TEST_F(TestAbstractWorker, PauseAllThread_Success)
{
    worker->pauseAllThread();
    SUCCEED();
}

TEST_F(TestAbstractWorker, ResumeAllThread_Success)
{
    worker->resumeAllThread();
    SUCCEED();
}

TEST_F(TestAbstractWorker, StopAllThread_Success)
{
    stub.set_lamda(VADDR(AbstractWorker, stop), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->stopAllThread();
    SUCCEED();
}

TEST_F(TestAbstractWorker, ResumeThread_WithErrorIds)
{
    QList<quint64> errorIds = { 1, 2, 3 };

    worker->resumeThread(errorIds);
    SUCCEED();
}

// ========== Utility Methods Tests ==========

TEST_F(TestAbstractWorker, FormatFileName_ValidName)
{
    QString original = "test file.txt";
    QString formatted = worker->formatFileName(original);

    EXPECT_FALSE(formatted.isEmpty());
}

TEST_F(TestAbstractWorker, FormatFileName_LongName)
{
    QString longName(300, 'a');
    QString formatted = worker->formatFileName(longName);

    EXPECT_LE(formatted.length(), 255);   // Should be truncated
}

TEST_F(TestAbstractWorker, IsStopped_NotStopped)
{
    worker->stopWork = false;

    bool result = worker->isStopped();

    EXPECT_FALSE(result);
}

TEST_F(TestAbstractWorker, IsStopped_Stopped)
{
    worker->stopWork = true;

    bool result = worker->isStopped();

    EXPECT_TRUE(result);
}

TEST_F(TestAbstractWorker, CreateCopyJobInfo_Success)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    QUrl from = QUrl::fromLocalFile("/tmp/source.txt");
    QUrl to = QUrl::fromLocalFile("/tmp/dest.txt");

    JobInfoPointer info = worker->createCopyJobInfo(from, to);

    ASSERT_NE(info, nullptr);
}

TEST_F(TestAbstractWorker, ParentUrl_ValidUrl)
{
    QUrl child = QUrl::fromLocalFile("/tmp/parent/child.txt");

    QUrl parent = worker->parentUrl(child);

    EXPECT_TRUE(parent.isValid());
}

TEST_F(TestAbstractWorker, SaveOperations_Success)
{
    worker->completeSourceFiles.append(QUrl::fromLocalFile("/tmp/test.txt"));

    stub.set_lamda(&OperationsStackProxy::instance, []() -> OperationsStackProxy & {
        __DBG_STUB_INVOKE__
        static OperationsStackProxy proxy;
        return proxy;
    });

    stub.set_lamda(&OperationsStackProxy::saveOperations,
                   [](OperationsStackProxy *, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    worker->saveOperations();
    SUCCEED();
}

TEST_F(TestAbstractWorker, CheckRetry_Success)
{
    worker->checkRetry();
    SUCCEED();
}

TEST_F(TestAbstractWorker, GetAction_SkipAction)
{
    worker->getAction(AbstractJobHandler::SupportAction::kSkipAction);

    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kSkipAction);
}

TEST_F(TestAbstractWorker, GetAction_RetryAction)
{
    worker->getAction(AbstractJobHandler::SupportAction::kRetryAction);

    EXPECT_EQ(worker->currentAction, AbstractJobHandler::SupportAction::kRetryAction);
}

TEST_F(TestAbstractWorker, SyncFilesToDevice_Success)
{
    stub.set_lamda(VADDR(AbstractWorker, needsSync), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    worker->syncFilesToDevice();
    SUCCEED();
}

TEST_F(TestAbstractWorker, SyncFilesToDevice_NeedsSync)
{
    stub.set_lamda(VADDR(AbstractWorker, needsSync), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractWorker, performSync), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->syncFilesToDevice();
    SUCCEED();
}

// ========== OnStatisticsFilesSizeUpdate Tests ==========

TEST_F(TestAbstractWorker, OnStatisticsFilesSizeUpdate_UpdatesSize)
{
    qint64 initialSize = worker->sourceFilesTotalSize;

    worker->onStatisticsFilesSizeUpdate(1000);

    EXPECT_EQ(worker->sourceFilesTotalSize, initialSize + 1000);
}

// ========== OnStatisticsFilesSizeFinish Tests ==========

TEST_F(TestAbstractWorker, OnStatisticsFilesSizeFinish_Success)
{
    worker->onStatisticsFilesSizeFinish();
    SUCCEED();
}

// ========== Edge Cases ==========

TEST_F(TestAbstractWorker, EdgeCase_MultipleStops)
{
    worker->stop();
    worker->stop();
    worker->stop();

    EXPECT_TRUE(worker->stopWork);
}

TEST_F(TestAbstractWorker, EdgeCase_StateChangeWhileStopped)
{
    worker->stopWork = true;

    stub.set_lamda(VADDR(AbstractWorker, emitStateChangedNotify), [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    worker->setStat(AbstractJobHandler::JobState::kPauseState);

    EXPECT_EQ(worker->currentState, AbstractJobHandler::JobState::kPauseState);
}

TEST_F(TestAbstractWorker, EdgeCase_EmitSignalsWithoutHandle)
{
    worker->handle.reset();

    // Should not crash even without handle
    worker->emitStateChangedNotify();
    worker->emitCurrentTaskNotify(QUrl(), QUrl());
    worker->emitProgressChangedNotify(100);

    SUCCEED();
}

#include "test_abstractworker.moc"
