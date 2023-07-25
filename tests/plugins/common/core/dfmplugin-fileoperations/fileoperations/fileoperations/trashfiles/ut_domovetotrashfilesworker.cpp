// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/trashfiles/movetotrashfiles.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/trashfiles/domovetotrashfilesworker.h"

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
class UT_DoMoveToTrashFilesWorker : public testing::Test
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

TEST_F(UT_DoMoveToTrashFilesWorker, testDoWork)
{
    MoveToTrashFiles job;
    DoMoveToTrashFilesWorker worker;
    stub_ext::StubExt stub;
    stub.set_lamda(&DoMoveToTrashFilesWorker::saveOperations, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWork());

    stub.set_lamda(VADDR(AbstractWorker, doWork), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(AbstractWorker, endWork), []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(worker.doWork());

    EXPECT_TRUE(worker.statisticsFilesSize());

    worker.onUpdateProgress();
}

TEST_F(UT_DoMoveToTrashFilesWorker, testIsCanMoveToTrash)
{
    DoMoveToTrashFilesWorker worker;
    stub_ext::StubExt stub;

    worker.stop();
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl_DoMoveToTrashFilesWorker.txt");
    bool skip{false};
    EXPECT_FALSE(worker.isCanMoveToTrash(targetUrl, &skip));

    worker.resume();
    stub.set_lamda(&DoMoveToTrashFilesWorker::canWriteFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoMoveToTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_TRUE(worker.isCanMoveToTrash(targetUrl, &skip));

    stub.set_lamda(&DoMoveToTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_FALSE(worker.isCanMoveToTrash(targetUrl, &skip));
}


bool isCanMoveToTrashFunc(DoMoveToTrashFilesWorker *&, const QUrl &url, bool *result) {
    __DBG_STUB_INVOKE__
    if (result)
        *result = true;
    return false;
}

TEST_F(UT_DoMoveToTrashFilesWorker, testDoMoveToTrash)
{
    DoMoveToTrashFilesWorker worker;
    stub_ext::StubExt stub;
    QUrl url = QUrl::fromLocalFile("/data/home");
    worker.sourceUrls.append(url);
    worker.stop();
    EXPECT_FALSE(worker.doMoveToTrash());

    worker.resume();
    stub.set_lamda(&FileUtils::isTrashFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doMoveToTrash());

    stub.set_lamda(&FileUtils::isTrashFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set(&DoMoveToTrashFilesWorker::isCanMoveToTrash, isCanMoveToTrashFunc);
    EXPECT_TRUE(worker.doMoveToTrash());

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doMoveToTrash());

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash, []{ __DBG_STUB_INVOKE__ return true;});
    worker.sourceUrls.clear();
    worker.sourceUrls.append(QUrl());
    stub.set_lamda(&DoMoveToTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.doMoveToTrash());

    stub.set_lamda(&DoMoveToTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.doMoveToTrash());

    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl_DoMoveToTrashFilesWorker.txt");
    worker.sourceUrls.clear();
    worker.sourceUrls.append(targetUrl);
    stub.set_lamda(&LocalFileHandler::trashFile, []{ __DBG_STUB_INVOKE__ return "oooo";});
    EXPECT_TRUE(worker.doMoveToTrash());

    stub.set_lamda(&LocalFileHandler::trashFile, []{ __DBG_STUB_INVOKE__ return "";});
    EXPECT_FALSE(worker.doMoveToTrash());

    stub.set_lamda(&DoMoveToTrashFilesWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__
                return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_TRUE(worker.doMoveToTrash());
}

