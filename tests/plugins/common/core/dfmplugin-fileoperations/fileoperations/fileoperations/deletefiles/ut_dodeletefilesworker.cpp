// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/deletefiles/deletefiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/deletefiles/dodeletefilesworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

#include <dfm-io/denumerator.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_DoDeleteFilesWorker : public testing::Test
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

TEST_F(UT_DoDeleteFilesWorker, testDoWork)
{
    DeleteFiles job;
    DoDeleteFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoDeleteFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());

    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoDeleteFilesWorker::deleteFilesOnCanNotRemoveDevice,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoDeleteFilesWorker::deleteFilesOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(AbstractWorker, endWork), []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doWork());

    worker.isSourceFileLocal = true;
    EXPECT_TRUE(worker.doWork());

    worker.onUpdateProgress();
    worker.stop();
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_EQ(AbstractJobHandler::SupportAction::kNoAction,
              worker.doHandleErrorAndWait(QUrl::fromLocalFile(QDir::currentPath()), AbstractJobHandler::JobErrorType::kNoError));
}

TEST_F(UT_DoDeleteFilesWorker, testDeleteFilesOnCanNotRemoveDevice)
{
    DoDeleteFilesWorker worker;
    stub_ext::StubExt stub;
    worker.localFileHandler.reset(new LocalFileHandler);

    worker.stop();
    EXPECT_TRUE(worker.deleteFilesOnCanNotRemoveDevice());

    worker.allFilesList.append(QUrl());
    EXPECT_FALSE(worker.deleteFilesOnCanNotRemoveDevice());

    worker.resume();
    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    worker.sourceUrls.append(QUrl());
    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.deleteFilesOnCanNotRemoveDevice());

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.deleteFilesOnCanNotRemoveDevice());

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.deleteFilesOnCanNotRemoveDevice());
}

TEST_F(UT_DoDeleteFilesWorker, testDeleteFilesOnOtherDevice)
{
    DoDeleteFilesWorker worker;
    stub_ext::StubExt stub;
    EXPECT_TRUE(worker.deleteFilesOnOtherDevice());

    worker.sourceUrls.append(QUrl());
    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.deleteFilesOnOtherDevice());

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.deleteFilesOnOtherDevice());

    worker.sourceUrls.clear();
    auto url = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "target_DoDeleteFilesWorker.txt");
    worker.sourceUrls.append(url);
    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoDeleteFilesWorker::deleteDirOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
            return type != OptInfoType::kIsSymLink;});
    EXPECT_TRUE(worker.deleteFilesOnOtherDevice());

    stub.set_lamda(&DoDeleteFilesWorker::deleteDirOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
            return false;});
    EXPECT_FALSE(worker.deleteFilesOnOtherDevice());
}

TEST_F(UT_DoDeleteFilesWorker, testDeleteFileOnOtherDevice)
{
    DoDeleteFilesWorker worker;
    stub_ext::StubExt stub;
    worker.localFileHandler.reset(new LocalFileHandler);
    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});

    worker.stop();
    auto url = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "target_DoDeleteFilesWorker.txt");
    EXPECT_FALSE(worker.deleteFileOnOtherDevice(url));

    worker.resume();
    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.deleteFileOnOtherDevice(url));

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.deleteFileOnOtherDevice(url));
}

TEST_F(UT_DoDeleteFilesWorker, testDeleteDirOnOtherDevice)
{
    DoDeleteFilesWorker worker;
    stub_ext::StubExt stub;
    worker.localFileHandler.reset(new LocalFileHandler);
    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return true;});

    worker.stop();
    auto url = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "target_DoDeleteFilesWorker.txt");
    auto info = InfoFactory::create<FileInfo>(url);
    EXPECT_FALSE(worker.deleteDirOnOtherDevice(info));

    worker.resume();
    stub.set_lamda(VADDR(SyncFileInfo,countChildFile), [](){ __DBG_STUB_INVOKE__
            return -1;});
    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.deleteDirOnOtherDevice(info));
    stub.set_lamda(VADDR(SyncFileInfo,countChildFile), [](){ __DBG_STUB_INVOKE__
            return 1;});
    EXPECT_TRUE(worker.deleteDirOnOtherDevice(info));

    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.deleteDirOnOtherDevice(info));

    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    stub.set_lamda(VADDR(LocalDirIterator,hasNext), [](){ __DBG_STUB_INVOKE__
            return true;});
    stub.set_lamda(VADDR(LocalDirIterator,next), [](){ __DBG_STUB_INVOKE__
            return QUrl();});
    EXPECT_FALSE(worker.deleteDirOnOtherDevice(info));

    int index{0};
    stub.set_lamda(VADDR(LocalDirIterator,hasNext), [&index](){ __DBG_STUB_INVOKE__
            index++; return index % 2;});
    stub.set_lamda(&DoDeleteFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.deleteDirOnOtherDevice(info));

    stub.set_lamda(VADDR(LocalDirIterator,next), [url](){ __DBG_STUB_INVOKE__
            return url;});
    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
            return type != OptInfoType::kIsSymLink;});
    stub.set_lamda(&DoDeleteFilesWorker::deleteFileOnOtherDevice,[]{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.deleteDirOnOtherDevice(info));
}
