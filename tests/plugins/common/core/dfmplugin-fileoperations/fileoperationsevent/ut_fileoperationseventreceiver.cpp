// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/fileoperationseventreceiver.h"
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
typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);
Q_DECLARE_METATYPE(QString *);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_FileOperationsEventReceiver : public testing::Test
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
    ~UT_FileOperationsEventReceiver() override;
};

UT_FileOperationsEventReceiver::~UT_FileOperationsEventReceiver(){

}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationOpenFiles)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch ./testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    EXPECT_FALSE(op->handleOperationOpenFiles(0, {}));
    stub_ext::StubExt stub;
    typedef bool (LocalFileHandler::*openFils)(const QList<QUrl> &);
    auto openFunc = static_cast<openFils>(&LocalFileHandler::openFiles);
    stub.set_lamda(openFunc, [] {return true;});
    EXPECT_TRUE(op->handleOperationOpenFiles(0, {url}));

    stub.clear();
    stub.set_lamda(openFunc, [] {return false;});
    stub.set_lamda(&LocalFileHandler::lastEventType,[](){return GlobalEventType::kUnknowType;});
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QList<QUrl> *&);
    auto pushFunc = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(pushFunc, [] {return false;});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    EXPECT_TRUE(op->handleOperationOpenFiles(0, {url}));


    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    QUrl urltmp("smb://10.8.11.1");
    EXPECT_TRUE(op->handleOperationOpenFiles(0, {urltmp}));
    bool ok = false;
    EXPECT_FALSE(op->handleOperationOpenFiles(0, {}, &ok));
    EXPECT_TRUE(op->handleOperationOpenFiles(0, {urltmp}, &ok));
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationOpenFiles(0, {urltmp}, QVariant(), callback));


    QProcess::execute("rm ./testSyncFileInfo.txt");
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationOpenFilesByApp)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());

    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showErrorDialog,[]{});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    EXPECT_FALSE(op->handleOperationOpenFilesByApp(0,{}, {"dde-file-manager"}));

    stub.set_lamda(&LocalFileHandler::openFilesByApp, [] {return true;});
    EXPECT_TRUE(op->handleOperationOpenFilesByApp(0, {url},{"dde-file-manager"}));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    QUrl urltmp("smb://10.8.11.1");
    EXPECT_TRUE(op->handleOperationOpenFilesByApp(0, {urltmp}, {"dde-file-manager"}));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationOpenFilesByApp(0, {urltmp}, {"dde-file-manager"}, QVariant(), callback));
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationRenameFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showRenameNameSameErrorDialog,[]{ return 0; });
    stub.set_lamda(&DialogManager::showRenameBusyErrDialog,[]{});
    stub.set_lamda(&DialogManager::showErrorDialog,[]{});
    stub.set_lamda(&LocalFileHandler::renameFilesBatch, []{return false;});
    EXPECT_FALSE(op->handleOperationRenameFile(0,url, url, AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(op->handleOperationRenameFiles(0,{url}, QPair<QString, QString>(), false));
    EXPECT_FALSE(op->handleOperationRenameFiles(0,{url}, QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag>()));
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationRenameFiles(0,{url}, QPair<QString, QString>(), false, QVariant(), callback));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationRenameFiles(0,{url}, QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag>(), QVariant(), callback));

    stub.set_lamda(&LocalFileHandler::renameFile, []{return false;});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});

    qRegisterMetaType<QMap<QString,QVariant>*>();

    typedef bool (EventDispatcherManager::*PublishFun1)(dpf::EventType, quint64,  QMap<QString,QVariant> *&);
    auto publishFun1 = static_cast<PublishFun1>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun1, [] {return true;});
    EXPECT_FALSE(op->handleOperationRenameFile(0,url, QUrl::fromLocalFile(url.path() + "test"), AbstractJobHandler::JobFlag::kNoHint));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, const QUrl &, const AbstractJobHandler::JobFlags &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    EXPECT_TRUE(op->handleOperationRenameFile(0,QUrl("smb://10.8.11.1"), QUrl::fromLocalFile(url.path() + "test"), AbstractJobHandler::JobFlag::kNoHint));
    typedef bool (EventSequenceManager::*RunFunc1)(const QString &, const QString &, quint64, const QList<QUrl> &, const QPair<QString, QString> &, const bool &);
    auto runFunc1 = static_cast<RunFunc1>(&EventSequenceManager::run);
    stub.set_lamda(runFunc1, [] {return true;});
    EXPECT_TRUE(op->handleOperationRenameFiles(0,{QUrl("smb://10.8.11.1")}, QPair<QString, QString>(), false));
    typedef bool (EventSequenceManager::*RunFunc2)(const QString &, const QString &, quint64, const QList<QUrl> &, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> &);
    auto runFunc2 = static_cast<RunFunc2>(&EventSequenceManager::run);
    stub.set_lamda(runFunc2, [] {return true;});
    EXPECT_TRUE(op->handleOperationRenameFiles(0,{QUrl("smb://10.8.11.1")}, QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag>()));

    stub.set_lamda(&FileUtils::isDesktopFile, []{return true;});
    EXPECT_FALSE(op->handleOperationRenameFile(0,url, QUrl::fromLocalFile(url.path() + "test"), AbstractJobHandler::JobFlag::kNoHint));

    EXPECT_NO_FATAL_FAILURE(op->handleOperationRenameFile(0,url, QUrl::fromLocalFile(url.path() + "test"), AbstractJobHandler::JobFlag::kNoHint, QVariant(), callback));
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationMkdir)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_FALSE(op->handleOperationMkdir(0, QUrl()));

    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showErrorDialog,[]{});
    stub.set_lamda(&LocalFileHandler::mkdir, []{return false;});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    EXPECT_FALSE(op->handleOperationMkdir(0, url));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationMkdir(0, url, QVariant(), callback));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, QUrl &,const QVariant &, AbstractJobHandler::OperatorCallback &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    stub.set_lamda(&FileUtils::isLocalFile,[]{return false;});
    EXPECT_TRUE(op->handleOperationMkdir(0, url));
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationTouchFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(op->handleOperationTouchFile(0, QUrl(), Global::CreateFileType::kCreateFileTypeText, QString()).isEmpty());
    EXPECT_TRUE(op->handleOperationTouchFile(0, QUrl(), url, QString()).isEmpty());
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationTouchFile(0, QUrl(), Global::CreateFileType::kCreateFileTypeText, QString(), QVariant(), callback));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationTouchFile(0, QUrl(), url, QString(), QVariant(), callback));

    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showErrorDialog,[]{});
    stub.set_lamda(&LocalFileHandler::touchFile, []{return false;});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    EXPECT_TRUE(op->handleOperationTouchFile(0, url, Global::CreateFileType::kCreateFileTypeText, QString()).isEmpty());
    EXPECT_TRUE(op->handleOperationTouchFile(0, url, QUrl(), QString()).isEmpty());

    stub.set_lamda(&FileUtils::isLocalFile,[]{ return false; });
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, QUrl &,
                                                  const QUrl & ,const QString &, const QVariant &, AbstractJobHandler::OperatorCallback &,
                                                  QString *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    EXPECT_TRUE(op->handleOperationTouchFile(0, url, Global::CreateFileType::kCreateFileTypeText, QString()).isEmpty());
    EXPECT_TRUE(op->handleOperationTouchFile(0, url, QUrl(), QString()).isEmpty());
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationLinkFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showErrorDialog,[]{});
    stub.set_lamda(&LocalFileHandler::createSystemLink, []{return false;});
    stub.set_lamda(&LocalFileHandler::deleteFile, []{return false;});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    EXPECT_FALSE(op->handleOperationLinkFile(0, url, url, true, true));
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationLinkFile(0, url, url, true, true, QVariant(), callback));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, const QUrl &,
                                                  const bool & ,const bool &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    stub.set_lamda(&FileUtils::isLocalFile,[]{return false;});
    EXPECT_TRUE(op->handleOperationLinkFile(0, url, url, true, true));
}
