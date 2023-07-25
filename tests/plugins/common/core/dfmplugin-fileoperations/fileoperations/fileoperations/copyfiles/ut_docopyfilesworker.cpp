// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/copyfiles/copyfiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/copyfiles/docopyfilesworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_DoCopyFilesWorker : public testing::Test
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

TEST_F(UT_DoCopyFilesWorker, testDoWork)
{
    CopyFiles job;
    DoCopyFilesWorker worker;
    worker.workData.reset(new WorkerData);
    worker.workData->jobFlags |= AbstractJobHandler::JobFlag::kCopyRemote;
    stub_ext::StubExt stub;
    stub.set_lamda(&ClipBoard::getRemoteUrls, []{ __DBG_STUB_INVOKE__ return QList<QUrl>();});
    stub.set_lamda(&DoCopyFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{__DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoCopyFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.doWork());

    worker.workData->jobFlags &= AbstractJobHandler::JobFlag::kNoHint;
    stub.clear();
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{__DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(worker.doWork());

    stub.set_lamda(&DoCopyFilesWorker::checkTotalDiskSpaceAvailable, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doWork());

    stub.set_lamda(&DoCopyFilesWorker::copyFiles, []{__DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());
}

TEST_F(UT_DoCopyFilesWorker, testInitArgs)
{
    DoCopyFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCopyFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DoCopyFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.initArgs());

    worker.sourceUrls.append(QUrl());
    EXPECT_FALSE(worker.initArgs());

    QUrl targetUrl;
    targetUrl.setScheme("smb");
    worker.targetUrl = targetUrl;
    EXPECT_FALSE(worker.initArgs());

    targetUrl = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "target.txt");
    worker.targetUrl = targetUrl;
    EXPECT_FALSE(worker.initArgs());

    stub.set_lamda(VADDR(SyncFileInfo, exists), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.initArgs());

    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    worker.precompleteTargetFileInfo.append(targetInfo);
    worker.endWork();
}

bool copyFileFunc(DoCopyFilesWorker *&, const FileInfoPointer &, const FileInfoPointer &, bool *skip) {
    __DBG_STUB_INVOKE__
    if (skip)
        *skip = true;
    return false;
}

TEST_F(UT_DoCopyFilesWorker, testCopyFiles)
{
    DoCopyFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    worker.stop();
    worker.sourceUrls.append(QUrl());
    EXPECT_FALSE(worker.copyFiles());

    worker.resume();
    stub.set_lamda(&DoCopyFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.copyFiles());

    stub.set_lamda(&DoCopyFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.copyFiles());

    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    worker.targetInfo = targetInfo;
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileUtils::isHigherHierarchy, []{ __DBG_STUB_INVOKE__ return true;});
    worker.sourceUrls.clear();
    worker.sourceUrls.append(sorceUrl);
    EXPECT_FALSE(worker.copyFiles());

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoCopyFilesWorker::doCopyFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyFiles());

    stub.set(ADDR(DoCopyFilesWorker, doCopyFile), copyFileFunc);
    EXPECT_TRUE(worker.copyFiles());

    worker.onUpdateProgress();
}
