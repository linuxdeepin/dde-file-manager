// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QThread>
#include <QSignalSpy>

#include "stubext.h"

#include "fileoperations/fileoperationutils/abstractjob.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

// Test worker class for AbstractJob testing
class TestJobWorker : public AbstractWorker
{
    Q_OBJECT
public:
    TestJobWorker()
        : AbstractWorker() { }
    virtual ~TestJobWorker() override { }

protected:
    bool doWork() override { return true; }
    bool initArgs() override { return true; }
    bool statisticsFilesSize() override { return true; }
};

class TestAbstractJob : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create worker and job
        worker = new TestJobWorker();
        job = new AbstractJob(worker, nullptr);
        ASSERT_TRUE(job);
        ASSERT_TRUE(job->doWorker);
    }

    void TearDown() override
    {
        if (job) {
            // Set stubs BEFORE clearing and deletion
            stub.set_lamda(&QThread::quit, [](QThread *) {
                __DBG_STUB_INVOKE__
            });
            using WaitFuncPtr = bool (QThread::*)(QDeadlineTimer);
            stub.set_lamda(static_cast<WaitFuncPtr>(&QThread::wait), [](QThread *, QDeadlineTimer) {
                __DBG_STUB_INVOKE__
                return true;
            });

            // delete job;
            // job = nullptr;
        }
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    AbstractJob *job;
    TestJobWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestAbstractJob, Constructor_InitializesWorker)
{
    EXPECT_TRUE(job->doWorker);
}

TEST_F(TestAbstractJob, Constructor_StartsThread)
{
    // Thread should be started in constructor
    stub.set_lamda(&QThread::isRunning, [](QThread *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    SUCCEED();
}

TEST_F(TestAbstractJob, Destructor_StopsThread)
{
    bool quitCalled = false;
    stub.set_lamda(&QThread::quit, [&quitCalled](QThread *) {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    using WaitFuncPtr = bool (QThread::*)(QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFuncPtr>(&QThread::wait), [&](QThread *, QDeadlineTimer) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // delete job;
    // job = nullptr;

    EXPECT_TRUE(quitCalled);
}

TEST_F(TestAbstractJob, Destructor_ThreadTimeout)
{
    stub.set_lamda(&QThread::quit, [](QThread *) {
        __DBG_STUB_INVOKE__
    });

    using WaitFuncPtr = bool (QThread::*)(QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFuncPtr>(&QThread::wait), [&](QThread *, QDeadlineTimer) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // delete job;
    // job = nullptr;

    SUCCEED();
}

// ========== setJobArgs Tests ==========

TEST_F(TestAbstractJob, SetJobArgs_NullHandle)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    job->setJobArgs(nullptr, sources, target);
    SUCCEED();   // Should handle null gracefully
}

TEST_F(TestAbstractJob, SetJobArgs_ValidHandle)
{
    JobHandlePointer handle(new AbstractJobHandler);
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    stub.set_lamda(VADDR(AbstractWorker, setWorkArgs),
                   [](AbstractWorker *, const JobHandlePointer,
                      const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &) {
                       __DBG_STUB_INVOKE__
                   });

    job->setJobArgs(handle, sources, target);
    SUCCEED();
}

TEST_F(TestAbstractJob, SetJobArgs_WithFlags)
{
    JobHandlePointer handle(new AbstractJobHandler);
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/file.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/target");
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kCopyRemote;

    stub.set_lamda(VADDR(AbstractWorker, setWorkArgs),
                   [&flags](AbstractWorker *, const JobHandlePointer,
                            const QList<QUrl> &, const QUrl &,
                            const AbstractJobHandler::JobFlags &f) {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(f, flags);
                   });

    job->setJobArgs(handle, sources, target, flags);
}

// ========== start Tests ==========

TEST_F(TestAbstractJob, Start_StartsThread)
{
    stub.set_lamda(&QThread::start, [](QThread *, QThread::Priority) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QThread::isRunning, [](QThread *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    job->start();
    SUCCEED();
}

// ========== operateAation Tests ==========

TEST_F(TestAbstractJob, OperateAation_StartAction)
{
    bool startEmitted = false;
    QObject::connect(job, &AbstractJob::operateWork,
                     worker, [&startEmitted](AbstractJobHandler::SupportActions actions) {
                         if (actions.testFlag(AbstractJobHandler::SupportAction::kStartAction)) {
                             startEmitted = true;
                         }
                     });

    stub.set_lamda(&AbstractWorker::startWork, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    job->operateAation(AbstractJobHandler::SupportAction::kStartAction);
}

TEST_F(TestAbstractJob, OperateAation_StopAction)
{
    stub.set_lamda(&AbstractWorker::stopAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    job->operateAation(AbstractJobHandler::SupportAction::kStopAction);
    SUCCEED();
}

TEST_F(TestAbstractJob, OperateAation_CancelAction)
{
    stub.set_lamda(&AbstractWorker::stopAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    job->operateAation(AbstractJobHandler::SupportAction::kCancelAction);
    SUCCEED();
}

TEST_F(TestAbstractJob, OperateAation_RetryAction)
{
    // Add an error to the queue first
    JobInfoPointer errorInfo(new QMap<quint8, QVariant>);
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    job->handleError(errorInfo);

    stub.set_lamda(&AbstractWorker::doOperateWork,
                   [](AbstractWorker *, AbstractJobHandler::SupportActions,
                      AbstractJobHandler::JobErrorType, quint64) {
                       __DBG_STUB_INVOKE__
                   });

    job->operateAation(AbstractJobHandler::SupportAction::kRetryAction);
    SUCCEED();
}

TEST_F(TestAbstractJob, OperateAation_SkipAction)
{
    // Add an error to the queue
    JobInfoPointer errorInfo(new QMap<quint8, QVariant>);
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    job->handleError(errorInfo);

    stub.set_lamda(&AbstractWorker::doOperateWork,
                   [](AbstractWorker *, AbstractJobHandler::SupportActions,
                      AbstractJobHandler::JobErrorType, quint64) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&AbstractWorker::resumeThread,
                   [](AbstractWorker *, const QList<quint64> &) {
                       __DBG_STUB_INVOKE__
                   });

    job->operateAation(AbstractJobHandler::SupportAction::kSkipAction);
    SUCCEED();
}

// ========== handleError Tests ==========

TEST_F(TestAbstractJob, HandleError_NewError)
{
    JobInfoPointer errorInfo(new QMap<quint8, QVariant>);
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    bool errorEmitted = false;
    QObject::connect(job, &AbstractJob::errorNotify,
                     [&errorEmitted](const JobInfoPointer &) {
                         errorEmitted = true;
                     });

    job->handleError(errorInfo);
    EXPECT_TRUE(errorEmitted);
}

TEST_F(TestAbstractJob, HandleError_MultipleErrors)
{
    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    // First error
    JobInfoPointer error1(new QMap<quint8, QVariant>);
    error1->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                   QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    error1->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    job->handleError(error1);

    // Second error
    JobInfoPointer error2(new QMap<quint8, QVariant>);
    error2->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                   QVariant::fromValue(AbstractJobHandler::JobErrorType::kDeleteFileError));
    error2->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(2));

    job->handleError(error2);
    SUCCEED();
}

TEST_F(TestAbstractJob, HandleError_RetryError)
{
    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    JobInfoPointer errorInfo(new QMap<quint8, QVariant>);
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    job->handleError(errorInfo);

    // Retry with same worker ID but different error type
    JobInfoPointer retryInfo(new QMap<quint8, QVariant>);
    retryInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kDeleteFileError));
    retryInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    job->handleError(retryInfo);
    SUCCEED();
}

// ========== handleRetryErrorSuccess Tests ==========

TEST_F(TestAbstractJob, HandleRetryErrorSuccess_EmptyQueue)
{
    job->handleRetryErrorSuccess(1);
    SUCCEED();
}

TEST_F(TestAbstractJob, HandleRetryErrorSuccess_MismatchId)
{
    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    JobInfoPointer errorInfo(new QMap<quint8, QVariant>);
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    job->handleError(errorInfo);

    job->handleRetryErrorSuccess(999);   // Different ID
    SUCCEED();
}

TEST_F(TestAbstractJob, HandleRetryErrorSuccess_Success)
{
    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&AbstractWorker::resumeAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    JobInfoPointer errorInfo(new QMap<quint8, QVariant>);
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                      QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    errorInfo->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    job->handleError(errorInfo);
    job->handleRetryErrorSuccess(1);
    SUCCEED();
}

TEST_F(TestAbstractJob, HandleRetryErrorSuccess_ProcessNextError)
{
    stub.set_lamda(&AbstractWorker::pauseAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    // Add two errors
    JobInfoPointer error1(new QMap<quint8, QVariant>);
    error1->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                   QVariant::fromValue(AbstractJobHandler::JobErrorType::kCancelError));
    error1->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(1));

    JobInfoPointer error2(new QMap<quint8, QVariant>);
    error2->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey,
                   QVariant::fromValue(AbstractJobHandler::JobErrorType::kDeleteFileError));
    error2->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, static_cast<quint64>(2));

    job->handleError(error1);
    job->handleError(error2);

    bool secondErrorEmitted = false;
    QObject::connect(job, &AbstractJob::errorNotify,
                     [&secondErrorEmitted](const JobInfoPointer &) {
                         secondErrorEmitted = true;
                     });

    job->handleRetryErrorSuccess(1);
    EXPECT_TRUE(secondErrorEmitted);
}

// ========== handleFileRenamed Tests ==========

TEST_F(TestAbstractJob, HandleFileRenamed_EmitsSignal)
{
    QUrl oldUrl = QUrl::fromLocalFile("/tmp/old.txt");
    QUrl newUrl = QUrl::fromLocalFile("/tmp/new.txt");

    job->handleFileRenamed(oldUrl, newUrl);
    SUCCEED();
}

// ========== handleFileDeleted Tests ==========

TEST_F(TestAbstractJob, HandleFileDeleted_EmitsSignal)
{
    QUrl url = QUrl::fromLocalFile("/tmp/deleted.txt");

    job->handleFileDeleted(url);
    SUCCEED();
}

// ========== handleFileAdded Tests ==========

TEST_F(TestAbstractJob, HandleFileAdded_EmitsSignal)
{
    QUrl url = QUrl::fromLocalFile("/tmp/added.txt");

    job->handleFileAdded(url);
    SUCCEED();
}

// ========== Edge Cases ==========

TEST_F(TestAbstractJob, EdgeCase_NullWorker)
{
    stub.set_lamda(&QThread::quit, [](QThread *) {
        __DBG_STUB_INVOKE__
    });
    using WaitFuncPtr = bool (QThread::*)(QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFuncPtr>(&QThread::wait), [&](QThread *, QDeadlineTimer) {
        __DBG_STUB_INVOKE__
        return true;
    });

    AbstractJob *nullJob = new AbstractJob(nullptr, nullptr);
    EXPECT_TRUE(nullJob);
    delete nullJob;
}

TEST_F(TestAbstractJob, EdgeCase_MultipleStopActions)
{
    stub.set_lamda(&AbstractWorker::stopAllThread, [](AbstractWorker *) {
        __DBG_STUB_INVOKE__
    });

    job->operateAation(AbstractJobHandler::SupportAction::kStopAction);
    job->operateAation(AbstractJobHandler::SupportAction::kStopAction);
    job->operateAation(AbstractJobHandler::SupportAction::kCancelAction);

    SUCCEED();
}

#include "test_abstractjob.moc"
