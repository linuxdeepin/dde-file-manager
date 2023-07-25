// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/trashfiles/restoretrashfiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/trashfiles/dorestoretrashfilesworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

#include <dfm-io/trashhelper.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_DoRestoreTrashFilesWorker : public testing::Test
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

TEST_F(UT_DoRestoreTrashFilesWorker, testDoWork)
{
    RestoreTrashFiles job;
    DoRestoreTrashFilesWorker worker;
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);
    stub.set_lamda(&DoRestoreTrashFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());

    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(AbstractWorker, endWork), []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doWork());

    EXPECT_FALSE(worker.statisticsFilesSize());

    worker.sourceUrls.append(FileUtils::trashRootUrl());
    EXPECT_TRUE(worker.statisticsFilesSize());

    stub.set_lamda(VADDR(AbstractWorker, initArgs), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.initArgs());
    worker.onUpdateProgress();
}

TEST_F(UT_DoRestoreTrashFilesWorker, testTranslateUrls)
{
    DoRestoreTrashFilesWorker worker;
    stub_ext::StubExt stub;

    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl_DoRestoreTrashFilesWorker.txt");
    worker.sourceUrls.append(targetUrl);
    stub.set_lamda(&DoRestoreTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.translateUrls());

    stub.set_lamda(&DoRestoreTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_FALSE(worker.translateUrls());

    targetUrl.setUserInfo("12-12");
    worker.sourceUrls.clear();
    worker.sourceUrls.append(targetUrl);
    stub.set_lamda(&TrashHelper::getTrashUrls, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.translateUrls());

    stub.reset(&TrashHelper::getTrashUrls);
    EXPECT_FALSE(worker.translateUrls());

    stub.set_lamda(&DoRestoreTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    worker.sourceUrls.append(targetUrl);
    EXPECT_TRUE(worker.translateUrls());
}

bool createParentDirFunc(DoRestoreTrashFilesWorker *&,
                     const FileInfoPointer &trashInfo,
                     const FileInfoPointer &restoreInfo,
                     FileInfoPointer &targetFileInfo,
                     bool *result)
{
    __DBG_STUB_INVOKE__
    if (result)
        *result = true;
    return false;
}

bool checkRestoreInfoFunc(DoRestoreTrashFilesWorker *&,
                          const QUrl &url,
                          FileInfoPointer &restoreInfo)
{
    __DBG_STUB_INVOKE__
    restoreInfo = InfoFactory::create<FileInfo>(
                QUrl::fromLocalFile(QDir::currentPath() +
                                    QDir::separator() + "testDir_DoRestoreTrashFilesWorker.txt"));
    return true;
}

bool doCheckFileFunc(DoRestoreTrashFilesWorker *&,
                 const FileInfoPointer &fromInfo,
                 const FileInfoPointer &toInfo,
                 const QString &fileName,
                 FileInfoPointer &newTargetInfo, bool *skip){
    __DBG_STUB_INVOKE__
    newTargetInfo = InfoFactory::create<FileInfo>(
                QUrl::fromLocalFile(QDir::currentPath() +
                                    QDir::separator() + "testDir_DoRestoreTrashFilesWorker.txt"));
    return true;
}

TEST_F(UT_DoRestoreTrashFilesWorker, testDoRestoreTrashFiles)
{
    DoRestoreTrashFilesWorker worker;
    stub_ext::StubExt stub;
    worker.stop();
    worker.allFilesList.append(QUrl());
    EXPECT_FALSE(worker.doRestoreTrashFiles());

    worker.resume();
    worker.handleSourceFiles.append(QUrl());
    EXPECT_TRUE(worker.doRestoreTrashFiles());

    worker.handleSourceFiles.clear();
    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.doRestoreTrashFiles());

    worker.handleSourceFiles.clear();
    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set(&DoRestoreTrashFilesWorker::checkRestoreInfo, checkRestoreInfoFunc);
    EXPECT_FALSE(worker.doRestoreTrashFiles());

    worker.handleSourceFiles.clear();
    stub.set(&DoRestoreTrashFilesWorker::createParentDir, createParentDirFunc);
    EXPECT_TRUE(worker.doRestoreTrashFiles());

    worker.allFilesList.clear();
    auto tmpUrl = QUrl::fromLocalFile(QDir::currentPath() +QDir::separator() + "testDir_DoRestoreTrashFilesWorker.txt");
    worker.allFilesList.append(tmpUrl);
    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoRestoreTrashFilesWorker::doCheckFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.doRestoreTrashFiles());

    worker.handleSourceFiles.clear();
    stub.set(&DoRestoreTrashFilesWorker::doCheckFile, doCheckFileFunc);
    stub.set_lamda(&LocalFileHandler::moveFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doRestoreTrashFiles());

    worker.handleSourceFiles.clear();
    stub.set_lamda(&LocalFileHandler::moveFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&LocalFileHandler::errorCode, []{ __DBG_STUB_INVOKE__ return DFMIOErrorCode::DFM_IO_ERROR_WOULD_MERGE;});
    stub.set_lamda(&DoRestoreTrashFilesWorker::mergeDir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doRestoreTrashFiles());
}

TEST_F(UT_DoRestoreTrashFilesWorker, testCreateParentDir)
{
    DoRestoreTrashFilesWorker worker;
    stub_ext::StubExt stub;

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    stub.set_lamda(&UrlRoute::urlParent, []{ __DBG_STUB_INVOKE__ return QUrl();});
    FileInfoPointer newTargetInfo(nullptr);
    bool skip{false};
    EXPECT_FALSE(worker.createParentDir(sorceInfo, targetInfo, newTargetInfo, &skip));

    auto tmpUrl = targetUrl;
    tmpUrl.setScheme("trash");
    stub.set_lamda(&UrlRoute::urlParent, [tmpUrl]{ __DBG_STUB_INVOKE__ return tmpUrl;});
    EXPECT_FALSE(worker.createParentDir(sorceInfo, targetInfo, newTargetInfo, &skip));

    tmpUrl = QUrl::fromLocalFile(QDir::currentPath());
    stub.set_lamda(&UrlRoute::urlParent, [tmpUrl]{ __DBG_STUB_INVOKE__ return tmpUrl;});
    EXPECT_TRUE(worker.createParentDir(sorceInfo, targetInfo, newTargetInfo, &skip));

    stub.set_lamda(&DoRestoreTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    tmpUrl = QUrl::fromLocalFile(QDir::currentPath() +QDir::separator() + "testDir_DoRestoreTrashFilesWorker");
    stub.set_lamda(&UrlRoute::urlParent, [tmpUrl]{ __DBG_STUB_INVOKE__ return tmpUrl;});
    stub.set_lamda(&LocalFileHandler::mkdir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.createParentDir(sorceInfo, targetInfo, newTargetInfo, &skip));
}

TEST_F(UT_DoRestoreTrashFilesWorker, testCheckRestoreInfo)
{
    DoRestoreTrashFilesWorker worker;
    stub_ext::StubExt stub;
    FileInfoPointer restoreInfo{nullptr};
    stub.set_lamda(&DoRestoreTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.checkRestoreInfo(QUrl(), restoreInfo));

    auto url = QUrl::fromLocalFile(QDir::currentPath() +QDir::separator() + "testDir_DoRestoreTrashFilesWorker");
    stub.set_lamda(VADDR(SyncFileInfo, urlOf), []{ __DBG_STUB_INVOKE__ return QUrl();});
    EXPECT_FALSE(worker.checkRestoreInfo(url, restoreInfo));

    worker.targetUrl.setScheme("trash");
    EXPECT_FALSE(worker.checkRestoreInfo(url, restoreInfo));

    worker.targetUrl = url;
    EXPECT_TRUE(worker.checkRestoreInfo(url, restoreInfo));
}

TEST_F(UT_DoRestoreTrashFilesWorker, testMergeDir)
{
    DoRestoreTrashFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoRestoreTrashFilesWorker::copyFileFromTrash, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return true;});
    QUrl source,target;
    EXPECT_TRUE(worker.mergeDir(source, target, dfmio::DFile::CopyFlag::kOverwrite));

    stub.set_lamda(&DoRestoreTrashFilesWorker::copyFileFromTrash, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.mergeDir(source, target, dfmio::DFile::CopyFlag::kOverwrite));
}
