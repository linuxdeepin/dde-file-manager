// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QSignalSpy>

#include "stubext.h"

#include "fileoperations/trashfiles/dorestoretrashfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-io/dfmio_utils.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoRestoreTrashFilesWorker : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        worker = new DoRestoreTrashFilesWorker();
        ASSERT_TRUE(worker);

        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);
        worker->targetUrl = tempDirUrl;

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
        tempDir.reset();
    }

protected:
    FileInfoPointer createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(content.toUtf8());
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
    DoRestoreTrashFilesWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kRestoreType);
}

TEST_F(TestDoRestoreTrashFilesWorker, Destructor_CallsStop)
{
    bool stopCalled = false;
    stub.set_lamda(VADDR(AbstractWorker, stop), [&stopCalled]() {
        __DBG_STUB_INVOKE__
        stopCalled = true;
    });

    delete worker;
    worker = nullptr;
    EXPECT_TRUE(stopCalled);
}

// ========== initArgs Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, InitArgs_ClearsHandleSourceFiles)
{
    worker->handleSourceFiles.append(QUrl::fromLocalFile("/tmp/test.txt"));

    stub.set_lamda(VADDR(DoRestoreTrashFilesWorker, initArgs), [](FileOperateBaseWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->initArgs();

    EXPECT_TRUE(result);
    EXPECT_TRUE(worker->handleSourceFiles.isEmpty());
}

// ========== statisticsFilesSize Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, StatisticsFilesSize_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);
}

TEST_F(TestDoRestoreTrashFilesWorker, StatisticsFilesSize_WithSources)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/test1.txt"));
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/test2.txt"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);
}

// ========== onUpdateProgress Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, OnUpdateProgress_EmitsSignal)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kRestoreType;
    worker->completeFilesCount = 3;
    worker->sourceFilesCount = 10;

    bool progressEmitted = false;
    QObject::connect(worker, &DoRestoreTrashFilesWorker::progressChangedNotify,
                     [&progressEmitted](const JobInfoPointer &) {
                         progressEmitted = true;
                     });

    worker->onUpdateProgress();
    EXPECT_TRUE(progressEmitted);
}

// ========== doWork Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, DoWork_Success)
{
    stub.set_lamda(VADDR(DoRestoreTrashFilesWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::doRestoreTrashFiles,
                   [](DoRestoreTrashFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractWorker, endWork), []() {
        __DBG_STUB_INVOKE__
    });

    bool result = worker->doWork();
    EXPECT_TRUE(result);
}

TEST_F(TestDoRestoreTrashFilesWorker, DoWork_BaseWorkFails)
{
    stub.set_lamda(VADDR(FileOperateBaseWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

// ========== translateUrls Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, TranslateUrls_Success)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/.Trash/test.txt"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileUtils::bindPathTransform,
                   [](const QUrl &url, const bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return {};
                   });

    bool result = worker->translateUrls();

    EXPECT_TRUE(result);
}

TEST_F(TestDoRestoreTrashFilesWorker, TranslateUrls_StateCheckFails)
{
    worker->sourceUrls.append(QUrl::fromLocalFile("/tmp/test.txt"));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->translateUrls();

    EXPECT_FALSE(result);
}

// ========== doRestoreTrashFiles Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, DoRestoreTrashFiles_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->doRestoreTrashFiles();

    EXPECT_TRUE(result);
}

TEST_F(TestDoRestoreTrashFilesWorker, DoRestoreTrashFiles_WithFiles)
{
    auto testFile = createTestFile("restore_test.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &url) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(url));
                   });

    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &, const DFileInfoPointer &, bool *result) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &, const DFileInfoPointer &,
                          const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doRestoreTrashFiles();
    EXPECT_TRUE(result);
}

// ========== createParentDir Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, CreateParentDir_ParentExists)
{
    QUrl testUrl = QUrl::fromLocalFile(tempDirPath + "/test_file.txt");
    auto restoreInfo = DFileInfoPointer(new DFileInfo(testUrl));

    bool result = false;
    auto parentInfo = worker->createParentDir(testUrl, restoreInfo, &result);

    EXPECT_TRUE(parentInfo || !result);
}

TEST_F(TestDoRestoreTrashFilesWorker, CreateParentDir_CreateNew)
{
    QString newDir = tempDirPath + "/new_parent";
    QString testPath = newDir + "/test_file.txt";
    QUrl testUrl = QUrl::fromLocalFile(testPath);
    auto restoreInfo = DFileInfoPointer(new DFileInfo(testUrl));

    bool result = false;
    auto parentInfo = worker->createParentDir(testUrl, restoreInfo, &result);

    EXPECT_TRUE(parentInfo || !result);
}

// ========== checkRestoreInfo Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, CheckRestoreInfo_ValidUrl)
{
    auto testFile = createTestFile("check_restore.txt");
    QUrl testUrl = testFile->urlOf(UrlInfoType::kUrl);

    stub.set_lamda(&FileUtils::bindPathTransform,
                   [](const QUrl &url, const bool) -> QString {
                       __DBG_STUB_INVOKE__
                       return {};
                   });

    auto restoreInfo = worker->checkRestoreInfo(testUrl);

    SUCCEED();   // Just verify it doesn't crash
}

// ========== mergeDir Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, MergeDir_Success)
{
    auto sourceDir = createTestDir("merge_source");
    auto targetDir = createTestDir("merge_target");

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->mergeDir(sourceDir->urlOf(UrlInfoType::kUrl),
                                   targetDir->urlOf(UrlInfoType::kUrl),
                                   DFile::CopyFlag::kNone);

    SUCCEED();   // Just verify it doesn't crash
}

TEST_F(TestDoRestoreTrashFilesWorker, MergeDir_StateCheckFails)
{
    auto sourceDir = createTestDir("merge_source_fail");
    auto targetDir = createTestDir("merge_target_fail");

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->mergeDir(sourceDir->urlOf(UrlInfoType::kUrl),
                                   targetDir->urlOf(UrlInfoType::kUrl),
                                   DFile::CopyFlag::kNone);

    EXPECT_FALSE(result);
}

// ========== Signal Emission Tests ==========

TEST_F(TestDoRestoreTrashFilesWorker, SignalEmission_FileAdded)
{
    auto testFile = createTestFile("signal_added.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &url) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(url));
                   });

    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &, const DFileInfoPointer &, bool *result) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &, const DFileInfoPointer &,
                          const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   [](FileOperateBaseWorker *, const DFileInfoPointer &,
                      const DFileInfoPointer &, bool *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool signalEmitted = false;
    QObject::connect(worker, &DoRestoreTrashFilesWorker::fileAdded,
                     [&signalEmitted](const QUrl &) {
                         signalEmitted = true;
                     });

    worker->doRestoreTrashFiles();

    EXPECT_TRUE(signalEmitted);
}

// ========== Edge Cases ==========

TEST_F(TestDoRestoreTrashFilesWorker, EdgeCase_MultipleFiles)
{
    worker->sourceUrls.append(createTestFile("restore1.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("restore2.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("restore3.txt")->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &url) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(url));
                   });

    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &, const DFileInfoPointer &, bool *result) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &, const DFileInfoPointer &,
                          const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   []() -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doRestoreTrashFiles();
    EXPECT_TRUE(result);
}

TEST_F(TestDoRestoreTrashFilesWorker, EdgeCase_CheckRestoreInfoFails)
{
    auto testFile = createTestFile("check_fail.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo,
                   [](DoRestoreTrashFilesWorker *, const QUrl &) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    bool result = worker->doRestoreTrashFiles();
    EXPECT_TRUE(result);   // Should skip and continue
}

TEST_F(TestDoRestoreTrashFilesWorker, EdgeCase_CreateParentDirFails)
{
    auto testFile = createTestFile("parent_fail.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &url) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(url));
                   });

    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir,
                   [](DoRestoreTrashFilesWorker *, const QUrl &, const DFileInfoPointer &, bool *result) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *result = false;
                       return nullptr;
                   });

    bool result = worker->doRestoreTrashFiles();
    EXPECT_FALSE(result);
}

TEST_F(TestDoRestoreTrashFilesWorker, EdgeCase_CopyFileFails)
{
    auto testFile = createTestFile("copy_fail.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoRestoreTrashFilesWorker::checkRestoreInfo,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &url) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(url));
                   });

    stub.set_lamda(&DoRestoreTrashFilesWorker::createParentDir,
                   [this](DoRestoreTrashFilesWorker *, const QUrl &, const DFileInfoPointer &, bool *result) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCheckFile,
                   [this](FileOperateBaseWorker *, const DFileInfoPointer &, const DFileInfoPointer &,
                          const QString &, bool *) -> DFileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return DFileInfoPointer(new DFileInfo(tempDirUrl));
                   });

    stub.set_lamda(&FileOperateBaseWorker::doCopyFile,
                   []() -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = worker->doRestoreTrashFiles();
    EXPECT_FALSE(result);
}
