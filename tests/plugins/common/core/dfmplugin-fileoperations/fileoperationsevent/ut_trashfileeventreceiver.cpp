// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/fileoperationseventreceiver.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/trashfileeventreceiver.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperationsevent/fileoperationseventhandler.h"

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

#include <gtest/gtest.h>

Q_DECLARE_METATYPE(QList<QUrl> *);
typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(bool *);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_TrashFileEventReceiver : public testing::Test
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
    ~UT_TrashFileEventReceiver() override;
};

UT_TrashFileEventReceiver::~UT_TrashFileEventReceiver(){

}

TEST_F(UT_TrashFileEventReceiver, testHandleOperationMoveToTrash)
{
    auto op = TrashFileEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventHandler::handleJobResult, []{});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {}, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath, []{return true;});
    stub.set_lamda(&DialogManager::showDeleteSystemPathWarnDialog, []{});

    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle));

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, quint64, const QList<QUrl> &, const AbstractJobHandler::JobFlag &);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] {return true;});

    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(runFunc, [] {return false;});
    stub.set_lamda(&FileCopyMoveJob::deletes, []{return nullptr;});
    stub.set_lamda(&Application::genericAttribute, []{return true;});
    stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []{return 0;});

    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(&DialogManager::showNormalDeleteConfirmDialog, []{return 1;});
    stub.set_lamda(&FileCopyMoveJob::moveToTrash, []{return nullptr;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(&FileUtils::fileCanTrash, []{return false;});
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []{return 0;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []{return 1;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationMoveToTrash(0, {url}, AbstractJobHandler::JobFlag::kNoHint, handle, QVariant(), callback));
}

TEST_F(UT_TrashFileEventReceiver, testHandleOperationRestoreFromTrash)
{
    auto op = TrashFileEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventHandler::handleJobResult, []{});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationRestoreFromTrash(0, {}, url, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(&FileCopyMoveJob::restoreFromTrash, []{return nullptr;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationRestoreFromTrash(0, {}, url, AbstractJobHandler::JobFlag::kNoHint, handle));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationRestoreFromTrash(0, {}, url, AbstractJobHandler::JobFlag::kNoHint, handle, QVariant(), callback));
}

TEST_F(UT_TrashFileEventReceiver, testHandleOperationCleanTrash)
{
    auto op = TrashFileEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventHandler::handleJobResult, []{});
    op->stoped = true;
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCleanTrash(0, {}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle));

    op->stoped = false;
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []{return 0;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCleanTrash(0, {url}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle));

    stub.set_lamda(&FileCopyMoveJob::cleanTrash, []{return nullptr;});
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []{return 1;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCleanTrash(0, {url}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCleanTrash(0, {url}, handle, QVariant(), callback));
}

TEST_F(UT_TrashFileEventReceiver, testHandleOperationCopyFromTrash)
{
    auto op = TrashFileEventReceiver::instance();
    EXPECT_TRUE(op != nullptr);
    AbstractJobHandler::OperatorHandleCallback handle = [](JobHandlePointer){};
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsEventHandler::handleJobResult, []{});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCopyFromTrash(0, {}, url, AbstractJobHandler::JobFlag::kNoHint, handle));

    stub.set_lamda(&FileCopyMoveJob::copyFromTrash, []{return nullptr;});
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCopyFromTrash(0, {}, url, AbstractJobHandler::JobFlag::kNoHint, handle));

    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus){};
    EXPECT_NO_FATAL_FAILURE(op->handleOperationCopyFromTrash(0, {}, url, AbstractJobHandler::JobFlag::kNoHint, handle, QVariant(), callback));

    op->stoped = true;
    stub.set_lamda(&DialogManager::showDeleteFilesDialog, []{return 0;});
    stub.set_lamda(&FileCopyMoveJob::cleanTrash, []{return nullptr;});
    stub.set_lamda(&FileCopyMoveJob::deletes, []{return nullptr;});
    stub.set_lamda(&DialogManager::showClearTrashDialog, []{return 0;});

    op->onCleanTrashUrls(0, {}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle);

    op->stoped = false;
    op->onCleanTrashUrls(0, {url}, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle);

    op->disconnect(op);

    op->countTrashFile(0, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle);

    QProcess::execute("touch ./testSyncFileInfo.txt");
    LocalFileHandler hand;
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    QProcess::execute("rm testSyncFileInfo.txt");
    op->countTrashFile(0, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handle);
}
