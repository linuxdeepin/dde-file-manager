// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
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
class UT_FileOperationsService : public testing::Test
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
    ~UT_FileOperationsService() override;
};

UT_FileOperationsService::~UT_FileOperationsService(){

}

TEST_F(UT_FileOperationsService, testFileOperationsService)
{
    FileOperationsService jobService;
    auto handle = jobService.copy({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
    handle = jobService.copyFromTrash({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
    handle = jobService.moveToTrash({}, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
    handle = jobService.restoreFromTrash({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
    handle = jobService.deletes({}, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
    handle = jobService.cut({}, QUrl(), AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
    handle = jobService.cleanTrash({});
    EXPECT_TRUE(!handle.isNull());
    handle->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);

}
