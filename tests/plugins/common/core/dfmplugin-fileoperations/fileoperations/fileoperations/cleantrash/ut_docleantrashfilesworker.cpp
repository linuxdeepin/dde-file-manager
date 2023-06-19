// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/cleantrash/cleantrashfiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/cleantrash/docleantrashfilesworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

#include <dfm-io/denumerator.h>

typedef QMap<QString,QVariant> * mapValue;
Q_DECLARE_METATYPE(mapValue);

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
class UT_DoCleanTrashFilesWorker : public testing::Test
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

TEST_F(UT_DoCleanTrashFilesWorker, testDoWork)
{
    CleanTrashFiles job;
    DoCleanTrashFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCleanTrashFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());

    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(AbstractWorker, endWork), []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doWork());

    worker.onUpdateProgress();

    stub.set_lamda(VADDR(AbstractWorker, initArgs), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.initArgs());

    EXPECT_FALSE(worker.statisticsFilesSize());

    worker.sourceUrls.append(FileUtils::trashRootUrl());
    int index = 0;
    stub.set_lamda(&DEnumerator::hasNext, [&index]{ __DBG_STUB_INVOKE__ index++; return index % 2;});
    stub.set_lamda(&DEnumerator::next, []{ __DBG_STUB_INVOKE__ return QUrl::fromLocalFile(QDir::currentPath());});
    EXPECT_TRUE(worker.statisticsFilesSize());
}

TEST_F(UT_DoCleanTrashFilesWorker, testCleanAllTrashFiles)
{
    DoCleanTrashFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCleanTrashFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    worker.stop();
    EXPECT_TRUE(worker.cleanAllTrashFiles());

    worker.sourceUrls.append(QUrl());
    EXPECT_FALSE(worker.cleanAllTrashFiles());

    worker.allFilesList.append(QUrl());
    EXPECT_FALSE(worker.cleanAllTrashFiles());

    worker.resume();
    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.cleanAllTrashFiles());

    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.cleanAllTrashFiles());

    worker.allFilesList.clear();
    QUrl url("trash:///text_DoCleanTrashFilesWorker.txt");
    worker.allFilesList.append(url);
    EXPECT_TRUE(worker.cleanAllTrashFiles());

    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.cleanAllTrashFiles());

    QSharedPointer<SyncFileInfo> info(new SyncFileInfo(QUrl::fromLocalFile(QDir::currentPath())));
    stub.set_lamda(&InfoFactory::create<FileInfo>, [&info] { __DBG_STUB_INVOKE__ return info; });
    stub.set_lamda(&DoCleanTrashFilesWorker::clearTrashFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.cleanAllTrashFiles());

    stub.set_lamda(&DoCleanTrashFilesWorker::clearTrashFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.cleanAllTrashFiles());

}

TEST_F(UT_DoCleanTrashFilesWorker, testClearTrashFile)
{
    DoCleanTrashFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCleanTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    stub.set_lamda(&DoCleanTrashFilesWorker::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    worker.localFileHandler.reset(new LocalFileHandler);
    QUrl url(QUrl::fromLocalFile(QDir::currentPath()));
    FileInfoPointer info(new SyncFileInfo(url));
    EXPECT_TRUE(worker.clearTrashFile(info));

    stub.clear();
    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_EQ(AbstractJobHandler::SupportAction::kNoAction,
              worker.doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kNoError));

    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.deleteFile(url));

}
