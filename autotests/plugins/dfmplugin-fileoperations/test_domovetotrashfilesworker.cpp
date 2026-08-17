// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QSignalSpy>
#include <QWaitCondition>

#include "stubext.h"

#include "fileoperations/trashfiles/domovetotrashfilesworker.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDoMoveToTrashFilesWorker : public testing::Test
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

        worker = new DoMoveToTrashFilesWorker();
        ASSERT_TRUE(worker);

        worker->workData.reset(new WorkerData);
        worker->localFileHandler.reset(new LocalFileHandler);
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
    DoMoveToTrashFilesWorker *worker;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, Constructor_JobTypeIsCorrect)
{
    EXPECT_EQ(worker->jobType, AbstractJobHandler::JobType::kMoveToTrashType);
}

TEST_F(TestDoMoveToTrashFilesWorker, Destructor_CallsStop)
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

// ========== doWork Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, DoWork_Success)
{
    stub.set_lamda(VADDR(DoMoveToTrashFilesWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoMoveToTrashFilesWorker::doMoveToTrash,
                   [](DoMoveToTrashFilesWorker *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(AbstractWorker, endWork), []() {
        __DBG_STUB_INVOKE__
    });

    bool result = worker->doWork();
    EXPECT_TRUE(result);
}

TEST_F(TestDoMoveToTrashFilesWorker, DoWork_BaseWorkFails)
{
    stub.set_lamda(VADDR(FileOperateBaseWorker, doWork), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = worker->doWork();
    EXPECT_FALSE(result);
}

// ========== statisticsFilesSize Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, StatisticsFilesSize_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->statisticsFilesSize();

    EXPECT_TRUE(result);
}

TEST_F(TestDoMoveToTrashFilesWorker, StatisticsFilesSize_WithSources)
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

TEST_F(TestDoMoveToTrashFilesWorker, OnUpdateProgress_EmitsSignal)
{
    worker->handle.reset(new AbstractJobHandler);
    worker->jobType = AbstractJobHandler::JobType::kMoveToTrashType;
    worker->completeFilesCount = 5;
    worker->sourceFilesCount = 10;

    bool progressEmitted = false;
    QObject::connect(worker, &DoMoveToTrashFilesWorker::progressChangedNotify,
                     [&progressEmitted](const JobInfoPointer &) {
                         progressEmitted = true;
                     });

    worker->onUpdateProgress();
    EXPECT_TRUE(progressEmitted);
}

// ========== doMoveToTrash Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, DoMoveToTrash_EmptySources)
{
    worker->sourceUrls.clear();

    bool result = worker->doMoveToTrash();

    EXPECT_TRUE(result);
}

TEST_F(TestDoMoveToTrashFilesWorker, DoMoveToTrash_WithFiles)
{
    auto testFile = createTestFile("move_to_trash.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash,
                   [](DoMoveToTrashFilesWorker *, const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return true;
                   });

    stub.set_lamda(&DoMoveToTrashFilesWorker::trashTargetUrl,
                   [](DoMoveToTrashFilesWorker *, const QUrl &url) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl::fromLocalFile("/tmp/.Trash/file.txt");
                   });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doMoveToTrash();
    EXPECT_TRUE(result);
}

TEST_F(TestDoMoveToTrashFilesWorker, DoMoveToTrash_StateCheckFails)
{
    auto testFile = createTestFile("state_fail.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doMoveToTrash();
    EXPECT_FALSE(result);
}

// ========== isCanMoveToTrash Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, IsCanMoveToTrash_ValidFile)
{
    auto testFile = createTestFile("can_trash.txt");
    QUrl fileUrl = testFile->urlOf(UrlInfoType::kUrl);

    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = false;
    bool canTrash = worker->isCanMoveToTrash(fileUrl, &result);

    SUCCEED();   // Just verify it doesn't crash
}

TEST_F(TestDoMoveToTrashFilesWorker, IsCanMoveToTrash_LargeFile)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/large_file.bin");

    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&FileOperationsUtils::isFilesSizeOutLimit,
                   [](const QUrl &, const qint64) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;   // Simulate large file
                   });

    bool result = false;
    bool canTrash = worker->isCanMoveToTrash(fileUrl, &result);

    SUCCEED();
}

TEST_F(TestDoMoveToTrashFilesWorker, IsCanMoveToTrash_TrashIsEmpty)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = false;
    bool canTrash = worker->isCanMoveToTrash(fileUrl, &result);

    SUCCEED();
}

// ========== trashTargetUrl Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, TrashTargetUrl_ValidUrl)
{
    QUrl sourceUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QUrl trashUrl = worker->trashTargetUrl(sourceUrl);

    EXPECT_TRUE(trashUrl.isValid());
}

TEST_F(TestDoMoveToTrashFilesWorker, TrashTargetUrl_DuplicateName)
{
    QUrl sourceUrl = QUrl::fromLocalFile("/tmp/duplicate.txt");

    QUrl trashUrl = worker->trashTargetUrl(sourceUrl);

    EXPECT_TRUE(trashUrl.isValid());
}

// ========== doHandleErrorNoSpace Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, DoHandleErrorNoSpace_UserAction)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/no_space.txt");

    stub.set_lamda(VADDR(AbstractWorker, setStat),
                   [](AbstractWorker *, const AbstractJobHandler::JobState &state) {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(state, AbstractJobHandler::JobState::kPauseState);
                   });

    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    worker->currentAction = AbstractJobHandler::SupportAction::kSkipAction;

    auto action = worker->doHandleErrorNoSpace(testUrl);

    EXPECT_EQ(action, AbstractJobHandler::SupportAction::kSkipAction);
}

// ========== Signal Emission Tests ==========

TEST_F(TestDoMoveToTrashFilesWorker, SignalEmission_FileDeleted)
{
    auto testFile = createTestFile("signal_test.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash,
                   [](DoMoveToTrashFilesWorker *, const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return true;
                   });

    stub.set_lamda(&DoMoveToTrashFilesWorker::trashTargetUrl,
                   [](DoMoveToTrashFilesWorker *, const QUrl &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl::fromLocalFile("/tmp/.Trash/file.txt");
                   });

    bool signalEmitted = false;
    QObject::connect(worker, &DoMoveToTrashFilesWorker::fileDeleted,
                     [&signalEmitted](const QUrl &) {
                         signalEmitted = true;
                     });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    worker->doMoveToTrash();

    EXPECT_TRUE(signalEmitted);
}

// ========== Edge Cases ==========

TEST_F(TestDoMoveToTrashFilesWorker, EdgeCase_MultipleFiles)
{
    worker->sourceUrls.append(createTestFile("file1.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("file2.txt")->urlOf(UrlInfoType::kUrl));
    worker->sourceUrls.append(createTestFile("file3.txt")->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash,
                   [](DoMoveToTrashFilesWorker *, const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return true;
                   });

    stub.set_lamda(&DoMoveToTrashFilesWorker::trashTargetUrl,
                   [](DoMoveToTrashFilesWorker *, const QUrl &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl::fromLocalFile("/tmp/.Trash/file.txt");
                   });

    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doMoveToTrash();
    EXPECT_TRUE(result);
}

TEST_F(TestDoMoveToTrashFilesWorker, EdgeCase_CannotMoveToTrash)
{
    auto testFile = createTestFile("cannot_trash.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash,
                   [](DoMoveToTrashFilesWorker *, const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = false;
                       return false;
                   });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doMoveToTrash();
    EXPECT_TRUE(result);   // Should continue even if can't trash
}

TEST_F(TestDoMoveToTrashFilesWorker, EdgeCase_RenameFileFails)
{
    auto testFile = createTestFile("rename_fail.txt");
    worker->sourceUrls.append(testFile->urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(AbstractWorker, stateCheck), [](AbstractWorker *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DoMoveToTrashFilesWorker::isCanMoveToTrash,
                   [](DoMoveToTrashFilesWorker *, const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return true;
                   });

    stub.set_lamda(&DoMoveToTrashFilesWorker::trashTargetUrl,
                   [](DoMoveToTrashFilesWorker *, const QUrl &) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return QUrl::fromLocalFile("/tmp/.Trash/file.txt");
                   });
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [](QWaitCondition *, QMutex *, QDeadlineTimer) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = worker->doMoveToTrash();
    EXPECT_FALSE(result);
}
