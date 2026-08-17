// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QTimer>
#include <QSignalSpy>

#include "stubext.h"

#include "fileoperations/filecopymovejob.h"
#include "fileoperations/fileoperationsservice.h"
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestFileCopyMoveJob : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        job = new FileCopyMoveJob();
        ASSERT_TRUE(job);
    }

    void TearDown() override
    {
        stub.clear();
        if (job) {
            delete job;
            job = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    FileCopyMoveJob *job;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestFileCopyMoveJob, Constructor_InitializesMutex)
{
    EXPECT_TRUE(job);
}

TEST_F(TestFileCopyMoveJob, Destructor_Cleanup)
{
    delete job;
    job = nullptr;
    SUCCEED();
}

// ========== getOperationsAndDialogService Tests ==========

TEST_F(TestFileCopyMoveJob, GetOperationsAndDialogService_Success)
{
    stub.set_lamda(&DialogManager::instance, []() -> DialogManager * {
        __DBG_STUB_INVOKE__
        static DialogManager mgr;
        return &mgr;
    });

    bool result = job->getOperationsAndDialogService();
    EXPECT_TRUE(result);
}

// ========== copy Tests ==========

TEST_F(TestFileCopyMoveJob, Copy_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test1.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::copy,
                   [](FileOperationsService *, const QList<QUrl> &,
                      const QUrl &, const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->copy(sources, target);
    EXPECT_TRUE(handle);
}

TEST_F(TestFileCopyMoveJob, Copy_ServiceFails)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/dest");

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    JobHandlePointer handle = job->copy(sources, target);
    EXPECT_FALSE(handle);
}

// ========== copyFromTrash Tests ==========

TEST_F(TestFileCopyMoveJob, CopyFromTrash_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/.Trash/file.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/restore");

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::copyFromTrash,
                   [](FileOperationsService *, const QList<QUrl> &,
                      const QUrl &, const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->copyFromTrash(sources, target);
    EXPECT_TRUE(handle);
}

// ========== moveToTrash Tests ==========

TEST_F(TestFileCopyMoveJob, MoveToTrash_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/delete_me.txt") };

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::moveToTrash,
                   [](FileOperationsService *, const QList<QUrl> &,
                      const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->moveToTrash(sources);
    EXPECT_TRUE(handle);
}

TEST_F(TestFileCopyMoveJob, MoveToTrash_WithoutInit)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/file.txt") };

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::moveToTrash,
                   [](FileOperationsService *, const QList<QUrl> &,
                      const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    JobHandlePointer handle = job->moveToTrash(sources, AbstractJobHandler::JobFlag::kNoHint, false);
    EXPECT_TRUE(handle);
}

// ========== restoreFromTrash Tests ==========

TEST_F(TestFileCopyMoveJob, RestoreFromTrash_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/.Trash/file.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/restore");

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::restoreFromTrash,
                   [](FileOperationsService *, const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->restoreFromTrash(sources, target);
    EXPECT_TRUE(handle);
}

// ========== deletes Tests ==========

TEST_F(TestFileCopyMoveJob, Deletes_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/delete.txt") };

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::deletes,
                   [](FileOperationsService *, const QList<QUrl> &,
                      const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->deletes(sources);
    EXPECT_TRUE(handle);
}

// ========== cut Tests ==========

TEST_F(TestFileCopyMoveJob, Cut_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/cut_source.txt") };
    QUrl target = QUrl::fromLocalFile("/tmp/cut_dest");

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::cut,
                   [](FileOperationsService *, const QList<QUrl> &, const QUrl &,
                      const AbstractJobHandler::JobFlags &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->cut(sources, target);
    EXPECT_TRUE(handle);
}

// ========== cleanTrash Tests ==========

TEST_F(TestFileCopyMoveJob, CleanTrash_Success)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/.Trash") };

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperationsService::cleanTrash,
                   [](FileOperationsService *, const QList<QUrl> &) -> JobHandlePointer {
                       __DBG_STUB_INVOKE__
                       return JobHandlePointer(new AbstractJobHandler);
                   });

    stub.set_lamda(&FileCopyMoveJob::initArguments,
                   [](FileCopyMoveJob *, const JobHandlePointer,
                      const AbstractJobHandler::JobFlags) {
                       __DBG_STUB_INVOKE__
                   });

    JobHandlePointer handle = job->cleanTrash(sources);
    EXPECT_TRUE(handle);
}

// ========== initArguments Tests ==========

TEST_F(TestFileCopyMoveJob, InitArguments_RemoteFlag)
{
    JobHandlePointer handler(new AbstractJobHandler);

    stub.set_lamda(&FileCopyMoveJob::startAddTaskTimer,
                   [](FileCopyMoveJob *, const JobHandlePointer, const bool isRemote) {
                       __DBG_STUB_INVOKE__
                       EXPECT_TRUE(isRemote);
                   });

    stub.set_lamda(VADDR(AbstractJobHandler, start),
                   [](AbstractJobHandler *) {
                       __DBG_STUB_INVOKE__
                   });

    job->initArguments(handler, AbstractJobHandler::JobFlag::kCopyRemote);
}

TEST_F(TestFileCopyMoveJob, InitArguments_LocalFlag)
{
    JobHandlePointer handler(new AbstractJobHandler);

    stub.set_lamda(&FileCopyMoveJob::startAddTaskTimer,
                   [](FileCopyMoveJob *, const JobHandlePointer, const bool isRemote) {
                       __DBG_STUB_INVOKE__
                       EXPECT_FALSE(isRemote);
                   });

    job->initArguments(handler, AbstractJobHandler::JobFlag::kNoHint);
}

// ========== startAddTaskTimer Tests ==========

TEST_F(TestFileCopyMoveJob, StartAddTaskTimer_RemoteJob)
{
    JobHandlePointer handler(new AbstractJobHandler);

    stub.set_lamda(VADDR(AbstractJobHandler, start),
                   [](AbstractJobHandler *) {
                       __DBG_STUB_INVOKE__
                   });

    job->startAddTaskTimer(handler, true);
    SUCCEED();
}

TEST_F(TestFileCopyMoveJob, StartAddTaskTimer_LocalJob)
{
    JobHandlePointer handler(new AbstractJobHandler);

    stub.set_lamda(VADDR(AbstractJobHandler, start),
                   [](AbstractJobHandler *) {
                       __DBG_STUB_INVOKE__
                   });

    job->startAddTaskTimer(handler, false);
    SUCCEED();
}

// ========== onHandleAddTask Tests ==========

TEST_F(TestFileCopyMoveJob, OnHandleAddTask_Success)
{
    JobHandlePointer handler(new AbstractJobHandler);

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&DialogManager::addTask,
                   [](DialogManager *, const JobHandlePointer) {
                       __DBG_STUB_INVOKE__
                   });

    QTimer timer;
    timer.setProperty("jobPointer", QVariant::fromValue(handler));
    QObject::connect(&timer, &QTimer::timeout, job, &FileCopyMoveJob::onHandleAddTask);

    timer.start(10);

    SUCCEED();
}

// ========== onHandleAddTaskWithArgs Tests ==========

TEST_F(TestFileCopyMoveJob, OnHandleAddTaskWithArgs_Success)
{
    JobHandlePointer handler(new AbstractJobHandler);
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handler));

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,
                   [](FileCopyMoveJob *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&DialogManager::addTask,
                   [](DialogManager *, const JobHandlePointer) {
                       __DBG_STUB_INVOKE__
                   });

    job->onHandleAddTaskWithArgs(info);
    SUCCEED();
}

// ========== onHandleTaskFinished Tests ==========

TEST_F(TestFileCopyMoveJob, OnHandleTaskFinished_RemovesTask)
{
    JobHandlePointer handler(new AbstractJobHandler);
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handler));

    job->onHandleTaskFinished(info);
    SUCCEED();
}
