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
#include <dfm-base/base/standardpaths.h>

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

TEST_F(UT_FileOperateBaseWorker, testTrashInfo)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    auto fileInfo = InfoFactory::create<FileInfo>(url);
    EXPECT_TRUE(worker.trashInfo(fileInfo).isEmpty());
    url.setPath(StandardPaths::location(StandardPaths::kHomePath) + "/.local/share/Trash/files/testttt.txt");
    fileInfo = InfoFactory::create<FileInfo>(url);
    EXPECT_TRUE(worker.trashInfo(fileInfo).path().endsWith("testttt.txt.trashinfo"));
}

TEST_F(UT_FileOperateBaseWorker, testFileOriginName)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(worker.fileOriginName(QUrl()).isEmpty());

    url.setPath(StandardPaths::location(StandardPaths::kHomePath) + "/.local/share/Trash/files/testttt.txt");
    auto fileInfo = InfoFactory::create<FileInfo>(url);
    auto trashInfoUrl = worker.trashInfo(fileInfo);
    EXPECT_TRUE(worker.fileOriginName(trashInfoUrl).isEmpty());

    stub_ext::StubExt stub;
    stub.set_lamda(&DFile::open, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DFile::readAll, []{ __DBG_STUB_INVOKE__ return QByteArray("fjkejfk");});
    EXPECT_TRUE(worker.fileOriginName(trashInfoUrl).isEmpty());

    stub.set_lamda(&DFile::readAll, []{ __DBG_STUB_INVOKE__ return QByteArray("fjkejfk ttt ttt ttt");});
    EXPECT_TRUE(worker.fileOriginName(trashInfoUrl).isEmpty());

    stub.set_lamda(&DFile::readAll, []{ __DBG_STUB_INVOKE__ return QByteArray("fjkejfk ttt ttt98-ooo ttt");});
    EXPECT_EQ(QString("-ooo"),worker.fileOriginName(trashInfoUrl));
}

TEST_F(UT_FileOperateBaseWorker, testRemoveTrashInfo)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&LocalFileHandler::deleteFile, []{ __DBG_STUB_INVOKE__ return true; });
    worker.removeTrashInfo(QUrl());

    worker.removeTrashInfo(url);

    worker.initArgs();
    worker.removeTrashInfo(url);
}

TEST_F(UT_FileOperateBaseWorker, testSetSkipValue)
{
    FileOperateBaseWorker worker;
    bool skip{ false };
    worker.setSkipValue(&skip, AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_TRUE(skip);
}

TEST_F(UT_FileOperateBaseWorker, testInitThreadCopy)
{
    FileOperateBaseWorker worker;
    worker.workData.reset(new WorkerData);
    worker.initThreadCopy();
    EXPECT_TRUE(worker.threadCopyWorker.count() > 0);

    EXPECT_TRUE(worker.copyOtherFileWorker.isNull());
    worker.initSignalCopyWorker();
    EXPECT_FALSE(worker.copyOtherFileWorker.isNull());

    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QString fileName{"testname"};
    auto fileInfo = InfoFactory::create<FileInfo>(url);
    auto targetUrl = worker.createNewTargetUrl(fileInfo, fileName);
    EXPECT_EQ(targetUrl.fileName(), fileName);
    auto newfileInfo = InfoFactory::create<FileInfo>(url);

    worker.currentState = AbstractJobHandler::JobState::kStopState;
    EXPECT_FALSE(worker.doCopyLocalFile(newfileInfo, fileInfo));

    worker.currentState = AbstractJobHandler::JobState::kRunningState;
    EXPECT_TRUE(worker.doCopyLocalFile(newfileInfo, fileInfo));
    worker.stopAllThread();
}

TEST_F(UT_FileOperateBaseWorker, testDoCopyLocalBigFile)
{
    QProcess::execute("rm sourceUrl.txt targetUrl.txt");
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.doCopyLocalBigFile(sorceInfo, targetInfo, &skip));

    QProcess::execute("touch sourceUrl.txt");
    sorceUrl.setPath(url.path() + QDir::separator() + "sourceUrl.txt");
    EXPECT_FALSE(worker.doCopyLocalBigFile(sorceInfo, targetInfo, &skip));

    QFile sorcefile(sorceUrl.path());
    if (sorcefile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QByteArray data("tttttttt");
        for (int i = 0; i < 10000; ++i) {
            sorcefile.write(data);
        }
        sorcefile.close();
        worker.threadPool.reset(new QThreadPool);
        worker.initThreadCopy();
        EXPECT_FALSE(worker.doCopyLocalBigFile(sorceInfo, targetInfo, &skip));
        sorceInfo->refresh();
        EXPECT_TRUE(worker.doCopyLocalBigFile(sorceInfo, targetInfo, &skip));
    }

    QProcess::execute("rm sourceUrl.txt targetUrl.txt");

}

int openTest(int fd, __off_t offset) {
    Q_UNUSED(fd);
    Q_UNUSED(offset);
    return -1;
};

TEST_F(UT_FileOperateBaseWorker, testDoCopyLocalBigFileCallFunc)
{
    FileOperateBaseWorker worker;
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);

    stub.set(&ftruncate, openTest);
    bool skip{false};
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.doCopyLocalBigFileResize(sorceInfo, targetInfo, -1, &skip));

    stub.set_lamda(&DoCopyFileWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.doCopyOtherFile(sorceInfo, targetInfo, &skip));
}

TEST_F(UT_FileOperateBaseWorker, testCreateNewTargetInfo)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath());
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);

    FileInfoPointer newTargetInfo{nullptr};
    bool skip{false};
    EXPECT_FALSE(worker.createNewTargetInfo(sorceInfo, targetInfo, newTargetInfo, QUrl(), &skip, false));
    EXPECT_TRUE(worker.createNewTargetInfo(sorceInfo, targetInfo, newTargetInfo, sorceUrl, &skip, false));

    worker.initThreadCopy();
    worker.skipMemcpyBigFile(sorceUrl);
}

TEST_F(UT_FileOperateBaseWorker, testCheckLinkAndSameUrl)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);


    EXPECT_FALSE(worker.checkLinkAndSameUrl(sorceInfo, targetInfo, false).isValid());
    EXPECT_FALSE(worker.doActionReplace(sorceInfo, targetInfo, true).isValid());
    EXPECT_FALSE(worker.doActionMerge(sorceInfo, targetInfo, true).isValid());

    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(worker.checkLinkAndSameUrl(targetInfo, targetInfo, false).toBool());
    EXPECT_FALSE(worker.doActionReplace(sorceInfo, targetInfo, true).toBool());
    EXPECT_TRUE(worker.doActionMerge(sorceInfo, targetInfo, true).isValid());
}

TEST_F(UT_FileOperateBaseWorker, testDoCopyFile)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);
    QProcess::execute("touch sourceUrl.txt");
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    auto targetInfo = InfoFactory::create<FileInfo>(targetUrl);
    auto sorceInfo = InfoFactory::create<FileInfo>(sorceUrl);
    bool skip{false};
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.doCopyFile(sorceInfo, targetInfo, &skip));
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kCoexistAction; });
    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsSymLink || type == OptInfoType::kIsDir) {
            return true;
        }
        return false;});
    targetInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(QDir::currentPath()));
    targetInfo->refresh();
    qInfo() << targetInfo->isAttributes(OptInfoType::kIsDir);
    stub.set_lamda(&FileOperateBaseWorker::createSystemLink,[]{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doCopyFile(sorceInfo, targetInfo, &skip));

    QProcess::execute("rm sourceUrl.txt");
}
