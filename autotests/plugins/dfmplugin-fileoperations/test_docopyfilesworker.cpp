// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>
#include <QSignalSpy>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-io/dfile.h>

#include "fileoperations/copyfiles/docopyfilesworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoCopyFilesWorker : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        // Create temporary directories
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        targetDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(targetDir->isValid());

        targetDirPath = targetDir->path();
        targetDirUrl = QUrl::fromLocalFile(targetDirPath);

        // Create worker instance
        worker = new DoCopyFilesWorker();
        ASSERT_TRUE(worker);

        // Initialize workData
        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);
        worker->targetUrl = targetDirUrl;
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
            delete worker;
            worker = nullptr;
        }
        targetDir.reset();
        tempDir.reset();
    }

protected:
    FileInfoPointer createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << content;
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
    std::unique_ptr<QTemporaryDir> targetDir;
    QString tempDirPath;
    QString targetDirPath;
    QUrl tempDirUrl;
    QUrl targetDirUrl;
    DoCopyFilesWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoCopyFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kCopyType);
}

TEST_F(TestDoCopyFilesWorker, Destructor_CallsStop)
{
    bool stopCalled = false;
    stub.set_lamda(VADDR(DoCopyFilesWorker, stop), [&stopCalled](DoCopyFilesWorker *) {
        __DBG_STUB_INVOKE__
        stopCalled = true;
    });

    delete worker;
    worker = nullptr;
    EXPECT_TRUE(stopCalled);
}

// ========== initArgs Tests ==========

TEST_F(TestDoCopyFilesWorker, InitArgs_EmptySourceUrls)
{
    worker->sourceUrls.clear();

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &error, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(error, AbstractJobHandler::JobErrorType::kProrogramError);
                       return AbstractJobHandler::SupportAction::kNoAction;
                   });

    bool result = worker->initArgs();
    EXPECT_FALSE(result);
}

TEST_F(TestDoCopyFilesWorker, InitArgs_InvalidTargetUrl)
{
    auto sourceFile = createTestFile("source.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = QUrl();

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &error, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(error, AbstractJobHandler::JobErrorType::kProrogramError);
                       return AbstractJobHandler::SupportAction::kNoAction;
                   });

    bool result = worker->initArgs();
    EXPECT_FALSE(result);
}

TEST_F(TestDoCopyFilesWorker, InitArgs_NonExistentTarget)
{
    auto sourceFile = createTestFile("source.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = QUrl::fromLocalFile("/nonexistent/path");

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &error, const bool isTo,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(error, AbstractJobHandler::JobErrorType::kNonexistenceError);
                       EXPECT_TRUE(isTo);
                       return AbstractJobHandler::SupportAction::kNoAction;
                   });

    bool result = worker->initArgs();
    EXPECT_FALSE(result);
}

TEST_F(TestDoCopyFilesWorker, InitArgs_ValidArguments)
{
    auto sourceFile = createTestFile("source.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;

    stub.set_lamda(VADDR(AbstractWorker, initArgs), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->initArgs();
    EXPECT_TRUE(result);
    EXPECT_TRUE(worker->targetInfo);
}

TEST_F(TestDoCopyFilesWorker, InitArgs_SymlinkTarget)
{
    auto sourceFile = createTestFile("source.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;

    stub.set_lamda(VADDR(AbstractWorker, initArgs), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMIO::DFileInfo::attribute,
                   [](DFileInfo *, DFileInfo::AttributeID id, bool) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (id == DFileInfo::AttributeID::kStandardIsSymlink)
                           return true;
                       if (id == DFileInfo::AttributeID::kStandardSymlinkTarget)
                           return "/real/path";
                       return QVariant();
                   });

    bool result = worker->initArgs();
    EXPECT_TRUE(result);
}

// ========== copyFiles Tests ==========

TEST_F(TestDoCopyFilesWorker, CopyFiles_SingleFile)
{
    auto sourceFile = createTestFile("copy_me.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->copyFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCopyFilesWorker, CopyFiles_DirectoryToSelf)
{
    auto sourceDir = createTestDir("self_copy");
    worker->sourceUrls.append(sourceDir->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = sourceDir->urlOf(UrlInfoType::kUrl);
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool signalEmitted = false;
    QObject::connect(worker, &DoCopyFilesWorker::requestShowTipsDialog,
                     [&signalEmitted](DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType type,
                                      const QList<QUrl> &) {
                         signalEmitted = true;
                         EXPECT_EQ(type, DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType::kCopyMoveToSelf);
                     });

    bool result = worker->copyFiles();
    EXPECT_FALSE(result);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestDoCopyFilesWorker, CopyFiles_DirectoryToParent)
{
    auto sourceDir = createTestDir("parent/child");
    QUrl parentUrl = QUrl::fromLocalFile(tempDirPath + "/parent");
    worker->sourceUrls.append(sourceDir->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = parentUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::isHigherHierarchy,
                   [](const QUrl &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool signalEmitted = false;
    QObject::connect(worker, &DoCopyFilesWorker::requestShowTipsDialog,
                     [&signalEmitted](DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType,
                                      const QList<QUrl> &) {
                         signalEmitted = true;
                     });

    bool result = worker->copyFiles();
    EXPECT_FALSE(result);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(TestDoCopyFilesWorker, CopyFiles_WithSkip)
{
    auto sourceFile = createTestFile("skip_me.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *skip) -> bool {
                       __DBG_STUB_INVOKE__
                       *skip = true;
                       return false;
                   });

    bool result = worker->copyFiles();
    EXPECT_TRUE(result);   // Should continue even with skip
}

TEST_F(TestDoCopyFilesWorker, CopyFiles_MultipleFiles)
{
    worker->sourceUrls.append(createTestFile("file1.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("file2.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("file3.txt")->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    int copyCount = 0;
    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   [&copyCount](FileOperateBaseWorker *, const DFileInfoPointer &,
                                const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       copyCount++;
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->copyFiles();
    EXPECT_TRUE(result);
    EXPECT_EQ(copyCount, 3);
}

// ========== doWork Tests ==========

TEST_F(TestDoCopyFilesWorker, DoWork_RemoteCopy)
{
    worker->sourceUrls.clear();
    worker->workData->jobFlags = AbstractJobHandler::JobFlag::kCopyRemote;

    QList<QUrl> remoteUrls;
    remoteUrls.append(QUrl("file:///remote/file.txt"));

    stub.set_lamda(ADDR(ClipBoard, getRemoteUrls),
                   [&remoteUrls]() -> QList<QUrl> {
                       __DBG_STUB_INVOKE__
                       return remoteUrls;
                   });

    bool signalEmitted = false;
    QObject::connect(worker, &DoCopyFilesWorker::requestTaskDailog,
                     [&signalEmitted]() {
                         signalEmitted = true;
                     });

    stub.set_lamda(VADDR(AbstractWorker, doWork), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileOperateBaseWorker::determineCountProcessType,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&FileOperateBaseWorker::checkTotalDiskSpaceAvailable,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperateBaseWorker::initCopyWay,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&DoCopyFilesWorker::copyFiles,
                   [](DoCopyFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(DoCopyFilesWorker, endWork),
                   [](DoCopyFilesWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    worker->doWork();
    EXPECT_TRUE(signalEmitted);
    EXPECT_FALSE(worker->sourceUrls.isEmpty());
}

TEST_F(TestDoCopyFilesWorker, DoWork_InsufficientDiskSpace)
{
    auto sourceFile = createTestFile("large_file.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;

    stub.set_lamda(VADDR(AbstractWorker, doWork), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileOperateBaseWorker::determineCountProcessType,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&FileOperateBaseWorker::checkTotalDiskSpaceAvailable,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(VADDR(DoCopyFilesWorker, endWork),
                   [](DoCopyFilesWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

TEST_F(TestDoCopyFilesWorker, DoWork_CopyFilesFails)
{
    auto sourceFile = createTestFile("fail.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;

    stub.set_lamda(VADDR(AbstractWorker, doWork), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileOperateBaseWorker::determineCountProcessType,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&FileOperateBaseWorker::checkTotalDiskSpaceAvailable,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&FileOperateBaseWorker::initCopyWay,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&DoCopyFilesWorker::copyFiles,
                   [](DoCopyFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(VADDR(DoCopyFilesWorker, endWork),
                   [](DoCopyFilesWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

// ========== stop Tests ==========

TEST_F(TestDoCopyFilesWorker, Stop_ResetsThreadCopyFileCount)
{
    worker->threadCopyFileCount = 10;

    worker->stop();
    EXPECT_EQ(worker->threadCopyFileCount, 0);
}

// ========== endWork Tests ==========

TEST_F(TestDoCopyFilesWorker, EndWork_ProcessesCompleteTargetFiles)
{
    worker->targetUrl = targetDirUrl;
    auto testFile = createTestFile("complete.txt");

    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    fileInfo->initQuerier();
    worker->precompleteTargetFileInfo.append(fileInfo);

    stub.set_lamda(&FileOperateBaseWorker::waitThreadPoolOver,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&FileOperateBaseWorker::setAllDirPermisson,
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(VADDR(AbstractWorker, endWork),
                   []() {
                       __DBG_STUB_INVOKE__
                   });

    worker->endWork();
    EXPECT_FALSE(worker->completeTargetFiles.isEmpty());
    EXPECT_TRUE(worker->precompleteTargetFileInfo.isEmpty());
}

// ========== setStat Tests ==========

TEST_F(TestDoCopyFilesWorker, SetStat_CallsBaseClass)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractWorker, setStat),
                   [&baseCalled](AbstractWorker *, const AbstractJobHandler::JobState &) {
                       __DBG_STUB_INVOKE__
                       baseCalled = true;
                   });

    worker->setStat(AbstractJobHandler::JobState::kRunningState);
    EXPECT_TRUE(baseCalled);
}

// ========== onUpdateProgress Tests ==========

TEST_F(TestDoCopyFilesWorker, OnUpdateProgress_EmitsSignals)
{
    worker->workData->currentWriteSize = 1000;
    worker->sourceFilesTotalSize = 2000;

    bool progressEmitted = false;
    bool speedEmitted = false;

    QObject::connect(worker, &DoCopyFilesWorker::progressChangedNotify,
                     [&progressEmitted](const JobInfoPointer &) {
                         progressEmitted = true;
                     });

    QObject::connect(worker, &DoCopyFilesWorker::speedUpdatedNotify,
                     [&speedEmitted](const JobInfoPointer &) {
                         speedEmitted = true;
                     });

    stub.set_lamda(&FileOperateBaseWorker::getWriteDataSize,
                   [](FileOperateBaseWorker *) -> qint64 {
                       __DBG_STUB_INVOKE__
                       return 1000;
                   });

    worker->onUpdateProgress();
    EXPECT_TRUE(progressEmitted);
    EXPECT_TRUE(speedEmitted);
}
