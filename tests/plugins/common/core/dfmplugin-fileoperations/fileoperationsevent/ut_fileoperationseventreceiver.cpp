// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/fileoperationseventreceiver.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/trashfileeventreceiver.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/systempathutil.h>

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
Q_DECLARE_METATYPE(bool *);

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
TEST_F(UT_FileOperationsEventReceiver, testHandleOperationCopy)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventReceiver::doCopyFile, []{ return nullptr; });
    stub.set_lamda(&FileOperationsEventReceiver::doCutFile, []{ return nullptr; });
    stub.set_lamda(&FileOperationsEventReceiver::doDeleteFile, []{ return nullptr; });
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCopy(0,{QUrl()}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCut(0,{QUrl()}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationDeletes(0,{QUrl()}, AbstractJobHandler::JobFlag::kNoHint, handle));

    EXPECT_NO_FATAL_FAILURE(op->handleOperationCopy(0,{QUrl()}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle, QVariant(), callback));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCut(0,{QUrl()}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle, QVariant(), callback));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationDeletes(0,{QUrl()}, AbstractJobHandler::JobFlag::kNoHint, handle, QVariant(), callback));
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
    stub.set_lamda(&LocalFileHandler::touchFile, []{return QUrl();});
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

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationSetPermission)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showErrorDialog,[]{});
    stub.set_lamda(&LocalFileHandler::setPermissions, []{return false;});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return true;});
    EXPECT_FALSE(op->handleOperationSetPermission(0, url, QFileDevice::ReadUser));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationSetPermission(0, url, QFileDevice::ReadUser, QVariant(), callback));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, const QFileDevice::Permissions &,
                                                  bool *&& , QString *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    stub.set_lamda(&FileUtils::isLocalFile,[]{return false;});
    EXPECT_FALSE(op->handleOperationSetPermission(0, url, QFileDevice::ReadUser));
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationWriteToClipboard)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_FALSE(op->handleOperationWriteDataToClipboard(0, nullptr));
    stub_ext::StubExt stub;
    stub.set_lamda(&ClipBoard::setUrlsToClipboard, []{});
    stub.set_lamda(&ClipBoard::setDataToClipboard, []{});
    EXPECT_TRUE(op->handleOperationWriteToClipboard(0,ClipBoard::ClipboardAction::kCopyAction, {url}));
    QMimeData data;
    EXPECT_TRUE(op->handleOperationWriteDataToClipboard(0, &data));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const ClipBoard::ClipboardAction &, const QList<QUrl> &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    stub.set_lamda(&FileUtils::isLocalFile,[]{return false;});
    EXPECT_TRUE(op->handleOperationWriteToClipboard(0,ClipBoard::ClipboardAction::kCopyAction, {url}));
}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationOpenInTerminal)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    typedef bool (*StartDetached)(const QString &);
    stub.set_lamda(static_cast<StartDetached>(QProcess::startDetached), [] { __DBG_STUB_INVOKE__ return false; });
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &,QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return false;});
    EXPECT_FALSE(op->handleOperationOpenInTerminal(0, {url}));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    stub.set_lamda(&FileUtils::isLocalFile,[]{return false;});
    typedef bool (EventDispatcherManager::*PublishFun1)(dpf::EventType, quint64, const QList<QUrl> &, bool &&,QString &);
    auto publishFun1 = static_cast<PublishFun1>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun1, [] {return false;});
    EXPECT_TRUE(op->handleOperationOpenInTerminal(0, {url}));

    EXPECT_NO_FATAL_FAILURE(op->handleOperationSaveOperations(QVariantMap()));
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCleanSaveOperationsStack());
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationRevocation(0, handle));

}

TEST_F(UT_FileOperationsEventReceiver, testHandleOperationHideFiles)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch ./testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    stub_ext::StubExt stub;
    stub.set_lamda(FileUtils::notifyFileChangeManual, []{});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, const QList<QUrl> &, bool &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return false;});
    EXPECT_TRUE(op->handleOperationHideFiles(0, {url}));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationHideFiles(0, {url}, QVariant(), callback));

    QProcess::execute("rm ./testSyncFileInfo.txt ./.hidden");
}

TEST_F(UT_FileOperationsEventReceiver, testCheckTargetUrl)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_EQ(QUrl(), op->checkTargetUrl(QUrl()));
    EXPECT_TRUE(op->checkTargetUrl(url).isValid());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::nonExistSymlinkFileName, []{return QString();});
    EXPECT_EQ(url, op->checkTargetUrl(url));
}

TEST_F(UT_FileOperationsEventReceiver, testNewDocmentName)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(!op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypeText).isEmpty());
    EXPECT_TRUE(!op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypeDefault).isEmpty());
    EXPECT_TRUE(!op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypeFolder).isEmpty());
    EXPECT_TRUE(!op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypeExcel).isEmpty());
    EXPECT_TRUE(!op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypeWord).isEmpty());
    EXPECT_TRUE(!op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypePowerpoint).isEmpty());
    EXPECT_TRUE(op->newDocmentName(url, ".tty", Global::CreateFileType::kCreateFileTypeUnknow).isEmpty());

    EXPECT_TRUE(op->newDocmentName(QUrl(), ".tty", "").isEmpty());
    EXPECT_TRUE(!op->newDocmentName(url, "testSyncFileInfo", "").isEmpty());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    QProcess::execute("touch ./testSyncFileInfo.txt");
    EXPECT_TRUE(!op->newDocmentName(url, "testSyncFileInfo", "").isEmpty());
    QProcess::execute("rm ./testSyncFileInfo.txt");
}

TEST_F(UT_FileOperationsEventReceiver, testRevocation)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    QVariantMap ret;
    ret.insert("event", GlobalEventType::kCopy);
    ret.insert("sources", {QUrl()});
    ret.insert("targets", {QUrl()});
    EXPECT_TRUE(op->revocation(0, ret, handle));

    ret.insert("sources", {url});
    ret.insert("targets", {QUrl()});
    EXPECT_TRUE(op->revocation(0, ret, handle));

    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<void (FileOperationsEventReceiver::*)(quint64, const QList<QUrl>, const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(
                       &FileOperationsEventReceiver::handleOperationCut), []{});
    ret.insert("targets", {});
    ret.insert("event", GlobalEventType::kCutFile);
    EXPECT_TRUE(op->revocation(0, ret, handle));
    ret.insert("targets", {QUrl()});
    EXPECT_TRUE(op->revocation(0, ret, handle));

    stub.set_lamda(static_cast<void (FileOperationsEventReceiver::*)(quint64, const QList<QUrl>, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(
                       &FileOperationsEventReceiver::handleOperationDeletes), []{});
    ret.insert("event", GlobalEventType::kDeleteFiles);
    EXPECT_TRUE(op->revocation(0, ret, handle));

    stub.set_lamda(static_cast<void (TrashFileEventReceiver::*)(quint64, const QList<QUrl>, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(
                       &TrashFileEventReceiver::handleOperationMoveToTrash), []{});
    ret.insert("event", GlobalEventType::kMoveToTrash);
    EXPECT_TRUE(op->revocation(0, ret, handle));

    stub.set_lamda(static_cast<void (TrashFileEventReceiver::*)(quint64, const QList<QUrl>,const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(
                       &TrashFileEventReceiver::handleOperationRestoreFromTrash), []{});
    ret.insert("event", GlobalEventType::kRestoreFromTrash);
    EXPECT_TRUE(op->revocation(0, ret, handle));

    ret.insert("targets", {});
    ret.insert("event", GlobalEventType::kRenameFile);
    EXPECT_TRUE(op->revocation(0, ret, handle));

    stub.set_lamda(static_cast<bool (FileOperationsEventReceiver::*)(const quint64, const QUrl, const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag)>(
                       &FileOperationsEventReceiver::handleOperationRenameFile), []{ return true; });
    ret.insert("targets", {QUrl()});
    EXPECT_TRUE(op->revocation(0, ret, handle));

    ret.insert("event", GlobalEventType::kRenameFiles);
    ret.insert("targets", {});
    EXPECT_TRUE(op->revocation(0, ret, handle));

    ret.insert("targets", {QUrl()});
    EXPECT_TRUE(op->revocation(0, ret, handle));
}

TEST_F(UT_FileOperationsEventReceiver, testDoRenameDesktopFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(op->doRenameDesktopFile(0, url, url, AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(op->doRenameDesktopFile(0, url, QUrl::fromLocalFile(url.path() + QDir::separator() + "festsf"), AbstractJobHandler::JobFlag::kNoHint));

    stub_ext::StubExt stub;
    stub.set_lamda(&Properties::save, []{return true;});
    EXPECT_TRUE(op->doRenameDesktopFile(0, url, QUrl::fromLocalFile(url.path() + QDir::separator() + "festsf"), AbstractJobHandler::JobFlag::kNoHint));
    op->handleOperationCleanSaveOperationsStack();
}

TEST_F(UT_FileOperationsEventReceiver, testDoCopyFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    EXPECT_TRUE(op->doCopyFile(0, {}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub_ext::StubExt stub;
    stub.set_lamda(&FileCopyMoveJob::copy, []{return nullptr;});
    EXPECT_TRUE(op->doCopyFile(0, {url}, url, AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &, const QUrl &,const AbstractJobHandler::JobFlags &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return false;});
    EXPECT_TRUE(op->doCopyFile(0, {url}, url, AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub.set_lamda(runFunc, [] {return true;});
    EXPECT_TRUE(op->doCopyFile(0, {url}, url, AbstractJobHandler::JobFlag::kNoHint, handle).isNull());
}

TEST_F(UT_FileOperationsEventReceiver, testDoCutFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    EXPECT_TRUE(op->doCutFile(0, {}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::isSameFile, []{return true;});
    EXPECT_TRUE(op->doCutFile(0, {url}, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub.clear();
    stub.set_lamda(&FileCopyMoveJob::cut, []{return nullptr;});
    EXPECT_TRUE(op->doCutFile(0, {url}, url, AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &, const QUrl &,const AbstractJobHandler::JobFlags &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return false;});
    EXPECT_TRUE(op->doCutFile(0, {url}, url, AbstractJobHandler::JobFlag::kNoHint, handle).isNull());

    stub.set_lamda(runFunc, [] {return true;});
    EXPECT_TRUE(op->doCutFile(0, {url}, url, AbstractJobHandler::JobFlag::kNoHint, handle).isNull());
}

TEST_F(UT_FileOperationsEventReceiver, testDoDeleteFile)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    FileOperationsEventReceiver::DoDeleteErrorType errtp;
    EXPECT_TRUE(op->doDeleteFile(0, {}, AbstractJobHandler::JobFlag::kNoHint, handle, true, errtp).isNull());

    stub_ext::StubExt stub;
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, []{return true;});
    stub.set_lamda(&DialogManager::showDeleteSystemPathWarnDialog, []{});
    EXPECT_TRUE(op->doDeleteFile(0, {}, AbstractJobHandler::JobFlag::kNoHint, handle, true, errtp).isNull());

    stub.clear();
    stub.set_lamda(&FileCopyMoveJob::deletes, []{return nullptr;});

    EXPECT_TRUE(op->doDeleteFile(0, {}, AbstractJobHandler::JobFlag::kNoHint, handle, true, errtp).isNull());

    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &, const AbstractJobHandler::JobFlags &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return false;});
    EXPECT_TRUE(op->doDeleteFile(0, {}, AbstractJobHandler::JobFlag::kNoHint, handle, true, errtp).isNull());

    stub.set_lamda(runFunc, [] {return true;});
    EXPECT_TRUE(op->doDeleteFile(0, {}, AbstractJobHandler::JobFlag::kNoHint, handle, true, errtp).isNull());
}

TEST_F(UT_FileOperationsEventReceiver, testDoCleanTrash)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []{return 0;});
    EXPECT_TRUE(op->doCleanTrash(0, {url}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle).isNull());

    stub.set_lamda(&DialogManager::showClearTrashDialog, []{return 0;});
    EXPECT_TRUE(op->doCleanTrash(0, {}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle).isNull());

    stub.set_lamda(&DialogManager::showClearTrashDialog, []{return 1;});
    stub.set_lamda(&FileCopyMoveJob::cleanTrash, []{return nullptr;});
    EXPECT_TRUE(op->doCleanTrash(0, {}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle).isNull());

}

TEST_F(UT_FileOperationsEventReceiver, testDoMkdir)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    stub_ext::StubExt stub;
    EXPECT_FALSE(op->doMkdir(0, QUrl(), QVariant(), callback));

    stub.set_lamda(&LocalFileHandler::mkdir, []{return false;});
    stub.set_lamda(&DialogManager::showErrorDialog, []{});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, QList<QUrl> &, bool &, QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return false;});
    EXPECT_FALSE(op->doMkdir(0, url, QVariant(), callback));

    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, QUrl &, const QVariant &,
                                                  AbstractJobHandler::OperatorCallback &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    typedef bool (EventDispatcherManager::*PublishFun1)(dpf::EventType, quint64, QList<QUrl> &, bool &&, QString &);
    auto publishFun1 = static_cast<PublishFun1>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun1, [] {return false;});
    EXPECT_TRUE(op->doMkdir(0, url, QVariant(), callback));

    op->handleOperationCleanSaveOperationsStack();

}

TEST_F(UT_FileOperationsEventReceiver, testDoTouchFilePractically)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&LocalFileHandler::touchFile, []{return QUrl();});
    stub.set_lamda(&DialogManager::showErrorDialog, []{});
    typedef bool (EventDispatcherManager::*PublishFun)(dpf::EventType, quint64, QList<QUrl> &, bool &, QString &);
    auto publishFun = static_cast<PublishFun>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun, [] {return false;});
    EXPECT_FALSE(op->doTouchFilePractically(0, url, QUrl()));
}

TEST_F(UT_FileOperationsEventReceiver, testDoTouchFilePremature)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventReceiver::doTouchFilePractically,[]{return false;});
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_TRUE(op->doTouchFilePremature(0, QUrl(), Global::CreateFileType::kCreateFileTypeText, "", QVariant(), callback).isEmpty());

    EXPECT_TRUE(op->doTouchFilePremature(0, url, Global::CreateFileType::kCreateFileTypeText, "", QVariant(), callback).isEmpty());

    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, QUrl &, const Global::CreateFileType &,
                                                  const QString &, const QVariant &,
                                                  AbstractJobHandler::OperatorCallback &, QString *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    typedef bool (EventDispatcherManager::*PublishFun1)(dpf::EventType, quint64, QList<QUrl> &, bool &&, QString &);
    auto publishFun1 = static_cast<PublishFun1>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun1, [] {return false;});
    EXPECT_TRUE(!op->doTouchFilePremature(0, url, Global::CreateFileType::kCreateFileTypeText, "", QVariant(), callback).isEmpty());

    op->handleOperationCleanSaveOperationsStack();
}

TEST_F(UT_FileOperationsEventReceiver, testDoTouchFilePremature1)
{
    auto op = FileOperationsEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventReceiver::doTouchFilePractically,[]{return false;});
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_TRUE(op->doTouchFilePremature(0, QUrl(), url, "", QVariant(), callback).isEmpty());

    EXPECT_TRUE(op->doTouchFilePremature(0, url, url, "", QVariant(), callback).isEmpty());

    stub.set_lamda(&FileUtils::isLocalFile, []{return false;});
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QUrl &, QUrl &, const QUrl &,
                                                  const QString &, const QVariant &,
                                                  AbstractJobHandler::OperatorCallback &, QString *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});
    typedef bool (EventDispatcherManager::*PublishFun1)(dpf::EventType, quint64, QList<QUrl> &, bool &&, QString &);
    auto publishFun1 = static_cast<PublishFun1>(&EventDispatcherManager::publish);
    stub.set_lamda(publishFun1, [] {return false;});
    EXPECT_TRUE(!op->doTouchFilePremature(0, url, url, "", QVariant(), callback).isEmpty());

    op->handleOperationCleanSaveOperationsStack();
}
