// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/abstractjob.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/abstractworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <gtest/gtest.h>


DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_AbstractJob : public testing::Test
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

TEST_F(UT_AbstractJob, testAbstractJob)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&QThread::start, []{ __DBG_STUB_INVOKE__ });
    AbstractJob job(new AbstractWorker());
    job.setJobArgs(nullptr, {});
    JobHandlePointer handle(new AbstractJobHandler);
    job.setJobArgs(handle, {});

    job.operateAation(AbstractJobHandler::SupportAction::kStartAction);
    job.operateAation(AbstractJobHandler::SupportAction::kResumAction);
    job.operateAation(AbstractJobHandler::SupportAction::kCancelAction);

    JobInfoPointer info(new QMap<quint8, QVariant>);
    job.errorQueue.enqueue(info);
    job.errorQueue.enqueue(info);

    job.operateAation(AbstractJobHandler::SupportAction::kSkipAction);

    job.errorQueue.enqueue(info);
    job.errorQueue.enqueue(info);

    job.operateAation(AbstractJobHandler::SupportAction::kRetryAction);
    info->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, quintptr(job.doWorker.data()));

    job.handleError(info);
    job.errorQueue.enqueue(info);
    JobInfoPointer info1(new QMap<quint8, QVariant>);
    info1->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, quintptr(job.doWorker.data()));
    info1->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey, QVariant::fromValue(AbstractJobHandler::JobErrorType::kReadError));
    job.handleError(info);
    job.handleError(info1);

    JobInfoPointer info2(new QMap<quint8, QVariant>);
    info2->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, quintptr(job.doWorker.data() - 1));
    info2->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey, QVariant::fromValue(AbstractJobHandler::JobErrorType::kReadError));
    job.handleError(info2);

    job.errorQueue.clear();
    job.handleError(info2);
    job.errorQueue.clear();
    job.errorQueue.enqueue(info);
    job.errorQueue.enqueue(info);

    EXPECT_EQ(job.errorQueue.size(), 2);

    job.handleRetryErrorSuccess(quintptr(job.doWorker.data()));
    EXPECT_EQ(job.errorQueue.size(), 1);
    job.handleRetryErrorSuccess(0);
    EXPECT_EQ(job.errorQueue.size(), 1);

    job.handleRetryErrorSuccess(quintptr(job.doWorker.data()));
    EXPECT_EQ(job.errorQueue.size(), 0);
}
