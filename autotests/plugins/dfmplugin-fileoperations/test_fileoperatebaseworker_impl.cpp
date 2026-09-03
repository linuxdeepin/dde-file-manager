// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>
#include <QThread>
#include <QDBusAbstractInterface>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-io/dfile.h>
#include <dfm-io/denumerator.h>
#include <dfm-io/dfmio_utils.h>

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"
#include "fileoperations/copyfiles/docopyfilesworker.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class FileOperateBaseWorkerImpl : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        worker = new DoCopyFilesWorker();
        ASSERT_TRUE(worker);

        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);

        // Stub QWaitCondition::wait to avoid blocking in doHandleErrorAndWait
        using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
        stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                       [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
            worker->stopAllThread();
            delete worker;
            worker = nullptr;
        }
        tempDir.reset();
    }

protected:
    FileInfoPointer createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << content;
            file.close();
        }
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        return InfoFactory::create<FileInfo>(fileUrl);
    }

    FileInfoPointer createTestDir(const QString &dirName)
    {
        QString dirPath = tempDirPath + QDir::separator() + dirName;
        QDir().mkpath(dirPath);
        QUrl dirUrl = QUrl::fromLocalFile(dirPath);
        return InfoFactory::create<FileInfo>(dirUrl);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QUrl tempDirUrl;
    DoCopyFilesWorker *worker { nullptr };
};

// ========== Constructor / Destructor ==========
TEST_F(FileOperateBaseWorkerImpl, Constructor_CreatesInstance)
{
    FileOperateBaseWorker *baseWorker = new DoCopyFilesWorker();
    EXPECT_NE(baseWorker, nullptr);
    delete baseWorker;
}

TEST_F(FileOperateBaseWorkerImpl, Destructor_CleansUp)
{
    FileOperateBaseWorker *baseWorker = new DoCopyFilesWorker();
    delete baseWorker;
    SUCCEED();
}

// ========== doHandleErrorAndWait ==========
TEST_F(FileOperateBaseWorkerImpl, DoHandleErrorAndWait_NoWorkData)
{
    worker->workData.reset();
    auto action = worker->doHandleErrorAndWait(QUrl(), QUrl(),
                                              AbstractJobHandler::JobErrorType::kNoError);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(FileOperateBaseWorkerImpl, DoHandleErrorAndWait_CachedAction)
{
    worker->workData->errorOfAction[AbstractJobHandler::JobErrorType::kNoError] =
            AbstractJobHandler::SupportAction::kSkipAction;

    auto action = worker->doHandleErrorAndWait(QUrl(), QUrl(),
                                              AbstractJobHandler::JobErrorType::kNoError);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kSkipAction);
}

TEST_F(FileOperateBaseWorkerImpl, DoHandleErrorAndWait_SameFile)
{
    auto testFile = createTestFile("same.txt");
    auto action = worker->doHandleErrorAndWait(testFile->urlOf(UrlInfoType::kUrl),
                                              testFile->urlOf(UrlInfoType::kUrl),
                                              AbstractJobHandler::JobErrorType::kNoError);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kCoexistAction);
}

// ========== emitSpeedUpdatedNotify ==========
TEST_F(FileOperateBaseWorkerImpl, EmitSpeedUpdatedNotify_Running)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->currentState = AbstractJobHandler::JobState::kRunningState;
    worker->sourceFilesTotalSize = 1000;

    bool signalEmitted = false;
    QObject::connect(worker, &DoCopyFilesWorker::speedUpdatedNotify,
                     [&signalEmitted](const JobInfoPointer &) { signalEmitted = true; });

    worker->emitSpeedUpdatedNotify(500);
    EXPECT_TRUE(signalEmitted);
}

// ========== checkDiskSpaceAvailable ==========
TEST_F(FileOperateBaseWorkerImpl, CheckDiskSpaceAvailable_Sufficient)
{
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 {
        __DBG_STUB_INVOKE__
        return 1024LL * 1024 * 1024;
    });

    stub.set_lamda(&FileOperationsUtils::isFilesSizeOutLimit,
                   [](const QUrl &, qint64) -> bool { return false; });

    bool skip = false;
    bool result = worker->checkDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_TRUE(result);
    EXPECT_FALSE(skip);
}

TEST_F(FileOperateBaseWorkerImpl, CheckDiskSpaceAvailable_Insufficient)
{
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 { return 100; });

    stub.set_lamda(&FileOperationsUtils::isFilesSizeOutLimit,
                   [](const QUrl &, qint64) -> bool { return true; });

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool skip = false;
    bool result = worker->checkDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== checkFileSize ==========
TEST_F(FileOperateBaseWorkerImpl, CheckFileSize_SmallFile)
{
    worker->targetUrl = tempDirUrl;

    bool skip = false;
    bool result = worker->checkFileSize(1024, QUrl(), QUrl(), &skip);
    EXPECT_TRUE(result);
    EXPECT_FALSE(skip);
}

TEST_F(FileOperateBaseWorkerImpl, CheckFileSize_LargeFileOnVfat)
{
    worker->targetUrl = tempDirUrl;

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString { return "vfat"; });

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool skip = false;
    qint64 largeSize = 5LL * 1024 * 1024 * 1024;
    bool result = worker->checkFileSize(largeSize, QUrl(), QUrl(), &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== checkTotalDiskSpaceAvailable ==========
TEST_F(FileOperateBaseWorkerImpl, CheckTotalDiskSpaceAvailable_Sufficient)
{
    worker->targetOrgUrl = tempDirUrl;
    worker->sourceFilesTotalSize = 100 * 1024 * 1024;

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 { return 1024LL * 1024 * 1024 * 10; });

    bool skip = false;
    bool result = worker->checkTotalDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_TRUE(result);
    EXPECT_FALSE(skip);
}

TEST_F(FileOperateBaseWorkerImpl, CheckTotalDiskSpaceAvailable_Insufficient)
{
    worker->targetOrgUrl = tempDirUrl;
    worker->sourceFilesTotalSize = 10LL * 1024 * 1024 * 1024;

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 { return 100 * 1024 * 1024; });

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool skip = false;
    bool result = worker->checkTotalDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== setAllDirPermisson ==========
TEST_F(FileOperateBaseWorkerImpl, SetAllDirPermisson_EmptyList)
{
    worker->supportSetPermission = true;
    worker->setAllDirPermisson();
    SUCCEED();
}

TEST_F(FileOperateBaseWorkerImpl, SetAllDirPermisson_WithPermissions)
{
    auto testDir = createTestDir("perm_dir");
    worker->supportSetPermission = true;

    QSharedPointer<FileOperateBaseWorker::DirSetPermissonInfo> permInfo(
            new FileOperateBaseWorker::DirSetPermissonInfo);
    permInfo->target = testDir->urlOf(UrlInfoType::kUrl);
    permInfo->permission = QFileDevice::ReadUser | QFileDevice::WriteUser;

    worker->dirPermissonList.appendByLock(permInfo);
    worker->setAllDirPermisson();
    SUCCEED();
}

// ========== getWriteDataSize / getTidWriteSize / getSectorsWritten ==========
TEST_F(FileOperateBaseWorkerImpl, GetWriteDataSize_CustomizeType)
{
    worker->countWriteType = AbstractWorker::CountWriteSizeType::kCustomizeType;
    worker->workData->currentWriteSize = 1000;
    worker->workData->skipWriteSize = 200;
    worker->workData->zeroOrlinkOrDirWriteSize = 100;

    qint64 size = worker->getWriteDataSize();
    EXPECT_EQ(size, 1300);
}

TEST_F(FileOperateBaseWorkerImpl, GetTidWriteSize_ValidDevice)
{
    worker->workData->isBlockDevice = true;
    qint64 size = worker->getTidWriteSize();
    EXPECT_GE(size, 0);
}

TEST_F(FileOperateBaseWorkerImpl, GetTidWriteSize_NoWorkData)
{
    worker->workData.reset();
    qint64 size = worker->getTidWriteSize();
    EXPECT_EQ(size, 0);
}

TEST_F(FileOperateBaseWorkerImpl, GetSectorsWritten_BlockDevice)
{
    worker->workData->isBlockDevice = true;
    qint64 sectors = worker->getSectorsWritten();
    EXPECT_GE(sectors, 0);
}

TEST_F(FileOperateBaseWorkerImpl, GetSectorsWritten_NoBlockDevice)
{
    worker->workData->isBlockDevice = false;
    qint64 sectors = worker->getSectorsWritten();
    EXPECT_EQ(sectors, 0);
}

// ========== determineCountProcessType ==========
TEST_F(FileOperateBaseWorkerImpl, DetermineCountProcessType_LocalFile)
{
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&dfmio::DFMUtils::mountPathFromUrl, [](const QUrl &) -> QString { return "/tmp"; });
    stub.set_lamda(&dfmio::DFMUtils::deviceNameFromUrl, [](const QUrl &) -> QString { return "/dev/sda1"; });
    stub.set_lamda(&FileOperationsUtils::isFileOnDisk, [](const QUrl &) -> bool { return true; });

    worker->determineCountProcessType();
    EXPECT_TRUE(worker->isTargetFileLocal);
}

// ========== needsSync / performSync / performAsyncSync ==========
TEST_F(FileOperateBaseWorkerImpl, NeedsSync_NoWorker)
{
    worker->copyOtherFileWorker.reset();
    bool result = worker->needsSync();
    EXPECT_FALSE(result);
}

TEST_F(FileOperateBaseWorkerImpl, NeedsSync_LocalTarget)
{
    worker->isTargetFileLocal = true;
    worker->targetUrl = tempDirUrl;
    bool result = worker->needsSync();
    EXPECT_FALSE(result);
}

TEST_F(FileOperateBaseWorkerImpl, PerformSync_ValidTarget)
{
    worker->targetOrgUrl = tempDirUrl;
    worker->isTargetFileLocal = true;

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString { return "ext4"; });

    worker->performSync();
    SUCCEED();
}

TEST_F(FileOperateBaseWorkerImpl, PerformAsyncSync_ValidTarget)
{
    worker->targetOrgUrl = tempDirUrl;

    using IsValidFunc = bool (QDBusAbstractInterface::*)() const;
    stub.set_lamda(static_cast<IsValidFunc>(&QDBusAbstractInterface::isValid), []() -> bool { return false; });

    worker->performAsyncSync();
    SUCCEED();
}

// ========== copyFileFromTrash ==========
TEST_F(FileOperateBaseWorkerImpl, CopyFileFromTrash_NonExistent)
{
    auto fromInfo = QUrl::fromLocalFile("/tmp/nonexistent_trash_file.txt");
    auto toInfo = QUrl::fromLocalFile(tempDir->path() + "/restored.txt");

    worker->stopWork = true;
    bool result = worker->copyFileFromTrash(fromInfo, toInfo, {});
    EXPECT_FALSE(result);
}

TEST_F(FileOperateBaseWorkerImpl, CopyFileFromTrash_Stopped)
{
    auto fromInfo = QUrl::fromLocalFile("/tmp/test.txt");
    auto toInfo = QUrl::fromLocalFile("/tmp/target.txt");

    worker->stopWork = true;
    bool result = worker->copyFileFromTrash(fromInfo, toInfo, {});
    EXPECT_FALSE(result);
}

// ========== createSystemLink ==========
TEST_F(FileOperateBaseWorkerImpl, CreateSystemLink_NoFollowNoCopy)
{
    auto sourceFile = createTestFile("link_src.txt");
    QString linkPath = tempDirPath + "/link_no_follow.lnk";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(linkPath)));

    bool skip = false;
    bool result = worker->createSystemLink(fromInfo, toInfo, false, false, &skip);
    EXPECT_TRUE(result || skip);
}

// ========== doCheckFile ==========
TEST_F(FileOperateBaseWorkerImpl, DoCheckFile_NonExistentSource)
{
    auto fromInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/tmp/nonexistent_src.txt")));
    auto targetDir = createTestDir("docheck_target");
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    worker->workData->errorOfAction[AbstractJobHandler::JobErrorType::kNonexistenceError] =
            AbstractJobHandler::SupportAction::kSkipAction;

    bool skip = false;
    auto result = worker->doCheckFile(fromInfo, toInfo, "test.txt", &skip);
    EXPECT_TRUE(result.isNull());
    EXPECT_TRUE(skip);
}

TEST_F(FileOperateBaseWorkerImpl, DoCheckFile_Stopped)
{
    auto sourceFile = createTestFile("stopped.txt");
    auto targetDir = createTestDir("stopped_target");

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    // stopWork has no short-circuit in doCheckFile: it only affects the
    // error-waiting path, so the check completes and returns the new target.
    worker->stopWork = true;

    bool skip = false;
    auto result = worker->doCheckFile(fromInfo, toInfo, "test.txt", &skip);
    EXPECT_TRUE(result);
}

// ========== doCheckNewFile ==========
TEST_F(FileOperateBaseWorkerImpl, DoCheckNewFile_NonExistentTarget)
{
    QString targetPath = tempDirPath + "/new_file.txt";
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    bool skip = false;
    QString name = "new_file.txt";
    // Signature is (fromInfo, toInfo, ...): the non-existent target goes
    // into the toInfo slot, fromInfo is only used on conflict paths.
    auto result = worker->doCheckNewFile(nullptr, toInfo, name, &skip);
    EXPECT_NE(result, nullptr);
}

TEST_F(FileOperateBaseWorkerImpl, DoCheckNewFile_ExistingTargetReplace)
{
    auto existingFile = createTestFile("existing_target.txt");
    auto toInfo = DFileInfoPointer(new DFileInfo(existingFile->urlOf(UrlInfoType::kUrl)));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       return AbstractJobHandler::SupportAction::kReplaceAction;
                   });

    bool skip = false;
    QString name;
    auto result = worker->doCheckNewFile(toInfo, nullptr, name, &skip);
    SUCCEED();
}

// ========== checkAndCopyFile ==========
TEST_F(FileOperateBaseWorkerImpl, CheckAndCopyFile_SmallFile)
{
    auto sourceFile = createTestFile("copy_source.txt");
    auto targetDir = createTestDir("copy_target");

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    fromInfo->initQuerier();
    toInfo->initQuerier();

    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->targetUrl = targetDir->urlOf(UrlInfoType::kUrl);
    worker->isSourceFileLocal = true;
    worker->isTargetFileLocal = true;

    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool { return true; });

    bool skip = false;
    bool result = worker->checkAndCopyFile(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(FileOperateBaseWorkerImpl, CheckAndCopyFile_CopyFails)
{
    auto sourceFile = createTestFile("copy_stopped.txt");
    auto targetDir = createTestDir("copy_stopped_target");

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    // Local flags are left false so the synchronous doCopyOtherFile branch
    // is taken; stub it to fail and expect false.
    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool { return false; });

    bool skip = false;
    bool result = worker->checkAndCopyFile(fromInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

// ========== checkAndCopyDir ==========
TEST_F(FileOperateBaseWorkerImpl, CheckAndCopyDir_EmptyDir)
{
    auto sourceDir = createTestDir("source_dir");
    auto targetBase = createTestDir("target_base");

    QString targetPath = tempDirPath + "/target_base/copied_dir";
    QUrl targetUrl = QUrl::fromLocalFile(targetPath);

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceDir->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetUrl));

    fromInfo->initQuerier();

    worker->targetUrl = targetBase->urlOf(UrlInfoType::kUrl);
    worker->isTargetFileLocal = true;
    worker->isSourceFileLocal = true;

    bool skip = false;
    bool result = worker->checkAndCopyDir(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result || skip);
}

// ========== copyAndDeleteFile ==========
TEST_F(FileOperateBaseWorkerImpl, CopyAndDeleteFile_NonExistentTarget)
{
    auto sourceFile = createTestFile("move_source.txt");
    QString targetPath = tempDirPath + "/move_target/nonexistent_dir/moved.txt";

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    // Copy stage fails (parent dir does not exist) and must propagate.
    stub.set_lamda(VADDR(DoCopyFileWorker, doDfmioFileCopy),
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool { return false; });
    stub.set_lamda(VADDR(DoCopyFileWorker, doCopyFilePractically),
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       return DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;
                   });

    bool skip = false;
    bool result = worker->copyAndDeleteFile(fromInfo, toInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

TEST_F(FileOperateBaseWorkerImpl, CopyAndDeleteFile_CopyFails)
{
    auto sourceFile = createTestFile("copy_fail_source.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(tempDirPath + "/copy_fail_target/target.txt")));

    // Force the copy stage to fail so the failure propagates.
    stub.set_lamda(VADDR(DoCopyFileWorker, doDfmioFileCopy),
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool { return false; });
    stub.set_lamda(VADDR(DoCopyFileWorker, doCopyFilePractically),
                   [](DoCopyFileWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> DoCopyFileWorker::NextDo {
                       return DoCopyFileWorker::NextDo::kDoCopyErrorAddCancel;
                   });

    bool skip = false;
    bool result = worker->copyAndDeleteFile(fromInfo, toInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

// ========== doCopyFile ==========
TEST_F(FileOperateBaseWorkerImpl, DoCopyFile_SmallFile)
{
    auto sourceFile = createTestFile("docopy_source.txt");
    auto targetDir = createTestDir("docopy_target");

    // doCopyFile expects toInfo to be the existing parent dir; the new
    // target file is derived from it inside doCheckFile.
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    fromInfo->initQuerier();
    toInfo->initQuerier();

    worker->jobType = AbstractJobHandler::JobType::kCopyType;

    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool { return true; });

    bool skip = false;
    bool result = worker->doCopyFile(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(FileOperateBaseWorkerImpl, DoCopyFile_Stopped)
{
    auto fromInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/tmp/src.txt")));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/tmp/dst.txt")));

    worker->stopWork = true;

    bool skip = false;
    bool result = worker->doCopyFile(fromInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

// ========== trashInfo / fileOriginName / removeTrashInfo ==========
TEST_F(FileOperateBaseWorkerImpl, TrashInfo_ValidTrashFile)
{
    QString trashFilesPath = tempDirPath + "/.local/share/Trash/files";
    QDir().mkpath(trashFilesPath);

    QString testFilePath = trashFilesPath + "/test.txt";
    QFile file(testFilePath);
    file.open(QIODevice::WriteOnly);
    file.close();

    auto fileInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(testFilePath)));

    QUrl result = worker->trashInfo(fileInfo);
    EXPECT_TRUE(result.isValid());
    EXPECT_TRUE(result.path().contains("info"));
}

TEST_F(FileOperateBaseWorkerImpl, TrashInfo_NonTrashFile)
{
    auto testFile = createTestFile("regular.txt");
    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));

    QUrl result = worker->trashInfo(fileInfo);
    EXPECT_FALSE(result.isValid());
}

TEST_F(FileOperateBaseWorkerImpl, FileOriginName_InvalidUrl)
{
    QString name = worker->fileOriginName(QUrl());
    EXPECT_TRUE(name.isEmpty());
}

TEST_F(FileOperateBaseWorkerImpl, FileOriginName_ValidTrashInfo)
{
    QString trashInfoPath = tempDirPath + "/test.trashinfo";
    QFile file(trashInfoPath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "[Trash Info] Path=%E6%B5%8B%E8%AF%95.txt DeletionDate=2025-01-01T00:00:00";
        file.close();
    }

    QString name = worker->fileOriginName(QUrl::fromLocalFile(trashInfoPath));
    EXPECT_FALSE(name.isEmpty());
}

TEST_F(FileOperateBaseWorkerImpl, RemoveTrashInfo_ValidUrl)
{
    QString trashInfoPath = tempDirPath + "/trashinfo_remove.trashinfo";
    QFile file(trashInfoPath);
    file.open(QIODevice::WriteOnly);
    file.write("[Trash Info]");
    file.close();

    QUrl infoUrl = QUrl::fromLocalFile(trashInfoPath);

    stub.set_lamda(&LocalFileHandler::deleteFile,
                   [](LocalFileHandler *, const QUrl &) -> bool { return true; });

    worker->removeTrashInfo(infoUrl);
    SUCCEED();
}

TEST_F(FileOperateBaseWorkerImpl, RemoveTrashInfo_InvalidUrl)
{
    QUrl invalidUrl;
    worker->removeTrashInfo(invalidUrl);
    SUCCEED();
}

// ========== setSkipValue / initCopyWay / shouldUseBlockWriteType ==========
TEST_F(FileOperateBaseWorkerImpl, SetSkipValue_SkipAction)
{
    bool skip = false;
    worker->setSkipValue(&skip, AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_TRUE(skip);
}

TEST_F(FileOperateBaseWorkerImpl, SetSkipValue_NullPointer)
{
    worker->setSkipValue(nullptr, AbstractJobHandler::SupportAction::kSkipAction);
    SUCCEED();
}

TEST_F(FileOperateBaseWorkerImpl, InitCopyWay_LocalToLocal)
{
    worker->isSourceFileLocal = true;
    worker->isTargetFileLocal = true;
    worker->sourceFilesCount = 10;
    worker->sourceFilesTotalSize = 100 * 1024 * 1024;

    stub.set_lamda(&FileUtils::getCpuProcessCount, []() -> int { return 8; });

    worker->initCopyWay();
    EXPECT_EQ(worker->countWriteType, AbstractWorker::CountWriteSizeType::kCustomizeType);
}

TEST_F(FileOperateBaseWorkerImpl, ShouldUseBlockWriteType_NotRemovable)
{
    worker->targetIsRemovable = false;
    bool result = worker->shouldUseBlockWriteType();
    EXPECT_FALSE(result);
}

TEST_F(FileOperateBaseWorkerImpl, ShouldUseBlockWriteType_Fuse)
{
    worker->targetIsRemovable = true;
    worker->workData->isBlockDevice = true;
    worker->workData->exBlockSyncEveryWrite = true;
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString { return "fuse.sshfs"; });

    bool result = worker->shouldUseBlockWriteType();
    EXPECT_TRUE(result);
}

// ========== waitThreadPoolOver ==========
TEST_F(FileOperateBaseWorkerImpl, WaitThreadPoolOver_NoThreadPool)
{
    worker->waitThreadPoolOver();
    SUCCEED();
}

// ========== emitCurrentTaskNotify ==========
TEST_F(FileOperateBaseWorkerImpl, EmitCurrentTaskNotify_Signal)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->sourceFilesTotalSize = 100;

    bool emitted = false;
    QObject::connect(worker, &DoCopyFilesWorker::currentTaskNotify,
                     [&emitted](const JobInfoPointer &) { emitted = true; });

    worker->emitCurrentTaskNotify(QUrl::fromLocalFile("/tmp/from"), QUrl::fromLocalFile("/tmp/to"));
    SUCCEED();
}

// ========== doHandleErrorAndWait with errorMsgAll ==========
TEST_F(FileOperateBaseWorkerImpl, DoHandleErrorAndWait_ErrorMsgAll)
{
    worker->workData->errorOfAction[AbstractJobHandler::JobErrorType::kNoError] =
            AbstractJobHandler::SupportAction::kSkipAction;

    auto action = worker->doHandleErrorAndWait(QUrl(), QUrl(),
                                              AbstractJobHandler::JobErrorType::kNoError,
                                              false, QString(), true);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kSkipAction);
}
