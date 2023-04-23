// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/filecopymovejob.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationsservice.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/application/application.h>

#include <dfm-framework/event/event.h>

#include <QDir>
#include <QProcess>
#include <QVariant>
#include <QMap>
#include <QTimer>

#include <gtest/gtest.h>

Q_DECLARE_METATYPE(QList<QUrl> *);
typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(bool *);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_FileCopyMoveJob : public testing::Test
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
    ~UT_FileCopyMoveJob() override;
};

UT_FileCopyMoveJob::~UT_FileCopyMoveJob(){

}

TEST_F(UT_FileCopyMoveJob, testFileCopyMoveJob)
{
    FileCopyMoveJob job;
    EXPECT_TRUE(job.getOperationsAndDialogService());

    JobHandlePointer jobHandle(new AbstractJobHandler);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&AbstractJobHandler::start,[]{});
    typedef void (QTimer::*SartFun)();
    stub.set_lamda(static_cast<SartFun>(&QTimer::start),[]{});
    EXPECT_NO_FATAL_FAILURE( job.initArguments(jobHandle));
    jobHandle.reset();

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,[]{return false;});
    stub.set_lamda(&FileCopyMoveJob::initArguments,[]{});
    EXPECT_TRUE(job.copy({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());
    EXPECT_TRUE(job.copyFromTrash({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());
    EXPECT_TRUE(job.moveToTrash({}, AbstractJobHandler::JobFlag::kNoHint).isNull());
    EXPECT_TRUE(job.restoreFromTrash({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());
    EXPECT_TRUE(job.deletes({}, AbstractJobHandler::JobFlag::kNoHint).isNull());
    EXPECT_TRUE(job.cut({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());
    EXPECT_TRUE(job.cleanTrash({}).isNull());

    QObject testSend;
    stub.set_lamda(&QObject::sender, [&]{return &testSend;});
    EXPECT_NO_FATAL_FAILURE(job.onHandleAddTask());

    stub.set_lamda(&DialogManager::addTask, [&]{});
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(jobHandle));
    EXPECT_NO_FATAL_FAILURE(job.onHandleAddTaskWithArgs(info));

    stub.set_lamda(&FileCopyMoveJob::getOperationsAndDialogService,[]{return true;});
    stub.set_lamda(&FileOperationsService::copy, []{return nullptr;});
    EXPECT_TRUE(job.copy({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());

    stub.set_lamda(&FileOperationsService::copyFromTrash, []{return nullptr;});
    EXPECT_TRUE(job.copyFromTrash({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());

    stub.set_lamda(&FileOperationsService::moveToTrash, []{return nullptr;});
    EXPECT_TRUE(job.moveToTrash({}, AbstractJobHandler::JobFlag::kNoHint).isNull());

    stub.set_lamda(&FileOperationsService::restoreFromTrash, []{return nullptr;});
    EXPECT_TRUE(job.restoreFromTrash({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());

    stub.set_lamda(&FileOperationsService::deletes, []{return nullptr;});
    EXPECT_TRUE(job.deletes({}, AbstractJobHandler::JobFlag::kNoHint).isNull());

    stub.set_lamda(&FileOperationsService::cut, []{return nullptr;});
    EXPECT_TRUE(job.cut({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint).isNull());

    stub.set_lamda(&FileOperationsService::cleanTrash, []{return nullptr;});
    EXPECT_TRUE(job.cleanTrash({}).isNull());


    EXPECT_NO_FATAL_FAILURE(job.onHandleAddTask());

    EXPECT_NO_FATAL_FAILURE(job.onHandleAddTaskWithArgs(info));
    EXPECT_NO_FATAL_FAILURE(job.onHandleTaskFinished(info));
}
