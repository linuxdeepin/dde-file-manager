// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/cutfiles/cutfiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/cutfiles/docutfilesworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>

#include <gtest/gtest.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_DoCutFilesWorker : public testing::Test
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

TEST_F(UT_DoCutFilesWorker, testDoWork)
{
    CutFiles job;
    DoCutFilesWorker worker;
    worker.workData.reset(new WorkerData);
    worker.workData->jobFlags |= AbstractJobHandler::JobFlag::kCopyRemote;
    stub_ext::StubExt stub;
    stub.set_lamda(&ClipBoard::getRemoteUrls, []{ __DBG_STUB_INVOKE__ return QList<QUrl>();});
    stub.set_lamda(&DoCutFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{__DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoCutFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.doWork());

    worker.workData->jobFlags &= AbstractJobHandler::JobFlag::kNoHint;
    stub.clear();
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{__DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doWork());

    stub.set_lamda(&DoCutFilesWorker::cutFiles, []{__DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());
}

TEST_F(UT_DoCutFilesWorker, testInitArgs)
{
    DoCutFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCutFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DoCutFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.initArgs());

    worker.sourceUrls.append(QUrl());
    EXPECT_FALSE(worker.initArgs());

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    worker.targetUrl.setScheme("smb");
    EXPECT_FALSE(worker.initArgs());

    worker.targetUrl = targetUrl;
    EXPECT_FALSE(worker.initArgs());

    stub.set_lamda(VADDR(SyncFileInfo,exists), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.initArgs());
}

TEST_F(UT_DoCutFilesWorker, testCutFiles)
{
    DoCutFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCutFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DoCutFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    worker.stop();
    worker.sourceUrls.append(QUrl());
    EXPECT_FALSE(worker.cutFiles());

    worker.resume();
    EXPECT_FALSE(worker.cutFiles());

    stub.set_lamda(&DoCutFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.cutFiles());

    worker.sourceUrls.clear();
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    worker.sourceUrls.append(sorceUrl);
    stub.set_lamda(&DoCutFilesWorker::checkSelf, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.cutFiles());

    stub.set_lamda(&DoCutFilesWorker::checkSelf, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileUtils::isHigherHierarchy, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(worker.cutFiles());

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [](SyncFileInfo *,OptInfoType type){
        __DBG_STUB_INVOKE__
        return type == OptInfoType::kIsSymLink;});
    stub.set_lamda(&DoCutFilesWorker::checkSymLink, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.cutFiles());

    stub.set_lamda(&DoCutFilesWorker::checkSymLink, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.cutFiles());

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoCutFilesWorker::doCutFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.cutFiles());

    stub.set_lamda(&DoCutFilesWorker::doCutFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.cutFiles());
}

bool checkDiskSpaceAvailableFunc(DoCutFilesWorker *&, const QUrl &fromUrl,
                  const QUrl &toUrl,
                  bool *skip) {
    __DBG_STUB_INVOKE__
    if (skip)
        *skip = true;
    return false;
}

TEST_F(UT_DoCutFilesWorker, testDoCutFile)
{
    DoCutFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCutFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DoCutFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    DFileInfoPointer targetInfo(new DFileInfo(targetUrl));
    DFileInfoPointer sorceInfo(new DFileInfo(sorceUrl));
    stub.set_lamda(&FileUtils::isTrashFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoCutFilesWorker::doRenameFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    worker.stopWork = true;
    bool skip = false;
    EXPECT_FALSE(worker.doCutFile(sorceInfo, targetInfo, &skip));

    stub.set_lamda(VADDR(SyncFileInfo, size), []{ __DBG_STUB_INVOKE__ return 0;});
    stub.set_lamda(&DoCutFilesWorker::doRenameFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    stub.set_lamda(&DoCutFilesWorker::removeTrashInfo,[]{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doCutFile(sorceInfo, targetInfo, &skip));

    stub.reset(&FileUtils::isTrashFile);
    stub.set_lamda(&DoCutFilesWorker::doRenameFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doCutFile(sorceInfo, targetInfo, &skip));

    stub.set_lamda(&DoCutFilesWorker::doRenameFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    stub.set(ADDR(DoCutFilesWorker, checkDiskSpaceAvailable), checkDiskSpaceAvailableFunc);
    EXPECT_TRUE(worker.doCutFile(sorceInfo, targetInfo, &skip));

    stub.set_lamda(&DoCutFilesWorker::checkDiskSpaceAvailable, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoCutFilesWorker::copyAndDeleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doCutFile(sorceInfo, targetInfo, &skip));

    stub.set_lamda(&DoCutFilesWorker::copyAndDeleteFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileUtils::isTrashFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoCutFilesWorker::removeTrashInfo, []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doCutFile(sorceInfo, targetInfo, &skip));
    worker.onUpdateProgress();
    worker.emitCompleteFilesUpdatedNotify(199);
}

bool doCheckFileFunc(DoCutFilesWorker *&, const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo, const QString &fileName,
                                 FileInfoPointer &newTargetInfo, bool *skip) {
    __DBG_STUB_INVOKE__
    if (skip)
        *skip = true;
    newTargetInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(QDir::currentPath()));
    return true;
}

TEST_F(UT_DoCutFilesWorker, testCheckSymLink)
{
    DoCutFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCutFilesWorker::doCheckFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    DFileInfoPointer targetInfo(new DFileInfo(targetUrl));
    DFileInfoPointer sorceInfo(new DFileInfo(sorceUrl));
    EXPECT_FALSE(worker.checkSymLink(sorceInfo));

    stub.set_lamda(&DoCutFilesWorker::doCheckFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    stub.set_lamda(&DoCutFilesWorker::createSystemLink, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.checkSymLink(sorceInfo));

    stub.set_lamda(&DoCutFilesWorker::createSystemLink, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoCutFilesWorker::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.checkSymLink(sorceInfo));

    stub.set_lamda(&DoCutFilesWorker::deleteFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set(ADDR(DoCutFilesWorker, doCheckFile), doCheckFileFunc);
    EXPECT_TRUE(worker.checkSymLink(sorceInfo));

    worker.targetInfo = sorceInfo;
    EXPECT_FALSE(worker.checkSelf(sorceInfo));
}

TEST_F(UT_DoCutFilesWorker, testRenameFileByHandler)
{
    DoCutFilesWorker worker;
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&LocalFileHandler::renameFile, []{ __DBG_STUB_INVOKE__ return false;});
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    DFileInfoPointer targetInfo(new DFileInfo(targetUrl));
    DFileInfoPointer sorceInfo(new DFileInfo(sorceUrl));
    EXPECT_FALSE(worker.renameFileByHandler(sorceInfo, targetInfo));

    worker.localFileHandler.reset(new LocalFileHandler);
    EXPECT_FALSE(worker.renameFileByHandler(sorceInfo, targetInfo));

    FileInfoPointer toInfo(nullptr);
    stub.set_lamda(&DoCutFilesWorker::doCheckFile,[]{ __DBG_STUB_INVOKE__ return nullptr;});
    bool ok{false},skip{false};
    EXPECT_FALSE(worker.doRenameFile(sorceInfo, targetInfo, "tests_iiii.txt", &ok, &skip));

    stub.set_lamda(&DFMUtils::deviceNameFromUrl, []{ __DBG_STUB_INVOKE__
        return QByteArray("test-device");
    });
    EXPECT_FALSE(worker.doRenameFile(sorceInfo, targetInfo, "tests_iiii.txt", &ok, &skip));

    stub.set_lamda(&DoCutFilesWorker::renameFileByHandler, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set(ADDR(DoCutFilesWorker, doCheckFile), doCheckFileFunc);
    worker.targetInfo = targetInfo;
    EXPECT_TRUE(worker.doRenameFile(sorceInfo, targetInfo, "tests_iiii.txt", &ok, &skip));
}
