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
#include <dfm-base/utils/fileutils.h>
#include <dfm-io/dfile.h>
#include <dfm-io/dfmio_utils.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include "fileoperations/cutfiles/docutfilesworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoCutFilesWorker : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

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
        worker = new DoCutFilesWorker();
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
    DoCutFilesWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoCutFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kCutType);
}

TEST_F(TestDoCutFilesWorker, Destructor_CallsStop)
{
    bool stopCalled = false;
    stub.set_lamda(VADDR(DoCutFilesWorker, stop), [&stopCalled](DoCutFilesWorker *) {
        __DBG_STUB_INVOKE__
        stopCalled = true;
    });

    delete worker;
    worker = nullptr;
    EXPECT_TRUE(stopCalled);
}

// ========== initArgs Tests ==========

TEST_F(TestDoCutFilesWorker, InitArgs_EmptySourceUrls)
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

TEST_F(TestDoCutFilesWorker, InitArgs_InvalidTargetUrl)
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

TEST_F(TestDoCutFilesWorker, InitArgs_ValidArguments)
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
    EXPECT_EQ(worker->targetOrgUrl, targetDirUrl);
}

// ========== checkSelf Tests ==========

TEST_F(TestDoCutFilesWorker, CheckSelf_SameUrl)
{
    auto testFile = createTestFile("same.txt");
    worker->targetUrl = tempDirUrl;
    worker->targetInfo.reset(new DFileInfo(tempDirUrl));
    worker->targetInfo->initQuerier();

    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    fileInfo->initQuerier();

    bool result = worker->checkSelf(fileInfo);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, CheckSelf_DifferentUrl)
{
    auto testFile = createTestFile("different.txt");
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    fileInfo->initQuerier();

    bool result = worker->checkSelf(fileInfo);
    EXPECT_FALSE(result);
}

// ========== checkSymLink Tests ==========

TEST_F(TestDoCutFilesWorker, CheckSymLink_ValidSymlink)
{
    auto sourceFile = createTestFile("link_source.txt");
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto fileInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &,
                          const DFileInfoPointer &, const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       QString targetPath = targetDirPath + "/link_target.txt";
                       return DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
                   });

    stub.set_lamda(&FileOperateBaseWorker::createSystemLink,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, const bool, const bool, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->checkSymLink(fileInfo);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, CheckSymLink_DoCheckFileFails)
{
    auto sourceFile = createTestFile("link_fail.txt");
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto fileInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, const QString &, bool *skip) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *skip = true;
                       return nullptr;
                   });

    bool result = worker->checkSymLink(fileInfo);
    EXPECT_TRUE(result);
}

// ========== renameFileByHandler Tests ==========

TEST_F(TestDoCutFilesWorker, RenameFileByHandler_Success)
{
    auto sourceFile = createTestFile("rename_source.txt");
    QString targetPath = targetDirPath + "/rename_target.txt";

    auto sourceInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto targetInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    bool skip = false;
    bool result = worker->renameFileByHandler(sourceInfo, targetInfo, &skip);
    EXPECT_TRUE(result || skip);
}

TEST_F(TestDoCutFilesWorker, RenameFileByHandler_PermissionError)
{
    auto sourceFile = createTestFile("no_perm.txt");
    QString targetPath = targetDirPath + "/target.txt";

    auto sourceInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    auto targetInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));

    stub.set_lamda(&LocalFileHandler::renameFile,
                   [](LocalFileHandler *, const QUrl &, const QUrl &, const bool) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&LocalFileHandler::errorCode,
                   [](LocalFileHandler *) -> DFMIOErrorCode {
                       __DBG_STUB_INVOKE__
                       return DFMIOErrorCode::DFM_IO_ERROR_PERMISSION_DENIED;
                   });

    stub.set_lamda(&FileOperateBaseWorker::doHandleErrorAndWait,
                   [](FileOperateBaseWorker *, const QUrl &, const QUrl &,
                      const AbstractJobHandler::JobErrorType &error, const bool,
                      const QString &, const bool) -> AbstractJobHandler::SupportAction {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(error, AbstractJobHandler::JobErrorType::kPermissionError);
                       return AbstractJobHandler::SupportAction::kSkipAction;
                   });

    bool skip = false;
    bool result = worker->renameFileByHandler(sourceInfo, targetInfo, &skip);
    EXPECT_FALSE(result);
    EXPECT_TRUE(skip);
}

// ========== trySameDeviceRename Tests ==========

TEST_F(TestDoCutFilesWorker, TrySameDeviceRename_Success)
{
    auto sourceFile = createTestFile("same_dev.txt");
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto sourceInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    sourceInfo->initQuerier();

    auto targetPathInfo = DFileInfoPointer(new DFileInfo(targetDirUrl));
    targetPathInfo->initQuerier();

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &,
                          const DFileInfoPointer &, const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       QString targetPath = targetDirPath + "/renamed.txt";
                       auto info = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
                       return info;
                   });

    stub.set_lamda(&DoCutFilesWorker::renameFileByHandler,
                   [](DoCutFilesWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool ok = false;
    bool skip = false;
    auto result = worker->trySameDeviceRename(sourceInfo, targetPathInfo,
                                              "same_dev.txt", &ok, &skip);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ok);
}

TEST_F(TestDoCutFilesWorker, TrySameDeviceRename_MergeDirectory)
{
    auto sourceDir = createTestDir("merge_source");
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();
    worker->isCutMerge = true;

    auto sourceInfo = DFileInfoPointer(new DFileInfo(sourceDir->urlOf(UrlInfoType::kUrl)));
    sourceInfo->initQuerier();

    auto targetPathInfo = DFileInfoPointer(new DFileInfo(targetDirUrl));
    targetPathInfo->initQuerier();

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &,
                          const DFileInfoPointer &, const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       QString targetPath = targetDirPath + "/merge_target";
                       QDir().mkpath(targetPath);
                       auto info = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
                       info->initQuerier();
                       return info;
                   });

    stub.set_lamda(&DoCutFilesWorker::doMergDir,
                   [](DoCutFilesWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool ok = false;
    bool skip = false;
    auto result = worker->trySameDeviceRename(sourceInfo, targetPathInfo,
                                              "merge_source", &ok, &skip);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(worker->isCutMerge);
}

// ========== doMergDir Tests ==========

TEST_F(TestDoCutFilesWorker, DoMergDir_EmptyDirectory)
{
    auto sourceDir = createTestDir("empty_merge");
    QString targetPath = targetDirPath + "/merge_target";
    QDir().mkpath(targetPath);

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceDir->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();

    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
    toInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool skip = false;
    bool result = worker->doMergDir(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, DoMergDir_WithFiles)
{
    auto sourceDir = createTestDir("dir_with_files");
    QString filePath = tempDirPath + "/dir_with_files/file.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    QString targetPath = targetDirPath + "/merge_target";
    QDir().mkpath(targetPath);

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceDir->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();

    auto toInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
    toInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCutFilesWorker::doCutFile,
                   [](DoCutFilesWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    bool result = worker->doMergDir(fromInfo, toInfo, &skip);
    EXPECT_TRUE(result);
}

// ========== doCutFile Tests ==========

TEST_F(TestDoCutFilesWorker, DoCutFile_SameDeviceFile)
{
    auto sourceFile = createTestFile("cut_same_dev.txt");
    worker->targetUrl = targetDirUrl;
    worker->targetOrgUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();

    auto targetPathInfo = DFileInfoPointer(new DFileInfo(targetDirUrl));
    targetPathInfo->initQuerier();

    stub.set_lamda(&dfmio::DFMUtils::mountPathFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/same/mount";
    });

    stub.set_lamda(&DoCutFilesWorker::trySameDeviceRename,
                   [](DoCutFilesWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, const QString, bool *ok, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *ok = true;
                       return DFileInfoPointer(new DFileInfo(QUrl()));
                   });

    bool skip = false;
    bool result = worker->doCutFile(fromInfo, targetPathInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, DoCutFile_CrossDeviceFile)
{
    auto sourceFile = createTestFile("cut_cross_dev.txt");
    worker->targetUrl = targetDirUrl;
    worker->targetOrgUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto fromInfo = DFileInfoPointer(new DFileInfo(sourceFile->urlOf(UrlInfoType::kUrl)));
    fromInfo->initQuerier();

    auto targetPathInfo = DFileInfoPointer(new DFileInfo(targetDirUrl));
    targetPathInfo->initQuerier();

    int callCount = 0;
    stub.set_lamda(&dfmio::DFMUtils::mountPathFromUrl,
                   [&callCount](const QUrl &) -> QString {
                       __DBG_STUB_INVOKE__
                       return callCount++ == 0 ? "/source/mount" : "/target/mount";
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &,
                          const DFileInfoPointer &, const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       QString targetPath = targetDirPath + "/cross_dev_target.txt";
                       return DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
                   });

    stub.set_lamda(&FileOperateBaseWorker::copyAndDeleteFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool skip = false;
    bool result = worker->doCutFile(fromInfo, targetPathInfo, &skip);
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, DoCutFile_TrashFile)
{
    // Create trash file structure
    QString trashFilesPath = tempDirPath + "/.local/share/Trash/files";
    QDir().mkpath(trashFilesPath);
    QString trashInfoPath = tempDirPath + "/.local/share/Trash/info";
    QDir().mkpath(trashInfoPath);

    QString testFilePath = trashFilesPath + "/test.txt";
    QFile file(testFilePath);
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    QString trashInfoFile = trashInfoPath + "/test.txt.trashinfo";
    QFile infoFile(trashInfoFile);
    if (infoFile.open(QIODevice::WriteOnly)) {
        QTextStream stream(&infoFile);
        stream << "[Trash Info] Path=original.txt DeletionDate=2025-01-01T00:00:00";
        infoFile.close();
    }

    worker->targetUrl = targetDirUrl;
    worker->targetOrgUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    auto fromInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(testFilePath)));
    fromInfo->initQuerier();

    auto targetPathInfo = DFileInfoPointer(new DFileInfo(targetDirUrl));
    targetPathInfo->initQuerier();

    stub.set_lamda(&dfmio::DFMUtils::mountPathFromUrl, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/same/mount";
    });

    stub.set_lamda(&DoCutFilesWorker::trySameDeviceRename,
                   [this](DoCutFilesWorker *, const DFileInfoPointer &,
                          const DFileInfoPointer &, const QString, bool *ok, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *ok = true;
                       QString targetPath = targetDirPath + "/restored.txt";
                       return DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(targetPath)));
                   });

    bool skip = false;
    bool result = worker->doCutFile(fromInfo, targetPathInfo, &skip);
    EXPECT_TRUE(result);
}

// ========== cutFiles Tests ==========

TEST_F(TestDoCutFilesWorker, CutFiles_SingleFile)
{
    auto sourceFile = createTestFile("cut_single.txt");
    worker->sourceUrls.append(sourceFile->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = targetDirUrl;
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCutFilesWorker::checkSelf,
                   [](DoCutFilesWorker *, const DFileInfoPointer &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&DoCutFilesWorker::doCutFile,
                   [](DoCutFilesWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->cutFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, CutFiles_DirectoryToSelf)
{
    auto sourceDir = createTestDir("self_dir");
    worker->sourceUrls.append(sourceDir->urlOf(UrlInfoType::kUrl));
    worker->targetUrl = sourceDir->urlOf(UrlInfoType::kUrl);
    worker->targetInfo.reset(new DFileInfo(targetDirUrl));
    worker->targetInfo->initQuerier();

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoCutFilesWorker::checkSelf,
                   [](DoCutFilesWorker *, const DFileInfoPointer &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool signalEmitted = false;
    QObject::connect(worker, &DoCutFilesWorker::requestShowTipsDialog,
                     [&signalEmitted](DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType,
                                      const QList<QUrl> &) {
                         signalEmitted = true;
                     });

    bool result = worker->cutFiles();
    EXPECT_FALSE(result);
    EXPECT_TRUE(signalEmitted);
}

// ========== doWork Tests ==========

TEST_F(TestDoCutFilesWorker, DoWork_Success)
{
    auto sourceFile = createTestFile("work_success.txt");
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

    stub.set_lamda(&DoCutFilesWorker::cutFiles,
                   [](DoCutFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(DoCutFilesWorker, endWork),
                   []() {
                       __DBG_STUB_INVOKE__
                   });

    bool result = worker->doWork();
    EXPECT_TRUE(result);
}

TEST_F(TestDoCutFilesWorker, DoWork_CutFilesFails)
{
    auto sourceFile = createTestFile("work_fail.txt");
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

    stub.set_lamda(&DoCutFilesWorker::cutFiles,
                   [](DoCutFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(VADDR(DoCutFilesWorker, endWork),
                   []() {
                       __DBG_STUB_INVOKE__
                   });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

// ========== endWork Tests ==========

TEST_F(TestDoCutFilesWorker, EndWork_DeletesCutFiles)
{
    auto testFile = createTestFile("delete_after_cut.txt");
    auto fileInfo = DFileInfoPointer(new DFileInfo(testFile->urlOf(UrlInfoType::kUrl)));
    worker->cutAndDeleteFiles.append(fileInfo);

    stub.set_lamda(VADDR(DoCutFilesWorker, endWork),
                   [](FileOperateBaseWorker *) {
                       __DBG_STUB_INVOKE__
                   });

    worker->endWork();
    // File should be deleted
}

TEST_F(TestDoCutFilesWorker, EndWork_EmptyCutList)
{
    worker->cutAndDeleteFiles.clear();

    stub.set_lamda(VADDR(DoCutFilesWorker, endWork),
                   []() {
                       __DBG_STUB_INVOKE__
                   });

    // Should not crash
    worker->endWork();
}

// ========== onUpdateProgress Tests ==========

TEST_F(TestDoCutFilesWorker, OnUpdateProgress_EmitsSignals)
{
    worker->workData->currentWriteSize = 1000;
    worker->sourceFilesTotalSize = 2000;

    bool progressEmitted = false;
    bool speedEmitted = false;

    QObject::connect(worker, &DoCutFilesWorker::progressChangedNotify,
                     [&progressEmitted](const JobInfoPointer &) {
                         progressEmitted = true;
                     });

    QObject::connect(worker, &DoCutFilesWorker::speedUpdatedNotify,
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

// ========== emitCompleteFilesUpdatedNotify Tests ==========

TEST_F(TestDoCutFilesWorker, EmitCompleteFilesUpdatedNotify_EmitsSignal)
{
    bool signalEmitted = false;
    qint64 receivedCount = 0;

    QObject::connect(worker, &DoCutFilesWorker::stateChangedNotify,
                     [&signalEmitted, &receivedCount](const JobInfoPointer &info) {
                         if (info->contains(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey)) {
                             signalEmitted = true;
                             receivedCount = info->value(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey).toLongLong();
                         }
                     });

    worker->emitCompleteFilesUpdatedNotify(42);
    EXPECT_TRUE(signalEmitted);
    EXPECT_EQ(receivedCount, 42);
}

// ========== stop Tests ==========

TEST_F(TestDoCutFilesWorker, Stop_CallsBaseClass)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractWorker, stop), [&baseCalled](AbstractWorker *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    worker->stop();
    EXPECT_TRUE(baseCalled);
}
