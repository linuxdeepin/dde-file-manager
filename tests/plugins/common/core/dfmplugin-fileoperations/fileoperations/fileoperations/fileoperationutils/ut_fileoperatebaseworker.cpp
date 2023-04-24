// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/fileoperatebaseworker.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/copyfiles/storageinfo.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <gtest/gtest.h>


DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_FileOperateBaseWorker : public testing::Test
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

TEST_F(UT_FileOperateBaseWorker, testDoHandleErrorAndWait)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});

    JobHandlePointer handle(new AbstractJobHandler);
    worker.setWorkArgs(handle, {}, QUrl());
    worker.currentState = AbstractJobHandler::JobState::kStopState;

    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub.set_lamda(VADDR(FileOperateBaseWorker, pause), []{ __DBG_STUB_INVOKE__ });
    EXPECT_EQ(worker.doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kOpenError), AbstractJobHandler::SupportAction::kCancelAction);

    worker.currentState = AbstractJobHandler::JobState::kRunningState;
    EXPECT_EQ(worker.doHandleErrorAndWait(url, QUrl(), AbstractJobHandler::JobErrorType::kOpenError), AbstractJobHandler::SupportAction::kNoAction);

    EXPECT_EQ(worker.doHandleErrorAndWait(url, url, AbstractJobHandler::JobErrorType::kOpenError), AbstractJobHandler::SupportAction::kCoexistAction);

    worker.workData->errorOfAction.insert(AbstractJobHandler::JobErrorType::kOpenError, AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_EQ(worker.doHandleErrorAndWait(url, url, AbstractJobHandler::JobErrorType::kOpenError), AbstractJobHandler::SupportAction::kSkipAction);

    worker.workData->errorOfAction.clear();

}

TEST_F(UT_FileOperateBaseWorker, testEmitSpeedUpdatedNotify)
{
    FileOperateBaseWorker worker;
    worker.emitSpeedUpdatedNotify(100);
}

TEST_F(UT_FileOperateBaseWorker, testSetTargetPermissions)
{
    FileOperateBaseWorker worker;
    worker.initArgs();
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch ./testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    auto info = InfoFactory::create<FileInfo>(url);
    auto targinfo = InfoFactory::create<FileInfo>(url);
    worker.setTargetPermissions(info, targinfo);
    QProcess::execute("rm ./testSyncFileInfo.txt");
}

TEST_F(UT_FileOperateBaseWorker, testReadAheadSourceFile)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch ./testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");

    FileOperateBaseWorker worker;
    auto info = InfoFactory::create<FileInfo>(url);
    worker.readAheadSourceFile(info);

    dfmio::DFile file(url);

    if (file.open(DFile::OpenFlag::kWriteOnly)) {
        file.write(QByteArray("test"));
        file.close();
        info->refresh();
        worker.readAheadSourceFile(info);
    }

    QProcess::execute("rm ./testSyncFileInfo.txt");
}

TEST_F(UT_FileOperateBaseWorker, testCheckDiskSpaceAvailable)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    FileOperateBaseWorker worker;
    worker.workData.reset(new WorkerData);
    EXPECT_TRUE(worker.checkDiskSpaceAvailable(url, url, nullptr, nullptr));

    QSharedPointer<StorageInfo> storeInfo(new StorageInfo(url.path()));
    EXPECT_TRUE(worker.checkDiskSpaceAvailable(url, url, storeInfo, nullptr));

    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsUtils::isFilesSizeOutLimit, []{  __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.checkDiskSpaceAvailable(url, url, storeInfo, nullptr));
}

TEST_F(UT_FileOperateBaseWorker, testDeleteFile)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch ./testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");

    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});

    FileOperateBaseWorker worker;
    worker.currentState = AbstractJobHandler::JobState::kStopState;
    EXPECT_FALSE(worker.stateCheck());
    bool ok = false;
    EXPECT_FALSE(worker.deleteFile(url, url, &ok, true));

    worker.currentState = AbstractJobHandler::JobState::kRunningState;
    EXPECT_TRUE(worker.stateCheck());
    worker.initArgs();
    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.deleteFile(url, url, &ok, true));

    worker.currentState = AbstractJobHandler::JobState::kPauseState;
    EXPECT_TRUE(worker.stateCheck());

    QProcess::execute("rm ./testSyncFileInfo.txt");
}
