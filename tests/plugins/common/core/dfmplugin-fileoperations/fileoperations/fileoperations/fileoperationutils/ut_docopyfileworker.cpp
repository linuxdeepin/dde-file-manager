// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/docopyfileworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>


DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_DoCopyFileWorker : public testing::Test
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

TEST_F(UT_DoCopyFileWorker, testpauseorresumeorstop)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    worker.pause();
    EXPECT_EQ(DoCopyFileWorker::kPasued, worker.state);

    worker.resume();
    EXPECT_EQ(DoCopyFileWorker::kNormal, worker.state);

    worker.stop();
    EXPECT_EQ(DoCopyFileWorker::kStoped, worker.state);

    auto url = QUrl::fromLocalFile(QDir::currentPath());
    worker.skipMemcpyBigFile(url);
    EXPECT_EQ(url, worker.memcpySkipUrl);
}

TEST_F(UT_DoCopyFileWorker, testOperateAction)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    worker.operateAction(AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_FALSE(worker.retry);
    EXPECT_EQ(AbstractJobHandler::SupportAction::kSkipAction, worker.currentAction);

    stub_ext::StubExt stub;
    stub.set_lamda(&DoCopyFileWorker::doDfmioFileCopy, []{ __DBG_STUB_INVOKE__ return false;});
    worker.doFileCopy(nullptr,nullptr);
    EXPECT_EQ(1 , data->completeFileCount);
}

TEST_F(UT_DoCopyFileWorker, testDoCopyFilePractically)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);

    worker.stop();
    EXPECT_FALSE(worker.doCopyFilePractically(nullptr, nullptr, nullptr));

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);

    bool skip{false};
    worker.resume();
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCopyFileWorker::createFileDevices, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    stub.clear();
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kSkipAction;
    });
    EXPECT_FALSE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    int index = 0;
    stub.set_lamda(&DoCopyFileWorker::openFile,[&index]{
        __DBG_STUB_INVOKE__
        index++;
        return index %2;
    });
    EXPECT_FALSE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    stub.clear();
    QProcess::execute("touch sourceUrl.txt");
    sorceInfo->refresh();
    EXPECT_TRUE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    data->jobFlags |= AbstractJobHandler::JobFlag::kCopyResizeDestinationFile;
    stub.set_lamda(&DoCopyFileWorker::resizeTargetFile,[]{ __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(VADDR(SyncFileInfo, size),[]{ __DBG_STUB_INVOKE__ return 10; });
    EXPECT_FALSE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    data->jobFlags &= AbstractJobHandler::JobFlag::kNoHint;
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kSkipAction;
    });
    EXPECT_FALSE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    stub.set_lamda(&DoCopyFileWorker::doReadFile,[]{
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&DoCopyFileWorker::doWriteFile,[]{
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_FALSE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    stub.set_lamda(&DoCopyFileWorker::doWriteFile,[]{
        __DBG_STUB_INVOKE__
        return true;
    });
    data->jobFlags |= AbstractJobHandler::JobFlag::kCopyIntegrityChecking;
    stub.set_lamda(&DFMIO::DFile::pos,[]{ __DBG_STUB_INVOKE__ return 10;});
    EXPECT_TRUE(worker.doCopyFilePractically(sorceInfo, targetInfo, &skip));

    QProcess::execute("rm sourceUrl.txt targetUrl.txt");
}

TEST_F(UT_DoCopyFileWorker, testDoDfmioFileCopy)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);

    worker.stop();
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    EXPECT_FALSE(worker.doDfmioFileCopy(sorceInfo, targetInfo, nullptr));

    bool skip{false};
    worker.resume();
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile, []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DoCopyFileWorker::stateCheck, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doDfmioFileCopy(sorceInfo, targetInfo, &skip));

    stub.set_lamda(&DoCopyFileWorker::stateCheck, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileUtils::isMtpFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DOperator::copyFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kSkipAction;
    });
    stub.set_lamda(&DoCopyFileWorker::actionOperating, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&DoCopyFileWorker::syncBlockFile, []{ __DBG_STUB_INVOKE__ });
    EXPECT_FALSE(worker.doDfmioFileCopy(sorceInfo, targetInfo, &skip));

    DoCopyFileWorker::ProgressData *datatt {new DoCopyFileWorker::ProgressData()};
    datatt->data = worker.workData;
    datatt->copyFile = sorceUrl;
    DoCopyFileWorker::progressCallback(0, 0, datatt);
    delete datatt;
}

TEST_F(UT_DoCopyFileWorker, testActionOperating)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);

    worker.stop();
    EXPECT_FALSE(worker.actionOperating(AbstractJobHandler::SupportAction::kNoAction, 10, nullptr));

    worker.resume();
    bool skip{false};
    EXPECT_FALSE(worker.actionOperating(AbstractJobHandler::SupportAction::kSkipAction, 10, &skip));

    EXPECT_TRUE(worker.actionOperating(AbstractJobHandler::SupportAction::kNoAction, 10, &skip));
}

TEST_F(UT_DoCopyFileWorker, testDoHandleErrorAndWait)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);

    data->errorOfAction.insert(AbstractJobHandler::JobErrorType::kNoSourceError, AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_EQ(worker.doHandleErrorAndWait(QUrl(), QUrl(), AbstractJobHandler::JobErrorType::kNoSourceError),
              AbstractJobHandler::SupportAction::kSkipAction);

    data->errorOfAction.clear();
    auto url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_EQ(worker.doHandleErrorAndWait(url, url, AbstractJobHandler::JobErrorType::kNoSourceError),
              AbstractJobHandler::SupportAction::kCoexistAction);

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    worker.stop();
    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::isSameFile, []{__DBG_STUB_INVOKE__ return false;});
    EXPECT_EQ(worker.doHandleErrorAndWait(sorceUrl, targetUrl, AbstractJobHandler::JobErrorType::kNoSourceError),
              AbstractJobHandler::SupportAction::kCancelAction);

    stub.set_lamda(static_cast<bool(QWaitCondition::*)(QMutex *, unsigned long )>(&QWaitCondition::wait), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.stateCheck());

    worker.pause();
    EXPECT_FALSE(worker.stateCheck());

    worker.resume();
    worker.currentAction = AbstractJobHandler::SupportAction::kNoAction;
    EXPECT_EQ(worker.doHandleErrorAndWait(sorceUrl, targetUrl, AbstractJobHandler::JobErrorType::kNoSourceError),
              AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(UT_DoCopyFileWorker, testReadAheadSourceFile)
{
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    QProcess::execute("touch sourceUrl.txt");

    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    worker.readAheadSourceFile(sorceInfo);

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(SyncFileInfo, size),[]{ __DBG_STUB_INVOKE__ return 10; });

    worker.readAheadSourceFile(sorceInfo);
    QProcess::execute("rm sourceUrl.txt");
}

TEST_F(UT_DoCopyFileWorker, testCreateFileDevice)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};
    QSharedPointer<DFMIO::DFile> file{nullptr};
    stub_ext::StubExt stub;

    stub.set_lamda(&DoCopyFileWorker::actionOperating, []{__DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.createFileDevice(sorceInfo, targetInfo, sorceInfo, file, &skip));

    stub.clear();
    int index = 0;
    stub.set_lamda(&DoCopyFileWorker::createFileDevice, [&index]{__DBG_STUB_INVOKE__ index++; return index%2;});
    EXPECT_FALSE(worker.createFileDevices(sorceInfo, targetInfo, file, file, &skip));
}

TEST_F(UT_DoCopyFileWorker, testResizeTargetFile)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};
    QSharedPointer<DFMIO::DFile> file{new DFile(sorceUrl)};
    stub_ext::StubExt stub;
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kSkipAction;
    });
    EXPECT_TRUE(worker.resizeTargetFile(sorceInfo, targetInfo, file, &skip));

    typedef qint64(DFile::*WriteFunc)(const QByteArray &);
    stub.set_lamda(static_cast<WriteFunc>(&DFile::write), []{ __DBG_STUB_INVOKE__ return 0;});
    EXPECT_FALSE(worker.resizeTargetFile(sorceInfo, targetInfo, file, &skip));
}

TEST_F(UT_DoCopyFileWorker, testDoReadFile)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};

    worker.stop();
    QSharedPointer<DFMIO::DFile> file{new DFile(sorceUrl)};
    qint64 blocksize = 512, readsize = 0;
    char buffer[512];
    stub_ext::StubExt stub;
    stub.set_lamda(&DFile::pos, []{ __DBG_STUB_INVOKE__ return 0; });
    EXPECT_FALSE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));

    worker.resume();
    int index{0};
    typedef qint64 (DFile::*readFunc)(char *, qint64);
    stub.set_lamda(static_cast<readFunc>(&DFile::read),[]{ __DBG_STUB_INVOKE__ return 0;});
    stub.set_lamda(&DoCopyFileWorker::stateCheck,[&index]{ __DBG_STUB_INVOKE__ index++; return index % 2;});
    EXPECT_FALSE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));

    stub.reset(&DoCopyFileWorker::stateCheck);
    EXPECT_TRUE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));

    stub.set_lamda(static_cast<readFunc>(&DFile::read),[]{ __DBG_STUB_INVOKE__ return -1;});
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kSkipAction;
    });
    EXPECT_FALSE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));

    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kNoAction;
    });
    EXPECT_TRUE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));

    index = 0;
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[&index]{
        __DBG_STUB_INVOKE__
        index++;
        if (index %2)
            return AbstractJobHandler::SupportAction::kRetryAction;
        return AbstractJobHandler::SupportAction::kNoAction;
    });
    stub.set_lamda(&DFile::seek, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));

    index = 0;
    stub.set_lamda(&DoCopyFileWorker::actionOperating,[]{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doReadFile(sorceInfo, targetInfo, file, buffer, blocksize, readsize, &skip));
}

TEST_F(UT_DoCopyFileWorker, testDoWriteFile)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};
    worker.stop();
    QSharedPointer<DFMIO::DFile> file{new DFile(sorceUrl)};
    qint64 blocksize = 512, readsize = 0;
    char buffer[512];
    stub_ext::StubExt stub;
    EXPECT_FALSE(worker.doWriteFile(sorceInfo, targetInfo, file, buffer, readsize, &skip));

    worker.resume();
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kSkipAction;
    });
    EXPECT_FALSE(worker.doWriteFile(sorceInfo, targetInfo, file, buffer, readsize, &skip));

    int index = 0;
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[&index]{
        __DBG_STUB_INVOKE__
        index++;
        if (index %2)
            return AbstractJobHandler::SupportAction::kRetryAction;
        return AbstractJobHandler::SupportAction::kNoAction;
    });
    stub.set_lamda(&DFile::seek, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWriteFile(sorceInfo, targetInfo, file, buffer, readsize, &skip));

    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kNoAction;
    });
    EXPECT_TRUE(worker.doWriteFile(sorceInfo, targetInfo, file, buffer, readsize, &skip));

    typedef qint64 (DFile::*writeFunc)(const char *, qint64);
    stub.set_lamda(static_cast<writeFunc>(&DFile::write), []{ __DBG_STUB_INVOKE__ return 1;});
    data->needSyncEveryRW = true;
    EXPECT_TRUE(worker.doWriteFile(sorceInfo, targetInfo, file, buffer, readsize, &skip));

    data->isFsTypeVfat = true;
    stub.set_lamda(&DoCopyFileWorker::openFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.doWriteFile(sorceInfo, targetInfo, file, buffer, readsize, &skip));
}

TEST_F(UT_DoCopyFileWorker, testVerifyFileIntegrity)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};
    worker.stop();
    QSharedPointer<DFMIO::DFile> file{new DFile(sorceUrl)};
    qint64 blocksize = 512, readsize = 0;
    char buffer[512];
    stub_ext::StubExt stub;
    data->signalThread = false;
    worker.retry = true;
    EXPECT_TRUE(worker.verifyFileIntegrity(blocksize, blocksize, sorceInfo, targetInfo, file));

    data->jobFlags |= AbstractJobHandler::JobFlag::kCopyIntegrityChecking;
    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait,[]{
        __DBG_STUB_INVOKE__
        return AbstractJobHandler::SupportAction::kNoAction;
    });
    EXPECT_FALSE(worker.verifyFileIntegrity(blocksize, blocksize, sorceInfo, targetInfo, file));

    typedef qint64 (DFile::*readFunc)(char *, qint64);
    stub.set_lamda(static_cast<readFunc>(&DFile::read),[]{ __DBG_STUB_INVOKE__ return 1;});
    EXPECT_FALSE(worker.verifyFileIntegrity(blocksize, blocksize, sorceInfo, targetInfo, file));

    worker.resume();
    int index = 0;
    stub.set_lamda(static_cast<readFunc>(&DFile::read),[&index]{ __DBG_STUB_INVOKE__
        index++;
        return index % 2;});
    worker.retry = true;
    stub.set_lamda(&DFile::pos, []{ __DBG_STUB_INVOKE__ return 0; });
    stub.set_lamda(VADDR(SyncFileInfo, size), []{ __DBG_STUB_INVOKE__ return 0;});
    EXPECT_FALSE(worker.verifyFileIntegrity(blocksize, blocksize, sorceInfo, targetInfo, file));

    EXPECT_TRUE(worker.verifyFileIntegrity(blocksize, 12517567, sorceInfo, targetInfo, file));
}

int OpenFunc(const char *__file, int __oflag, ...){
    Q_UNUSED(__file);
    Q_UNUSED(__oflag);
    __DBG_STUB_INVOKE__
    return 0;
}

int SyncFFunc(int) {
    __DBG_STUB_INVOKE__
    return 0;
}

TEST_F(UT_DoCopyFileWorker, testSyncBlockFile)
{
    QSharedPointer<WorkerData> data(new WorkerData);
    DoCopyFileWorker worker(data);

    worker.syncBlockFile(nullptr);

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);

    stub_ext::StubExt stub;
    stub.set(&::open, OpenFunc);
    stub.set(&::syncfs, SyncFFunc);
    stub.set(&::close, SyncFFunc);
    worker.syncBlockFile(sorceInfo);
}
