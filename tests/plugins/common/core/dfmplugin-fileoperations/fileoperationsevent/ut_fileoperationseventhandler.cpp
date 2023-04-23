// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/fileoperationseventhandler.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/event/event.h>

#include <QDir>
#include <QProcess>
#include <QVariant>
#include <QMap>

#include <gtest/gtest.h>

Q_DECLARE_METATYPE(QList<QUrl> *);
Q_DECLARE_METATYPE(const QList<QUrl> *);
typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);
Q_DECLARE_METATYPE(QString *);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_FileOperationsEventHandler : public testing::Test
{
public:
    void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }
    void TearDown() override {}
    ~UT_FileOperationsEventHandler() override;
};

UT_FileOperationsEventHandler::~UT_FileOperationsEventHandler(){

}

TEST_F(UT_FileOperationsEventHandler, testFileOperationsEventHandler)
{
    auto op = FileOperationsEventHandler::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());

    EXPECT_NO_FATAL_FAILURE(op->handleJobResult(AbstractJobHandler::JobType::kUnknow, nullptr));
    JobHandlePointer job(new AbstractJobHandler());
    EXPECT_NO_FATAL_FAILURE(op->handleJobResult(AbstractJobHandler::JobType::kCopyType, nullptr));
    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, QList<QUrl> *&, bool &,const QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    typedef bool (EventDispatcherManager::*PublishFun1)(dpf::EventType, quint64, QList<QUrl> *&,const QList<QUrl> &, bool &,const QString &);
    auto publishFun1 = static_cast<PublishFun1>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun1, [] {return true;});
    typedef bool (EventDispatcherManager::*PublishFun2)(dpf::EventType, quint64, const QList<QUrl> *&, bool &,const QString &);
    auto publishFun2 = static_cast<PublishFun2>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun2, [] {return true;});
    typedef bool (EventDispatcherManager::*PublishFun3)(dpf::EventType, quint64, QList<QUrl> *&,const QList<QUrl> &,const QList<QVariant> &, bool &,const QString &);
    auto publishFun3 = static_cast<PublishFun3>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun3, [] {return true;});
    op->publishJobResultEvent(AbstractJobHandler::JobType::kCopyType, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");
    op->publishJobResultEvent(AbstractJobHandler::JobType::kCutType, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");
    op->publishJobResultEvent(AbstractJobHandler::JobType::kDeleteType, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");
    op->publishJobResultEvent(AbstractJobHandler::JobType::kMoveToTrashType, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");
    op->publishJobResultEvent(AbstractJobHandler::JobType::kRestoreType, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");
    op->publishJobResultEvent(AbstractJobHandler::JobType::kCleanTrashType, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");
    op->publishJobResultEvent(AbstractJobHandler::JobType::kUnknow, QList<QUrl>(), QList<QUrl>(), QVariantList(), false, "");

    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(AbstractJobHandler::JobType::kCopyType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(QList<QUrl>()));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey, QVariant::fromValue(QList<QUrl>()));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteCustomInfosKey, QVariant::fromValue(QList<QUrl>()));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(nullptr));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey, QVariant::fromValue(AbstractJobHandler::JobErrorType::kNoSourceError));
    op->handleErrorNotify(info);
    op->handleFinishedNotify(info);

    op->disconnect(job.data());
}
