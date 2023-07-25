// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/trashfiles/copyfromtrashfiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/trashfiles/docopyfromtrashfilesworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

#include <dfm-io/dfmio_utils.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_DoCopyFromTrashFilesWorker : public testing::Test
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

TEST_F(UT_DoCopyFromTrashFilesWorker, testDoWork)
{
    CopyFromTrashTrashFiles job;
    DoCopyFromTrashFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCopyFromTrashFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());

    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(AbstractWorker, endWork), []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doWork());

    EXPECT_FALSE(worker.statisticsFilesSize());

    worker.sourceUrls.append(QUrl());
    EXPECT_TRUE(worker.statisticsFilesSize());

    stub.set_lamda(VADDR(AbstractWorker, initArgs), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.initArgs());
}

bool createParentDirFunc(DoCopyFromTrashFilesWorker *,
                         const FileInfoPointer &trashInfo,
                         const FileInfoPointer &restoreInfo,
                         FileInfoPointer &targetFileInfo,
                         bool *result) {
    if (result)
        *result = true;
    return false;
}

bool createParentDirFunc1(DoCopyFromTrashFilesWorker *&,
                         const FileInfoPointer &trashInfo,
                         const FileInfoPointer &restoreInfo,
                         FileInfoPointer &targetFileInfo,
                         bool *result) {
    __DBG_STUB_INVOKE__
    if (result)
        *result = true;
    return true;
}

bool doCheckFileFunc(DoCopyFromTrashFilesWorker *&,
                 const FileInfoPointer &fromInfo,
                 const FileInfoPointer &toInfo,
                 const QString &fileName,
                 FileInfoPointer &newTargetInfo, bool *skip){
    __DBG_STUB_INVOKE__
    newTargetInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(QDir::currentPath()));
    return true;
}

TEST_F(UT_DoCopyFromTrashFilesWorker, testDoOperate)
{
    DoCopyFromTrashFilesWorker worker;
    stub_ext::StubExt stub;

    worker.stop();
    worker.sourceUrls.append(QUrl());
    EXPECT_FALSE(worker.doOperate());

    worker.resume();
    stub.set_lamda(&DoCopyFromTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.doOperate());

    stub.set_lamda(&DoCopyFromTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.doOperate());

    auto url = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "target_DoCopyFromTrashFilesWorker.txt");
    worker.sourceUrls.clear();
    worker.sourceUrls.append(url);
    EXPECT_FALSE(worker.doOperate());

    stub.set_lamda(&DoCopyFromTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.doOperate());

    worker.targetUrl = QUrl::fromLocalFile(QDir::currentPath());
    stub.set_lamda(&DoCopyFromTrashFilesWorker::createParentDir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doOperate());

    stub.set(&DoCopyFromTrashFilesWorker::createParentDir, createParentDirFunc);
    EXPECT_TRUE(worker.doOperate());

    stub.set(&DoCopyFromTrashFilesWorker::createParentDir, createParentDirFunc1);
    stub.set_lamda(&DoCopyFromTrashFilesWorker::doCheckFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.doOperate());

    stub.set(&DoCopyFromTrashFilesWorker::doCheckFile, doCheckFileFunc);
    stub.set_lamda(&DoCopyFromTrashFilesWorker::copyFileFromTrash, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doOperate());

    stub.set_lamda(&DoCopyFromTrashFilesWorker::copyFileFromTrash, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doOperate());
}

TEST_F(UT_DoCopyFromTrashFilesWorker, testCreateParentDir)
{
    DoCopyFromTrashFilesWorker worker;
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

    stub.set_lamda(&DoCopyFromTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    tmpUrl = QUrl::fromLocalFile(QDir::currentPath() +QDir::separator() + "testDir_DoCopyFromTrashFilesWorker");
    stub.set_lamda(&UrlRoute::urlParent, [tmpUrl]{ __DBG_STUB_INVOKE__ return tmpUrl;});
    stub.set_lamda(&LocalFileHandler::mkdir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.createParentDir(sorceInfo, targetInfo, newTargetInfo, &skip));
}
