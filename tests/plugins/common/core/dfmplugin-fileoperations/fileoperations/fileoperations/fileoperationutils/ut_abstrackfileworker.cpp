// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/abstractworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_AbstractWorker : public testing::Test
{
public:
    void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }
    void TearDown() override {}
};

// 超长UT，崩溃，卡顿，需要整改， todo： liyigang
TEST_F(UT_AbstractWorker, testSetWorkArgs)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    AbstractWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_NO_FATAL_FAILURE( worker.setWorkArgs(nullptr, {}, QUrl(),AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_EQ(QUrl(), worker.targetUrl);
    JobHandlePointer handl(new AbstractJobHandler);
    EXPECT_NO_FATAL_FAILURE(worker.setWorkArgs(handl, {}, url,AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_EQ(url, worker.targetUrl);
    worker.initHandleConnects(nullptr);
}

TEST_F(UT_AbstractWorker, testGetAction)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    AbstractWorker worker;
    worker.workData.reset(new WorkerData);
    worker.getAction(AbstractJobHandler::SupportAction::kNoAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kNoAction);
    worker.getAction(AbstractJobHandler::SupportAction::kCancelAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kCancelAction);
    worker.getAction(AbstractJobHandler::SupportAction::kCoexistAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kCoexistAction);
    worker.getAction(AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kSkipAction);
    worker.getAction(AbstractJobHandler::SupportAction::kMergeAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kMergeAction);
    worker.getAction(AbstractJobHandler::SupportAction::kReplaceAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kReplaceAction);
    worker.getAction(AbstractJobHandler::SupportAction::kRetryAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kRetryAction);
    worker.getAction(AbstractJobHandler::SupportAction::kEnforceAction);
    EXPECT_TRUE(worker.currentAction == AbstractJobHandler::SupportAction::kEnforceAction);
}

TEST_F(UT_AbstractWorker, testResumeAndPause)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&QWaitCondition::wakeAll, []{ __DBG_STUB_INVOKE__ });
    AbstractWorker worker;
    worker.resume();
    EXPECT_TRUE(worker.currentState == AbstractJobHandler::JobState::kRunningState);

    EXPECT_TRUE(worker.initArgs());

    worker.pause();
    worker.pause();
    EXPECT_TRUE(worker.currentState == AbstractJobHandler::JobState::kPauseState);
}

TEST_F(UT_AbstractWorker, testStatisticsFilesSize)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&QWaitCondition::wakeAll, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&QThread::start, []{});
    AbstractWorker worker;

    worker.startCountProccess();
    EXPECT_TRUE(worker.updateProgressTimer);
    EXPECT_TRUE(worker.updateProgressThread);

    worker.workData.reset(new WorkerData);
    EXPECT_FALSE(worker.statisticsFilesSize());

    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    worker.sourceUrls = {url};
    EXPECT_TRUE(worker.statisticsFilesSize());

    stub.set_lamda(&FileOperationsUtils::isFileOnDisk, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.statisticsFilesSize());
    worker.onStatisticsFilesSizeFinish();
    worker.currentState = AbstractJobHandler::JobState::kStartState;
    EXPECT_TRUE(worker.stateCheck());

    worker.workData->signalThread = false;
    worker.retry = true;
    worker.checkRetry();
}

TEST_F(UT_AbstractWorker, testsCurrentTaskNotifyAndStop)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&QWaitCondition::wakeAll, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&QThread::start, []{});
    AbstractWorker worker;
    worker.workData.reset(new WorkerData);

    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    worker.emitCurrentTaskNotify(url, url);
    EXPECT_FALSE(worker.isStopped());

    for (int i = 0; i < worker.threadCount; i++) {
        QSharedPointer<DoCopyFileWorker> copy(new DoCopyFileWorker(worker.workData));
        worker.threadCopyWorker.append(copy);
    }
    stub.set_lamda(&DoCopyFileWorker::pause, []{ __DBG_STUB_INVOKE__ });
    worker.copyOtherFileWorker.reset(new DoCopyFileWorker(worker.workData));
    worker.pauseAllThread();


    worker.resumeAllThread();
    worker.resumeThread({quintptr(&worker)});
    worker.checkRetry();

    EXPECT_FALSE(worker.doWork());

    worker.stopAllThread();
    EXPECT_FALSE(worker.stateCheck());
    worker.onStatisticsFilesSizeUpdate(100);
    stub.set_lamda(VADDR(AbstractWorker, initArgs), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());
    stub.set_lamda(VADDR(AbstractWorker, initArgs), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(AbstractWorker,statisticsFilesSize), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doWork());
}

TEST_F(UT_AbstractWorker, testsformatFileName)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&QWaitCondition::wakeAll, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&QThread::start, []{});
    AbstractWorker worker;
    worker.workData.reset(new WorkerData);

    worker.workData->jobFlags |= AbstractJobHandler::JobFlag::kDontFormatFileName;
    EXPECT_EQ("testeeee", worker.formatFileName("testeeee"));

    worker.workData->jobFlags = AbstractJobHandler::JobFlag::kNoHint;
    EXPECT_EQ("testeeee", worker.formatFileName("testeeee"));
    stub.set_lamda(&QStorageInfo::fileSystemType, []{ __DBG_STUB_INVOKE__ return QByteArray("vfat");});
    EXPECT_EQ("testee_ee", worker.formatFileName("testee:ee"));
}

TEST_F(UT_AbstractWorker, testSaveOperations)
{
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, QMap<QString,QVariant>);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] { __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&QWaitCondition::wakeAll, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&QThread::start, []{});
    AbstractWorker worker;

    worker.workData.reset(new WorkerData);

    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    worker.isConvert = false;
    worker.completeTargetFiles.append(url);
    worker.completeSourceFiles.append(url);
    worker.jobType = AbstractJobHandler::JobType::kCopyType;
    worker.saveOperations();
    worker.sourceUrls.append(QUrl());
    stub.set_lamda(&FileUtils::isTrashFile, []{ __DBG_STUB_INVOKE__ return true;});
    worker.jobType = AbstractJobHandler::JobType::kCutType;
    worker.saveOperations();
    worker.jobType = AbstractJobHandler::JobType::kMoveToTrashType;
    worker.saveOperations();
    worker.jobType = AbstractJobHandler::JobType::kRestoreType;
    worker.saveOperations();
    worker.jobType = AbstractJobHandler::JobType::kDeleteType;
    worker.saveOperations();
    worker.jobType = AbstractJobHandler::JobType::kUnknow;
    worker.saveOperations();
    worker.jobType = AbstractJobHandler::JobType::kDeleteType;

    worker.doOperateWork(AbstractJobHandler::SupportAction::kStopAction);
    EXPECT_EQ(AbstractJobHandler::JobState::kStopState, worker.currentState);
    worker.copyOtherFileWorker.reset(new DoCopyFileWorker(worker.workData));
    stub.set_lamda(&DoCopyFileWorker::operateAction, []{ __DBG_STUB_INVOKE__ });
    worker.doOperateWork(AbstractJobHandler::SupportAction::kRememberAction, AbstractJobHandler::JobErrorType::kOpenError, quintptr(&worker));
    EXPECT_EQ(worker.workData->errorOfAction.keys().first(), AbstractJobHandler::JobErrorType::kOpenError);

    worker.workData->signalThread = false;
    worker.doOperateWork(AbstractJobHandler::SupportAction::kRememberAction, AbstractJobHandler::JobErrorType::kOpenError,
                         quintptr(&worker));
    EXPECT_EQ(worker.workData->errorOfAction.keys().first(), AbstractJobHandler::JobErrorType::kOpenError);

    worker.threadCopyWorker.append(worker.copyOtherFileWorker);
    worker.doOperateWork(AbstractJobHandler::SupportAction::kRememberAction, AbstractJobHandler::JobErrorType::kOpenError, quintptr(worker.copyOtherFileWorker.data()));
    EXPECT_EQ(worker.workData->errorOfAction.keys().first(), AbstractJobHandler::JobErrorType::kOpenError);

    stub.set_lamda(&AbstractWorker::pauseAllThread, []{ __DBG_STUB_INVOKE__ });
    worker.doOperateWork(AbstractJobHandler::SupportAction::kPauseAction);
    stub.set_lamda(&AbstractWorker::resumeAllThread, []{ __DBG_STUB_INVOKE__ });
    worker.doOperateWork(AbstractJobHandler::SupportAction::kResumAction);


    worker.emitErrorNotify(url, url, AbstractJobHandler::JobErrorType::kOpenError);
    worker.statisticsFilesSizeJob.reset(new DFMBASE_NAMESPACE::FileStatisticsJob());
    stub.set_lamda(&DFMBASE_NAMESPACE::FileStatisticsJob::stop, []{ __DBG_STUB_INVOKE__ });
    worker.updateProgressThread.reset(new QThread);
    stub.set_lamda(&QThread::quit, []{ __DBG_STUB_INVOKE__ });
    worker.updateProgressTimer.reset(new UpdateProgressTimer);
    worker.stop();

    worker.onUpdateProgress();
}
