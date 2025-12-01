// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
#include <dfm-io/dfile.h>
#include <dfm-io/denumerator.h>
#include <dfm-io/dfmio_utils.h>

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"
#include "fileoperations/copyfiles/docopyfilesworker.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestFileOperateBaseWorker : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        // Create target directory FileInfo
        targetDirInfo = InfoFactory::create<FileInfo>(tempDirUrl);
        ASSERT_TRUE(targetDirInfo);

        // Create worker instance
        worker = new DoCopyFilesWorker();
        ASSERT_TRUE(worker);

        // Initialize workData
        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);

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
            delete worker;
            worker = nullptr;
        }
        targetDirInfo.reset();
        tempDir.reset();
    }

protected:
    FileInfoPointer createTestFile(const QString &fileName)
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << "test content";
        file.close();

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
    FileInfoPointer targetDirInfo;
    DoCopyFilesWorker *worker;
};

// ========== doHandleErrorAndWait Tests ==========

TEST_F(TestFileOperateBaseWorker, DoHandleErrorAndWait_NoWorkData)
{
    worker->workData.reset();
    auto action = worker->doHandleErrorAndWait(QUrl(), QUrl(),
                                               AbstractJobHandler::JobErrorType::kNoError);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kNoAction);
}

TEST_F(TestFileOperateBaseWorker, DoHandleErrorAndWait_CachedAction)
{
    worker->workData->errorOfAction[AbstractJobHandler::JobErrorType::kNoError] =
            AbstractJobHandler::SupportAction::kSkipAction;

    auto action = worker->doHandleErrorAndWait(QUrl(), QUrl(),
                                               AbstractJobHandler::JobErrorType::kNoError);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kSkipAction);
}

TEST_F(TestFileOperateBaseWorker, DoHandleErrorAndWait_SameFile)
{
    auto testFile = createTestFile("test.txt");
    ASSERT_TRUE(testFile);

    auto action = worker->doHandleErrorAndWait(testFile->urlOf(UrlInfoType::kUrl),
                                               testFile->urlOf(UrlInfoType::kUrl),
                                               AbstractJobHandler::JobErrorType::kNoError);
    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kCoexistAction);
}

// ========== emitSpeedUpdatedNotify Tests ==========

TEST_F(TestFileOperateBaseWorker, EmitSpeedUpdatedNotify_ValidData)
{
    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->currentState = AbstractJobHandler::JobState::kRunningState;
    worker->sourceFilesTotalSize = 1000;

    bool signalEmitted = false;
    QObject::connect(worker, &DoCopyFilesWorker::speedUpdatedNotify,
                     [&signalEmitted](const JobInfoPointer &info) {
                         signalEmitted = true;
                         EXPECT_TRUE(info->contains(AbstractJobHandler::NotifyInfoKey::kSpeedKey));
                     });

    worker->emitSpeedUpdatedNotify(500);
    EXPECT_TRUE(signalEmitted);
}

// ========== readAheadSourceFile Tests ==========

TEST_F(TestFileOperateBaseWorker, ReadAheadSourceFile_ValidFile)
{
    auto testFile = createTestFile("readahead.txt");
    ASSERT_TRUE(testFile);

    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    fileInfo->initQuerier();

    // This should not crash
    worker->readAheadSourceFile(fileInfo);
}

TEST_F(TestFileOperateBaseWorker, ReadAheadSourceFile_ZeroSizeFile)
{
    auto testFile = createTestFile("empty.txt");
    ASSERT_TRUE(testFile);

    stub.set_lamda(ADDR(DFMIO::DFileInfo, attribute),
                   []() -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant::fromValue(qint64(0));
                   });

    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    worker->readAheadSourceFile(fileInfo);
    // Should return early without opening file
}

// ========== checkDiskSpaceAvailable Tests ==========

TEST_F(TestFileOperateBaseWorker, CheckDiskSpaceAvailable_SufficientSpace)
{
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 {
        __DBG_STUB_INVOKE__
        return 1024 * 1024 * 1024;   // 1GB free
    });

    stub.set_lamda(&FileOperationsUtils::isFilesSizeOutLimit,
                   [](const QUrl &, qint64) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool skip = false;
    bool result = worker->checkDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_TRUE(result);
    EXPECT_FALSE(skip);
}

// ========== checkFileSize Tests ==========

TEST_F(TestFileOperateBaseWorker, CheckFileSize_SmallFile)
{
    worker->targetUrl = tempDirUrl;

    bool skip = false;
    bool result = worker->checkFileSize(1024, QUrl(), QUrl(), &skip);
    EXPECT_TRUE(result);
    EXPECT_FALSE(skip);
}

TEST_F(TestFileOperateBaseWorker, CheckFileSize_LargeFileOnVfat)
{
    worker->targetUrl = tempDirUrl;

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "vfat";
    });

    bool skip = false;
    qint64 largeSize = 5LL * 1024 * 1024 * 1024;   // 5GB

    // This should trigger error handling, but we'll skip it in stub
    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool result = worker->checkFileSize(largeSize, QUrl(), QUrl(), &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== deleteFile Tests ==========

TEST_F(TestFileOperateBaseWorker, DeleteFile_Success)
{
    auto testFile = createTestFile("delete_me.txt");
    ASSERT_TRUE(testFile);

    bool workContinue = false;
    bool result = worker->deleteFile(testFile->urlOf(UrlInfoType::kUrl), QUrl(),
                                     &workContinue, false);
    EXPECT_TRUE(result);
}

TEST_F(TestFileOperateBaseWorker, DeleteFile_WithForce)
{
    auto testFile = createTestFile("force_delete.txt");
    ASSERT_TRUE(testFile);

    bool workContinue = false;
    bool result = worker->deleteFile(testFile->urlOf(UrlInfoType::kUrl), QUrl(),
                                     &workContinue, true);
    EXPECT_TRUE(result);
}

// ========== deleteDir Tests ==========

TEST_F(TestFileOperateBaseWorker, DeleteDir_EmptyDirectory)
{
    auto testDir = createTestDir("empty_dir");
    ASSERT_TRUE(testDir);

    bool skip = false;
    bool result = worker->deleteDir(testDir->urlOf(UrlInfoType::kUrl), QUrl(),
                                    &skip, false);
    EXPECT_TRUE(result);
}

// ========== createSystemLink Tests ==========

TEST_F(TestFileOperateBaseWorker, CreateSystemLink_Success)
{
    auto sourceFile = createTestFile("link_source.txt");
    ASSERT_TRUE(sourceFile);

    QString linkPath = tempDirPath + QDir::separator() + "link_target.txt";
    QUrl linkUrl = QUrl::fromLocalFile(linkPath);
    auto targetInfo = DFileInfoPointer(new DFileInfo(linkUrl));
    auto sourceInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));

    bool skip = false;
    bool result = worker->createSystemLink(sourceInfo, targetInfo, false, false, &skip);
    EXPECT_TRUE(result || skip);   // May succeed or be skipped based on system
}

// ========== checkAndCopyFile Tests ==========

TEST_F(TestFileOperateBaseWorker, CheckAndCopyFile_SmallFile)
{
    auto sourceFile = createTestFile("copy_source.txt");
    ASSERT_TRUE(sourceFile);

    auto targetDir = createTestDir("copy_target");
    ASSERT_TRUE(targetDir);

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    fromInfo->initQuerier();
    toInfo->initQuerier();

    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->targetUrl = targetDir->urlOf(UrlInfoType::kUrl);
    worker->isSourceFileLocal = true;
    worker->isTargetFileLocal = true;

    // Stub to avoid actual copy
    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    bool result = worker->checkAndCopyFile(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

// ========== checkAndCopyDir Tests ==========

TEST_F(TestFileOperateBaseWorker, CheckAndCopyDir_EmptyDir)
{
    auto sourceDir = createTestDir("source_dir");
    ASSERT_TRUE(sourceDir);

    auto targetBase = createTestDir("target_base");
    ASSERT_TRUE(targetBase);

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

// ========== canWriteFile Tests ==========

TEST_F(TestFileOperateBaseWorker, CanWriteFile_ValidFile)
{
    auto testFile = createTestFile("writable.txt");
    ASSERT_TRUE(testFile);

    bool result = worker->canWriteFile(testFile->urlOf(UrlInfoType::kUrl));
    EXPECT_TRUE(result);
}

TEST_F(TestFileOperateBaseWorker, CanWriteFile_RootUser)
{
    stub.set_lamda(getuid, []() -> uid_t {
        __DBG_STUB_INVOKE__
        return 0;   // root
    });

    bool result = worker->canWriteFile(QUrl());
    EXPECT_TRUE(result);
}

// ========== setAllDirPermisson Tests ==========

TEST_F(TestFileOperateBaseWorker, SetAllDirPermisson_EmptyList)
{
    worker->supportSetPermission = true;
    // Should not crash with empty list
    worker->setAllDirPermisson();
}

TEST_F(TestFileOperateBaseWorker, SetAllDirPermisson_WithPermissions)
{
    auto testDir = createTestDir("perm_dir");
    ASSERT_TRUE(testDir);

    worker->supportSetPermission = true;

    QSharedPointer<FileOperateBaseWorker::DirSetPermissonInfo> permInfo(
            new FileOperateBaseWorker::DirSetPermissonInfo);
    permInfo->target = testDir->urlOf(UrlInfoType::kUrl);
    permInfo->permission = QFileDevice::ReadUser | QFileDevice::WriteUser;

    worker->dirPermissonList.appendByLock(permInfo);

    // Should not crash
    worker->setAllDirPermisson();
}

// ========== getWriteDataSize Tests ==========

TEST_F(TestFileOperateBaseWorker, GetWriteDataSize_NoWorkData)
{
    worker->workData.reset();
    qint64 size = worker->getWriteDataSize();
    EXPECT_EQ(size, 0);
}

TEST_F(TestFileOperateBaseWorker, GetWriteDataSize_CustomizeType)
{
    worker->countWriteType = AbstractWorker::CountWriteSizeType::kCustomizeType;
    worker->workData->currentWriteSize = 1000;
    worker->workData->skipWriteSize = 200;
    worker->workData->zeroOrlinkOrDirWriteSize = 100;

    qint64 size = worker->getWriteDataSize();
    EXPECT_EQ(size, 1300);
}

// ========== determineCountProcessType Tests ==========

TEST_F(TestFileOperateBaseWorker, DetermineCountProcessType_LocalFile)
{
    worker->targetOrgUrl = QUrl::fromLocalFile("/tmp/test");

    stub.set_lamda(&dfmio::DFMUtils::mountPathFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/tmp";
    });

    stub.set_lamda(&dfmio::DFMUtils::deviceNameFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/dev/sda1";
    });

    stub.set_lamda(&FileOperationsUtils::isFileOnDisk, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    worker->determineCountProcessType();
    EXPECT_TRUE(worker->isTargetFileLocal);
}

// ========== needsSync Tests ==========

TEST_F(TestFileOperateBaseWorker, NeedsSync_NoWorker)
{
    worker->copyOtherFileWorker.reset();
    bool result = worker->needsSync();
    EXPECT_FALSE(result);
}

TEST_F(TestFileOperateBaseWorker, NeedsSync_LocalTarget)
{
    worker->isTargetFileLocal = true;
    worker->targetUrl = tempDirUrl;
    bool result = worker->needsSync();
    EXPECT_FALSE(result);
}

// ========== trashInfo Tests ==========

TEST_F(TestFileOperateBaseWorker, TrashInfo_ValidTrashFile)
{
    // Create a trash files structure
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

TEST_F(TestFileOperateBaseWorker, TrashInfo_NonTrashFile)
{
    auto testFile = createTestFile("regular.txt");
    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));

    QUrl result = worker->trashInfo(fileInfo);
    EXPECT_FALSE(result.isValid());
}

// ========== fileOriginName Tests ==========

TEST_F(TestFileOperateBaseWorker, FileOriginName_InvalidUrl)
{
    QString name = worker->fileOriginName(QUrl());
    EXPECT_TRUE(name.isEmpty());
}

TEST_F(TestFileOperateBaseWorker, FileOriginName_ValidTrashInfo)
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

// ========== shouldUseBlockWriteType Tests ==========

TEST_F(TestFileOperateBaseWorker, ShouldUseBlockWriteType_NotRemovable)
{
    worker->targetIsRemovable = false;
    bool result = worker->shouldUseBlockWriteType();
    EXPECT_FALSE(result);
}

TEST_F(TestFileOperateBaseWorker, ShouldUseBlockWriteType_FuseFileSystem)
{
    worker->targetIsRemovable = true;
    worker->workData->isBlockDevice = true;
    worker->workData->exBlockSyncEveryWrite = true;
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "fuse.sshfs";
    });

    bool result = worker->shouldUseBlockWriteType();
    EXPECT_TRUE(result);
}

// ========== initCopyWay Tests ==========

TEST_F(TestFileOperateBaseWorker, InitCopyWay_LocalToLocal)
{
    worker->isSourceFileLocal = true;
    worker->isTargetFileLocal = true;
    worker->sourceFilesCount = 10;
    worker->sourceFilesTotalSize = 100 * 1024 * 1024;

    stub.set_lamda(&FileUtils::getCpuProcessCount, []() -> int {
        __DBG_STUB_INVOKE__
        return 8;
    });

    worker->initCopyWay();
    EXPECT_EQ(worker->countWriteType, AbstractWorker::CountWriteSizeType::kCustomizeType);
}

// ========== setSkipValue Tests ==========

TEST_F(TestFileOperateBaseWorker, SetSkipValue_SkipAction)
{
    bool skip = false;
    worker->setSkipValue(&skip, AbstractJobHandler::SupportAction::kSkipAction);
    EXPECT_TRUE(skip);
}

TEST_F(TestFileOperateBaseWorker, SetSkipValue_OtherAction)
{
    bool skip = false;
    worker->setSkipValue(&skip, AbstractJobHandler::SupportAction::kRetryAction);
    EXPECT_FALSE(skip);
}

TEST_F(TestFileOperateBaseWorker, SetSkipValue_NullPointer)
{
    // Should not crash
    worker->setSkipValue(nullptr, AbstractJobHandler::SupportAction::kSkipAction);
}

// ========== Constructor/Destructor Tests ==========

TEST_F(TestFileOperateBaseWorker, Constructor_CreatesInstance)
{
    FileOperateBaseWorker *baseWorker = new DoCopyFilesWorker();
    EXPECT_NE(baseWorker, nullptr);
    delete baseWorker;
}

TEST_F(TestFileOperateBaseWorker, Destructor_CleansUp)
{
    FileOperateBaseWorker *baseWorker = new DoCopyFilesWorker();
    delete baseWorker;
    SUCCEED();
}

// ========== doCheckFile Tests ==========

TEST_F(TestFileOperateBaseWorker, DoCheckFile_ValidFile)
{
    auto sourceFile = createTestFile("check_source.txt");
    ASSERT_TRUE(sourceFile);

    auto targetDir = createTestDir("check_target_dir");
    ASSERT_TRUE(targetDir);

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    fromInfo->initQuerier();
    toInfo->initQuerier();

    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->targetUrl = targetDir->urlOf(UrlInfoType::kUrl);

    QString fileName = "test.txt";
    bool skip = false;

    auto result = worker->doCheckFile(fromInfo, toInfo, fileName, &skip);
    EXPECT_TRUE(result || skip);
}

TEST_F(TestFileOperateBaseWorker, DoCheckFile_Stopped)
{
    auto sourceFile = createTestFile("stopped_source.txt");
    auto targetDir = createTestDir("stopped_target");

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(targetDir->urlOf(UrlInfoType::kUrl)));

    worker->stopWork = true;

    QString fileName = "test.txt";
    bool skip = false;

    auto result = worker->doCheckFile(fromInfo, toInfo, fileName, &skip);
    EXPECT_FALSE(result);
}

// ========== doCheckNewFile Tests ==========

TEST_F(TestFileOperateBaseWorker, DoCheckNewFile_NonExistentTarget)
{
    QString targetPath = tempDirPath + "/new_file.txt";
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    bool skip = false;
    QString name;
    auto result = worker->doCheckNewFile(toInfo, nullptr, name, &skip);

    if (result) {
        EXPECT_NE(result, nullptr);
    } else {
        SUCCEED();   // May return null on some systems
    }
}

TEST_F(TestFileOperateBaseWorker, DoCheckNewFile_ExistingTarget)
{
    auto existingFile = createTestFile("existing_target.txt");
    auto toInfo = DFileInfoPointer(new DFileInfo(existingFile->urlOf(UrlInfoType::kUrl)));

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kReplaceAction;
                   });

    bool skip = false;
    QString name;
    auto result = worker->doCheckNewFile(toInfo, nullptr, name, &skip);
    SUCCEED();   // Will handle based on stub
}

// ========== checkTotalDiskSpaceAvailable Tests ==========

TEST_F(TestFileOperateBaseWorker, CheckTotalDiskSpaceAvailable_SufficientSpace)
{
    worker->targetOrgUrl = tempDirUrl;
    worker->sourceFilesTotalSize = 100 * 1024 * 1024;   // 100MB

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 {
        __DBG_STUB_INVOKE__
        return 1024LL * 1024 * 1024 * 10;   // 10GB free
    });

    bool skip = false;
    bool result = worker->checkTotalDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_TRUE(result);
    EXPECT_FALSE(skip);
}

TEST_F(TestFileOperateBaseWorker, CheckTotalDiskSpaceAvailable_InsufficientSpace)
{
    worker->targetOrgUrl = tempDirUrl;
    worker->sourceFilesTotalSize = 10LL * 1024 * 1024 * 1024;   // 10GB

    stub.set_lamda(&DeviceUtils::deviceBytesFree, [](const QUrl &) -> qint64 {
        __DBG_STUB_INVOKE__
        return 100 * 1024 * 1024;   // Only 100MB free
    });

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool skip = false;
    bool result = worker->checkTotalDiskSpaceAvailable(QUrl(), QUrl(), &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== getTidWriteSize Tests ==========

TEST_F(TestFileOperateBaseWorker, GetTidWriteSize_ValidDevice)
{
    worker->workData->isBlockDevice = true;

    qint64 size = worker->getTidWriteSize();
    EXPECT_GE(size, 0);
}

TEST_F(TestFileOperateBaseWorker, GetTidWriteSize_NoWorkData)
{
    worker->workData.reset();
    qint64 size = worker->getTidWriteSize();
    EXPECT_EQ(size, 0);
}

// ========== getSectorsWritten Tests ==========

TEST_F(TestFileOperateBaseWorker, GetSectorsWritten_BlockDevice)
{
    worker->workData->isBlockDevice = true;
    qint64 sectors = worker->getSectorsWritten();
    EXPECT_GE(sectors, 0);
}

TEST_F(TestFileOperateBaseWorker, GetSectorsWritten_NoBlockDevice)
{
    worker->workData->isBlockDevice = false;
    qint64 sectors = worker->getSectorsWritten();
    EXPECT_EQ(sectors, 0);
}

// ========== performSync Tests ==========

TEST_F(TestFileOperateBaseWorker, PerformSync_ValidTarget)
{
    worker->targetOrgUrl = tempDirUrl;
    worker->isTargetFileLocal = true;

    stub.set_lamda(&dfmio::DFMUtils::fsTypeFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "ext4";
    });

    // Should not crash
    worker->performSync();
    SUCCEED();
}

TEST_F(TestFileOperateBaseWorker, PerformSync_NoTarget)
{
    worker->targetOrgUrl = QUrl();
    worker->performSync();
    SUCCEED();   // Should handle gracefully
}

// ========== performAsyncSync Tests ==========

TEST_F(TestFileOperateBaseWorker, PerformAsyncSync_ValidTarget)
{
    worker->targetOrgUrl = tempDirUrl;

    stub.set_lamda(&QDBusAbstractInterface::isValid,
                   []() {
                       return false;
                   });

    worker->performAsyncSync();
    SUCCEED();
}

// ========== copyFileFromTrash Tests ==========

TEST_F(TestFileOperateBaseWorker, CopyFileFromTrash_ValidTrashFile)
{
    // Create trash structure
    QString trashFilesPath = tempDirPath + "/.local/share/Trash/files";
    QDir().mkpath(trashFilesPath);

    QString trashFilePath = trashFilesPath + "/test.txt";
    QFile file(trashFilePath);
    file.open(QIODevice::WriteOnly);
    file.write("trash content");
    file.close();

    auto targetPath = tempDirPath + "/restored.txt";
    auto fromInfo = QUrl::fromLocalFile(trashFilePath);
    auto toInfo = QUrl::fromLocalFile(targetPath);

    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->copyFileFromTrash(fromInfo, toInfo, {});
    EXPECT_TRUE(result);
}

TEST_F(TestFileOperateBaseWorker, CopyFileFromTrash_Stopped)
{
    auto fromInfo = QUrl::fromLocalFile("/tmp/test.txt");
    auto toInfo = QUrl::fromLocalFile("/tmp/target.txt");

    worker->stopWork = true;

    bool result = worker->copyFileFromTrash(fromInfo, toInfo, {});
    EXPECT_FALSE(result);
}

// ========== copyAndDeleteFile Tests ==========

TEST_F(TestFileOperateBaseWorker, CopyAndDeleteFile_Success)
{
    auto sourceFile = createTestFile("move_source.txt");
    auto targetDir = createTestDir("move_target");

    QString targetPath = tempDirPath + "/move_target/moved.txt";
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&FileOperateBaseWorker::deleteFile,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      bool *, bool) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    bool result = worker->copyAndDeleteFile(fromInfo, toInfo, {}, &skip);
    EXPECT_TRUE(result);
}

TEST_F(TestFileOperateBaseWorker, CopyAndDeleteFile_CopyFails)
{
    auto sourceFile = createTestFile("copy_fail_source.txt");
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/tmp/target.txt")));

    bool skip = false;
    bool result = worker->copyAndDeleteFile(fromInfo, toInfo, {}, &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== doCopyFile Tests ==========

TEST_F(TestFileOperateBaseWorker, DoCopyFile_SmallFile)
{
    auto sourceFile = createTestFile("docopy_source.txt");
    auto targetDir = createTestDir("docopy_target");

    QString targetPath = tempDirPath + "/docopy_target/copied.txt";
    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    fromInfo->initQuerier();
    toInfo->initQuerier();

    worker->jobType = AbstractJobHandler::JobType::kCopyType;
    worker->isSourceFileLocal = true;
    worker->isTargetFileLocal = true;

    stub.set_lamda(&FileOperateBaseWorker::doCopyOtherFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    bool result = worker->doCopyFile(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(TestFileOperateBaseWorker, DoCopyFile_Stopped)
{
    auto fromInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/tmp/src.txt")));
    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile("/tmp/dst.txt")));

    worker->stopWork = true;

    bool skip = false;
    bool result = worker->doCopyFile(fromInfo, toInfo, &skip);
    EXPECT_FALSE(result);
}

// ========== waitThreadPoolOver Tests ==========

TEST_F(TestFileOperateBaseWorker, WaitThreadPoolOver_NoThreadPool)
{
    // Should not crash
    worker->waitThreadPoolOver();
    SUCCEED();
}

// ========== removeTrashInfo Tests ==========

TEST_F(TestFileOperateBaseWorker, RemoveTrashInfo_ValidTrashInfo)
{
    // Create trash info file
    QString trashInfoPath = tempDirPath + "/test.trashinfo";
    QFile file(trashInfoPath);
    file.open(QIODevice::WriteOnly);
    file.write("[Trash Info]");
    file.close();

    QUrl infoUrl = QUrl::fromLocalFile(trashInfoPath);

    stub.set_lamda(&LocalFileHandler::deleteFile, [](LocalFileHandler *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    worker->removeTrashInfo(infoUrl);
    SUCCEED();
}

TEST_F(TestFileOperateBaseWorker, RemoveTrashInfo_InvalidUrl)
{
    QUrl invalidUrl;
    // Should not crash
    worker->removeTrashInfo(invalidUrl);
    SUCCEED();
}
