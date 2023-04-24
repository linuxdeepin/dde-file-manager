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

    job.operateWork(AbstractJobHandler::SupportAction::kStartAction);

    JobInfoPointer info(new QMap<quint8, QVariant>);
    job.errorQueue.enqueue(info);

    job.operateWork(AbstractJobHandler::SupportAction::kSkipAction);
    info->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, quintptr(job.doWorker.data()));
    job.errorQueue.enqueue(info);
    job.handleError(info);
    EXPECT_EQ(job.errorQueue.size(), 2);

    job.handleRetryErrorSuccess(quintptr(job.doWorker.data()));
    EXPECT_EQ(job.errorQueue.size(), 1);
    job.handleRetryErrorSuccess(0);
    EXPECT_EQ(job.errorQueue.size(), 1);

    job.handleRetryErrorSuccess(quintptr(job.doWorker.data()));
    EXPECT_EQ(job.errorQueue.size(), 0);
}
