// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-io/dfmio_utils.h>

#include <gtest/gtest.h>
#include <dfm-io/denumerator.h>

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
    worker.setTargetPermissions(url, url);
    QProcess::execute("rm ./testSyncFileInfo.txt");
}

TEST_F(UT_FileOperateBaseWorker, testReadAheadSourceFile)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch ./testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");

    FileOperateBaseWorker worker;
    DFileInfoPointer info(new DFileInfo(url));
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
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperationsUtils::isFilesSizeOutLimit, []{  __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.checkDiskSpaceAvailable(url, url, nullptr));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kNoAction; });
    EXPECT_TRUE(worker.checkDiskSpaceAvailable(url, url, nullptr));
}

TEST_F(UT_FileOperateBaseWorker, testCheckFileSize)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(worker.checkFileSize(0, url, url, nullptr));

    bool skip{false};
    worker.targetUrl = url;
    stub_ext::StubExt stub;
    stub.set_lamda(&DFMUtils::fsTypeFromUrl, []{ __DBG_STUB_INVOKE__ return QByteArray("lll");});
    EXPECT_TRUE(worker.checkFileSize(0, url, url, nullptr));

    stub.set_lamda(&DFMUtils::fsTypeFromUrl, []{ __DBG_STUB_INVOKE__ return QByteArray("vfat");});
    EXPECT_TRUE(worker.checkFileSize(0, url, url, nullptr));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kEnforceAction; });
    EXPECT_TRUE(worker.checkFileSize(5l * 1024 * 1024 * 1024, url, url, &skip));

    worker.workData.reset(new WorkerData);
    WorkerData::BlockFileCopyInfo info;
    WorkerData::BlockFileCopyInfo info2(info);
    info.buffer = new char[10];
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.checkFileSize(5l * 1024 * 1024 * 1024, url, url, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kNoAction; });
    EXPECT_TRUE(worker.checkFileSize(5l * 1024 * 1024 * 1024, url, url, &skip));
}

TEST_F(UT_FileOperateBaseWorker, testDeleteDir)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    int index = 0;
    stub.set_lamda(&DEnumerator::hasNext, [&index]{ __DBG_STUB_INVOKE__ index++; return index <= 1;});
    stub.set_lamda(&DEnumerator::next, [url]{ __DBG_STUB_INVOKE__ return url;});
    stub.set_lamda(&FileOperateBaseWorker::deleteFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&LocalFileHandler::setPermissions, []{ __DBG_STUB_INVOKE__ return false;});
    worker.localFileHandler.reset(new LocalFileHandler);
    EXPECT_TRUE(worker.deleteDir(url, url, nullptr, true));

    stub.set_lamda(&FileOperateBaseWorker::deleteFile, []{ __DBG_STUB_INVOKE__ return false;});
    index = 0;
    stub.set_lamda(&DEnumerator::hasNext, [&index]{ __DBG_STUB_INVOKE__ index++; return index <= 1;});
    stub.set_lamda(&DFileInfo::attribute, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.deleteDir(url, url, nullptr, true));
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

bool doCheckFileFunc(FileOperateBaseWorker *&, const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo, const QString &fileName,
                 FileInfoPointer &newTargetInfo, bool *skip) {
    __DBG_STUB_INVOKE__
    newTargetInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(QDir::currentPath()));
    return true;
}

TEST_F(UT_FileOperateBaseWorker, testCopyFileFromTrash)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DFile::exists, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&LocalFileHandler::mkdir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));

    stub.set_lamda(&DFile::exists, []{ __DBG_STUB_INVOKE__ return true;});
//    EXPECT_FALSE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));

    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);

    int index = 0;
    stub.set_lamda(VADDR(LocalDirIterator, hasNext), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));

    stub.set_lamda(VADDR(LocalDirIterator, hasNext), [&index]{ __DBG_STUB_INVOKE__ index++; return index <= 1;});
    stub.set_lamda(VADDR(LocalDirIterator, next), [url]{ __DBG_STUB_INVOKE__ return url;});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_TRUE(worker.copyFileFromTrash(url, QUrl(), DFile::CopyFlag::kOverwrite));

    index = 0;
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kCancelAction; });
    EXPECT_FALSE(worker.copyFileFromTrash(url, QUrl(), DFile::CopyFlag::kOverwrite));

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    EXPECT_TRUE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));

    stub.set(&FileOperateBaseWorker::doCheckFile, doCheckFileFunc);
    index = 0;

    EXPECT_TRUE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));

    int index1 = 0;
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [&index1](SyncFileInfo *, const dfmbase::FileInfo::FileIsType type){
        __DBG_STUB_INVOKE__
        index1++;
        if (index1 > 1 && type == dfmbase::FileInfo::FileIsType::kIsDir)
            return false;
        return true;
    });
    index = 0;
    stub.set_lamda(&LocalFileHandler::copyFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));

    index = 0;
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyFileFromTrash(url, url, DFile::CopyFlag::kOverwrite));


}

TEST_F(UT_FileOperateBaseWorker, testCopyAndDeleteFile)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    DFileInfoPointer fromInfo(new DFileInfo(url));
    worker.workData.reset(new WorkerData);
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperateBaseWorker::checkDiskSpaceAvailable, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyAndDeleteFile(fromInfo, nullptr, nullptr, nullptr));

    stub.set_lamda(&FileOperateBaseWorker::checkDiskSpaceAvailable, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(worker.copyAndDeleteFile(fromInfo, nullptr, nullptr, nullptr));

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [](SyncFileInfo *, const dfmbase::FileInfo::FileIsType type){
        __DBG_STUB_INVOKE__
        if ( type == dfmbase::FileInfo::FileIsType::kIsSymLink)
            return true;
        return false;
    });
    stub.set_lamda(&FileOperateBaseWorker::createSystemLink, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::deleteFile, []{ __DBG_STUB_INVOKE__ return true;});
    bool skip{false};
    worker.targetInfo = fromInfo;
    EXPECT_TRUE(worker.copyAndDeleteFile(fromInfo, fromInfo, fromInfo, &skip));

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [](SyncFileInfo *, const dfmbase::FileInfo::FileIsType type){
        __DBG_STUB_INVOKE__
        if ( type == dfmbase::FileInfo::FileIsType::kIsDir)
            return true;
        return false;
    });

    stub.set_lamda(&FileOperateBaseWorker::checkAndCopyDir, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::deleteDir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyAndDeleteFile(fromInfo, fromInfo, fromInfo, &skip));

    stub.set_lamda(VADDR(SyncFileInfo, isAttributes),[]{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&FileOperateBaseWorker::checkFileSize, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.copyAndDeleteFile(fromInfo, fromInfo, fromInfo, &skip));

    stub.set_lamda(&FileOperateBaseWorker::checkFileSize, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&DoCopyFileWorker::doCopyFilePractically, []{ __DBG_STUB_INVOKE__ return DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;});
    stub.set_lamda(&DoCopyFileWorker::doDfmioFileCopy, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.copyAndDeleteFile(fromInfo, fromInfo, fromInfo, &skip));

}

TEST_F(UT_FileOperateBaseWorker, testDoCheckFile)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "testDoCheckFile.txt");
    DFileInfoPointer fromInfo(new DFileInfo(url));
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction;});
    FileInfoPointer newInfo;
    EXPECT_FALSE(worker.doCheckFile(nullptr, nullptr, QString(), nullptr));

    EXPECT_FALSE(worker.doCheckFile(fromInfo, nullptr, QString(), nullptr));

    stub.set_lamda(VADDR(SyncFileInfo, exists), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(worker.doCheckFile(fromInfo, nullptr, QString(), nullptr));

    int index = 0;
    stub.set_lamda(VADDR(SyncFileInfo, exists), [&index]{ __DBG_STUB_INVOKE__ index++; return index > 1;});
    EXPECT_FALSE(worker.doCheckFile(fromInfo, fromInfo, QString(), nullptr));

    stub.set_lamda(VADDR(SyncFileInfo, exists), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(SyncFileInfo, fileType), []{ __DBG_STUB_INVOKE__ return FileInfo::FileType::kSocketFile;});
    bool skip(false);
    worker.workData.reset(new WorkerData);
    EXPECT_FALSE(worker.doCheckFile(fromInfo, fromInfo, QString(), &skip));

    stub.set_lamda(VADDR(SyncFileInfo, fileType), []{ __DBG_STUB_INVOKE__ return FileInfo::FileType::kDocuments;});
    index = 0;
    stub.set_lamda(VADDR(SyncFileInfo, exists), [&index]{ __DBG_STUB_INVOKE__ index++; return index < 3;});
    stub.set_lamda(&FileUtils::isTrashFile, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::doCheckNewFile, []{ __DBG_STUB_INVOKE__ return nullptr;});
    EXPECT_FALSE(worker.doCheckFile(fromInfo, fromInfo, QString(), &skip));
}

TEST_F(UT_FileOperateBaseWorker, testCreateSystemLink)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator());
    worker.workData.reset(new WorkerData);
    DFileInfoPointer fileInfo(new DFileInfo(url));
    bool skip(false);
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperateBaseWorker::checkAndCopyFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&FileOperateBaseWorker::checkAndCopyDir, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.createSystemLink(fileInfo, nullptr, true, true, &skip));

    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsFile) {
            return true;
        }
        return false;});
    EXPECT_FALSE(worker.createSystemLink(fileInfo, nullptr, true, true, &skip));

    stub.set_lamda(&LocalFileHandler::createSystemLink, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.createSystemLink(fileInfo, fileInfo, true, false, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction;});
    stub.set_lamda(&LocalFileHandler::createSystemLink, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&LocalFileHandler::errorString, []{ __DBG_STUB_INVOKE__ return QString();});
    EXPECT_FALSE(worker.createSystemLink(fileInfo, fileInfo, true, false, &skip));
}
bool createNewTargetInfoFunc(FileOperateBaseWorker *&, const FileInfoPointer &fromInfo, const FileInfoPointer &toInfo,
                         FileInfoPointer &newTargetInfo, const QUrl &fileNewUrl,
                         bool *skip, bool isCountSize = false){
    newTargetInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(QDir::currentPath()));
    return false;
}

TEST_F(UT_FileOperateBaseWorker, testDoCheckNewFile)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    worker.workData.reset(new WorkerData);
    DFileInfoPointer fileInfo(new DFileInfo(url));
    bool skip(false);
    stub_ext::StubExt stub;
    FileInfoPointer newInfo(nullptr);
    QString fileBaseName;
    stub.set_lamda(&FileOperateBaseWorker::createNewTargetUrl, []{ __DBG_STUB_INVOKE__ return QUrl();});
    EXPECT_TRUE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperationsUtils::isAncestorUrl, []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperationsUtils::isAncestorUrl, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kReplaceAction;});
    stub.set_lamda(&FileOperateBaseWorker::doActionReplace, []{ __DBG_STUB_INVOKE__ return QVariant();});
    EXPECT_TRUE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doActionReplace, []{ __DBG_STUB_INVOKE__ return QVariant(true);});
    EXPECT_TRUE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kCoexistAction;});
    stub.set_lamda(&FileUtils::nonExistFileName, []{ __DBG_STUB_INVOKE__ return QString();});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileUtils::nonExistFileName, []{ __DBG_STUB_INVOKE__ return QString("pp");});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kMergeAction;});
    stub.set_lamda(&FileOperateBaseWorker::doActionMerge, []{ __DBG_STUB_INVOKE__ return QVariant();});
    EXPECT_TRUE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doActionMerge, []{ __DBG_STUB_INVOKE__ return QVariant(true);});
    EXPECT_TRUE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction;});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kCancelAction;});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kNoAction;});
    EXPECT_FALSE(worker.doCheckNewFile(fileInfo, fileInfo, fileBaseName, &skip));
}

TEST_F(UT_FileOperateBaseWorker, testCheckAndCopyFile)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    worker.workData.reset(new WorkerData);
    DFileInfoPointer fileInfo(new DFileInfo(url));
    bool skip(false);
    stub_ext::StubExt stub;
    stub.set_lamda(&FileOperateBaseWorker::checkFileSize, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.checkAndCopyFile(fileInfo, fileInfo, &skip));

    stub.set_lamda(&FileOperateBaseWorker::checkFileSize, []{ __DBG_STUB_INVOKE__ return true;});
    worker.jobType = AbstractJobHandler::JobType::kCutType;
    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.checkAndCopyFile(fileInfo, fileInfo, &skip));

    worker.jobType = AbstractJobHandler::JobType::kCopyType;
    worker.isSourceFileLocal = true;
    worker.isTargetFileLocal = true;
    worker.workData->signalThread = false;
    stub.set_lamda(VADDR(SyncFileInfo, size), []{ __DBG_STUB_INVOKE__ return 100 * 1024 *1024;});
    stub.set_lamda(&FileOperateBaseWorker::doCopyLocalByRange, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(worker.checkAndCopyFile(fileInfo, fileInfo, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doCopyLocalFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(VADDR(SyncFileInfo, size), []{ __DBG_STUB_INVOKE__ return 1 * 1024 *1024;});
    EXPECT_FALSE(worker.checkAndCopyFile(fileInfo, fileInfo, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile, []{ __DBG_STUB_INVOKE__ return false;});
    worker.workData->signalThread = true;
    EXPECT_FALSE(worker.checkAndCopyFile(fileInfo, fileInfo, &skip));
}

TEST_F(UT_FileOperateBaseWorker, testCheckAndCopyDir)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    worker.workData.reset(new WorkerData);
    DFileInfoPointer fileInfo(new DFileInfo(url));
    bool skip(false);
    stub_ext::StubExt stub;
    auto toUrl = QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "testCheckAndCopyDir");
    DFileInfoPointer toInfo(new DFileInfo(toUrl));
    stub.set_lamda(&LocalFileHandler::mkdir, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&LocalFileHandler::errorString, []{ __DBG_STUB_INVOKE__ return QString();});
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kSkipAction;});
    worker.localFileHandler.reset(new LocalFileHandler);
    EXPECT_FALSE(worker.checkAndCopyDir(fileInfo, toInfo, &skip));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   []{ __DBG_STUB_INVOKE__ return AbstractJobHandler::SupportAction::kNoAction;});
    worker.workData->jobFlags |= AbstractJobHandler::JobFlag::kCopyToSelf;
    EXPECT_TRUE(worker.checkAndCopyDir(fileInfo, toInfo, &skip));

    stub.set_lamda(&LocalFileHandler::mkdir, []{ __DBG_STUB_INVOKE__ return true;});
    DirIteratorFactory::instance().constructList.clear();
    DirIteratorFactory::instance().constructAguList.clear();
    EXPECT_FALSE(worker.checkAndCopyDir(fileInfo, fileInfo, &skip));

    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    stub.set_lamda(VADDR(LocalDirIterator, hasNext), []{ __DBG_STUB_INVOKE__ return true;});
    worker.stop();
    EXPECT_FALSE(worker.checkAndCopyDir(fileInfo, fileInfo, &skip));

    worker.resume();
    stub.set_lamda(&FileOperateBaseWorker::doCopyFile, []{ __DBG_STUB_INVOKE__ return false; });
    skip = false;
    EXPECT_FALSE(worker.checkAndCopyDir(fileInfo, fileInfo, &skip));

    int index = 0;
    stub.set_lamda(VADDR(LocalDirIterator, hasNext), [&index]{ __DBG_STUB_INVOKE__ index++; return index < 2;});
    skip = true;
    stub.set_lamda(&FileUtils::isMtpFile, []{ __DBG_STUB_INVOKE__ return false; });
    EXPECT_TRUE(worker.checkAndCopyDir(fileInfo, fileInfo, &skip));

    index = 0;
    worker.isTargetFileLocal = true;
    worker.isSourceFileLocal = true;
    EXPECT_TRUE(worker.checkAndCopyDir(fileInfo, fileInfo, &skip));

    worker.workData->jobFlags |= AbstractJobHandler::JobFlag::kCountProgressCustomize;
    worker.initCopyWay();
}

TEST_F(UT_FileOperateBaseWorker, testTrashInfo)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    DFileInfoPointer fileInfo(new DFileInfo(url));
    EXPECT_TRUE(worker.trashInfo(fileInfo).isEmpty());
    url.setPath(StandardPaths::location(StandardPaths::kHomePath) + "/.local/share/Trash/files/testttt.txt");
    fileInfo.reset(new DFileInfo(url));
    EXPECT_TRUE(worker.trashInfo(fileInfo).path().endsWith("testttt.txt.trashinfo"));
}

TEST_F(UT_FileOperateBaseWorker, testFileOriginName)
{
    FileOperateBaseWorker worker;
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    EXPECT_TRUE(worker.fileOriginName(QUrl()).isEmpty());

    url.setPath(StandardPaths::location(StandardPaths::kHomePath) + "/.local/share/Trash/files/testttt.txt");
    DFileInfoPointer fileInfo(new DFileInfo(url));
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
    DFileInfoPointer fileInfo(new DFileInfo(url));
    auto targetUrl = worker.createNewTargetUrl(fileInfo, fileName);
    EXPECT_EQ(targetUrl.fileName(), fileName);
    DFileInfoPointer newfileInfo(new DFileInfo(url));

    worker.currentState = AbstractJobHandler::JobState::kStopState;
    EXPECT_FALSE(worker.doCopyLocalFile(newfileInfo, fileInfo));

    worker.currentState = AbstractJobHandler::JobState::kRunningState;
    EXPECT_TRUE(worker.doCopyLocalFile(newfileInfo, fileInfo));
    worker.stopAllThread();
}

int openTest(int fd, __off_t offset) {
    Q_UNUSED(fd);
    Q_UNUSED(offset);
    __DBG_STUB_INVOKE__
    return -1;
};

TEST_F(UT_FileOperateBaseWorker, testCreateNewTargetInfo)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath());
    DFileInfoPointer targetInfo(new DFileInfo(targetUrl));
    DFileInfoPointer sorceInfo(new DFileInfo(sorceUrl));

    FileInfoPointer newTargetInfo{nullptr};
    bool skip{false};

    worker.initThreadCopy();
    worker.skipMemcpyBigFile(sorceUrl);
}

TEST_F(UT_FileOperateBaseWorker, testCheckLinkAndSameUrl)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;

    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    auto targetUrl = QUrl::fromLocalFile(QDir::currentPath() + "/targetUrl.txt");
    DFileInfoPointer targetInfo(new DFileInfo(targetUrl));
    DFileInfoPointer sorceInfo(new DFileInfo(sorceUrl));


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
    DFileInfoPointer targetInfo(new DFileInfo(targetUrl));
    DFileInfoPointer sorceInfo(new DFileInfo(sorceUrl));
    bool skip{false};
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kSkipAction; });
    EXPECT_FALSE(worker.doCopyFile(sorceInfo, targetInfo, &skip));
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait, []{ __DBG_STUB_INVOKE__  return AbstractJobHandler::SupportAction::kCoexistAction; });
    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsSymLink || type == OptInfoType::kIsDir) {
            return true;
        }
        return false;});
    targetInfo.reset(new DFileInfo(QUrl::fromLocalFile(QDir::currentPath())));
    targetInfo->refresh();
    stub.set_lamda(&FileOperateBaseWorker::createSystemLink,[]{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(worker.doCopyFile(sorceInfo, targetInfo, &skip));

    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
        if (type == OptInfoType::kIsDir) {
            return true;
        }
        return false;});
    stub.set_lamda(&FileOperateBaseWorker::checkAndCopyDir,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,[]{ __DBG_STUB_INVOKE__ return nullptr;});
    worker.targetInfo = targetInfo;
    EXPECT_TRUE(worker.doCopyFile(sorceInfo, targetInfo, &skip));

    stub.set_lamda(VADDR(SyncFileInfo,isAttributes), [](SyncFileInfo *,OptInfoType type){ __DBG_STUB_INVOKE__
        return false;});
    stub.set_lamda(&FileOperateBaseWorker::checkAndCopyFile,[]{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,[]{ __DBG_STUB_INVOKE__ return nullptr;});
    worker.targetInfo = targetInfo;
    EXPECT_TRUE(worker.doCopyFile(sorceInfo, targetInfo, &skip));

    QProcess::execute("rm sourceUrl.txt");
}

int getuidTest (void){
    __DBG_STUB_INVOKE__
    return 0;
}

TEST_F(UT_FileOperateBaseWorker, testCanWriteFile)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    auto sorceUrl = QUrl::fromLocalFile(QDir::currentPath() + "/sourceUrl.txt");
    stub.set(::getuid,getuidTest);
    EXPECT_TRUE(worker.canWriteFile(sorceUrl));

    stub.clear();
    EXPECT_FALSE(worker.canWriteFile(QUrl()));

    EXPECT_TRUE(worker.canWriteFile(sorceUrl));
}

TEST_F(UT_FileOperateBaseWorker, testSetAllDirPermisson)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    if (!worker.localFileHandler)
        worker.localFileHandler.reset(new LocalFileHandler);
    QSharedPointer<FileOperateBaseWorker::DirSetPermissonInfo> info(new FileOperateBaseWorker::DirSetPermissonInfo);
    worker.dirPermissonList.push_backByLock(info);
    stub.set_lamda(&LocalFileHandler::setPermissions, []{ __DBG_STUB_INVOKE__ return false;});
    worker.setAllDirPermisson();
}

TEST_F(UT_FileOperateBaseWorker, testGetWriteDataSize)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    worker.workData.reset(new WorkerData);
    worker.countWriteType = AbstractWorker::CountWriteSizeType::kTidType;
    EXPECT_TRUE(0 == worker.getWriteDataSize());

    worker.countWriteType = AbstractWorker::CountWriteSizeType::kCustomizeType;
    EXPECT_TRUE(0 == worker.getWriteDataSize());

    worker.countWriteType = AbstractWorker::CountWriteSizeType::kWriteBlockType;
    stub.set_lamda(&FileOperateBaseWorker::getSectorsWritten,[]{ __DBG_STUB_INVOKE__ return 10;});
    EXPECT_TRUE(10 * 512 == worker.getWriteDataSize());
}

TEST_F(UT_FileOperateBaseWorker, testGetTidWriteSize)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;
    typedef bool (*openfile)(QFile *,QFile::OpenMode);
    stub.set_lamda((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)),[](){return true;});
    stub.set_lamda(VADDR(QFile, close), []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(QFile, readAll), []{ __DBG_STUB_INVOKE__ return QByteArray("write_bytes: 1000"); });
    EXPECT_TRUE(1000 == worker.getTidWriteSize());

    stub.set_lamda(VADDR(QFile, readAll), []{ __DBG_STUB_INVOKE__ return QByteArray(); });
    EXPECT_TRUE(0 == worker.getTidWriteSize());
}

TEST_F(UT_FileOperateBaseWorker, testGetSectorsWritten)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;

    EXPECT_TRUE(0 == worker.getSectorsWritten());

    typedef bool (*openfile)(QFile *,QFile::OpenMode);
    stub.set_lamda((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)),[](){return true;});
    stub.set_lamda(VADDR(QFile, close), []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(VADDR(QFile, readAll), []{ __DBG_STUB_INVOKE__ return QByteArray("sdf dd dd dd ff oo 99 00"); });
    EXPECT_TRUE(99 == worker.getSectorsWritten());

    auto url = QUrl::fromLocalFile(QDir::currentPath());
    worker.targetUrl = url;
    worker.determineCountProcessType();

    stub.set_lamda(&DFMUtils::fsTypeFromUrl,[]{ __DBG_STUB_INVOKE__ return QByteArray();});
    worker.determineCountProcessType();
}

TEST_F(UT_FileOperateBaseWorker, testSyncFilesToDevice)
{
    FileOperateBaseWorker worker;
    stub_ext::StubExt stub;

    worker.countWriteType = AbstractWorker::CountWriteSizeType::kCustomizeType;
    worker.syncFilesToDevice();

    int index = 0;
    stub.set_lamda(&FileOperateBaseWorker::isStopped,[&index]{
        __DBG_STUB_INVOKE__
        index++;
        if (index % 2)
            return false;
        return true;
    });
    worker.sourceFilesTotalSize = 1;
    worker.countWriteType = AbstractWorker::CountWriteSizeType::kWriteBlockType;
    stub.set_lamda(&FileOperateBaseWorker::getWriteDataSize,[]{ __DBG_STUB_INVOKE__ return 0;});
    worker.syncFilesToDevice();
}
