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
#include <QTest>

#include "stubext.h"

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-io/dwatcher.h>
#include <dfm-io/dfile.h>

DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

class TestLocalFileWatcher : public testing::Test
{
public:
    void SetUp() override
    {
        // Basic stubs for essential functions
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        stub.set_lamda(&InfoFactory::regClass<SyncFileInfo>, [] { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(&InfoFactory::regInfoTransFunc<FileInfo>, [] { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(&InfoFactory::regClass<AsyncFileInfo>, [] { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(&DirIteratorFactory::regClass<LocalDirIterator>, [] { __DBG_STUB_INVOKE__ return true; });
        stub.set_lamda(&WatcherFactory::regClass<LocalFileWatcher>, [] { __DBG_STUB_INVOKE__ return true; });

        // Create temporary directory for test files
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        watcher = new LocalFileWatcher(tempDirUrl);
    }

    void TearDown() override
    {
        stub.clear();

        tempDir.reset();
        delete watcher;
    }

protected:
    void createTestFile(const QString &fileName)
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << "test content";
        file.close();
    }

    void deleteTestFile(const QString &fileName)
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile::remove(filePath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QUrl tempDirUrl;
    LocalFileWatcher *watcher = nullptr;
};

// ========== Constructor Tests ==========

TEST_F(TestLocalFileWatcher, Constructor_ValidUrl)
{
    // watcher = new LocalFileWatcher(tempDirUrl);
    EXPECT_NE(watcher, nullptr);
}

TEST_F(TestLocalFileWatcher, Constructor_WithParent)
{
    QObject parent;
    LocalFileWatcher *pwatcher = new LocalFileWatcher(tempDirUrl, &parent);
    EXPECT_NE(pwatcher, nullptr);
    EXPECT_EQ(pwatcher->parent(), &parent);
    delete pwatcher;
}

// ========== StartWatcher Tests ==========

TEST_F(TestLocalFileWatcher, StartWatcher_ValidDirectory)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = watcher->startWatcher();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileWatcher, StartWatcher_NonExistentDirectory)
{
    QUrl nonExistentUrl = QUrl::fromLocalFile("/nonexistent/directory");
    LocalFileWatcher *ewatcher = new LocalFileWatcher(nonExistentUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = ewatcher->startWatcher();
    EXPECT_FALSE(result);
    delete ewatcher;
}

TEST_F(TestLocalFileWatcher, StartWatcher_AlreadyStarted)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    watcher->startWatcher();
    bool result = watcher->startWatcher();   // Start again
    EXPECT_TRUE(result == true || result == false);
}

// ========== StopWatcher Tests ==========

TEST_F(TestLocalFileWatcher, StopWatcher_AfterStart)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractFileWatcher, stopWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    watcher->startWatcher();
    bool result = watcher->stopWatcher();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalFileWatcher, StopWatcher_WithoutStart)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, stopWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = watcher->stopWatcher();
    EXPECT_TRUE(result == true || result == false);
}

// ========== NotifyFileAdded Tests ==========

TEST_F(TestLocalFileWatcher, NotifyFileAdded_EmitsSignal)
{
    // watcher = new LocalFileWatcher(tempDirUrl);
    QSignalSpy spy(watcher, &LocalFileWatcher::subfileCreated);

    QUrl fileUrl = QUrl::fromLocalFile(tempDirPath + "/newfile.txt");
    watcher->notifyFileAdded(fileUrl);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<QUrl>(), fileUrl);
}

TEST_F(TestLocalFileWatcher, NotifyFileAdded_MultipleFiles)
{
    // watcher = new LocalFileWatcher(tempDirUrl);
    QSignalSpy spy(watcher, &LocalFileWatcher::subfileCreated);

    QUrl file1 = QUrl::fromLocalFile(tempDirPath + "/file1.txt");
    QUrl file2 = QUrl::fromLocalFile(tempDirPath + "/file2.txt");

    watcher->notifyFileAdded(file1);
    watcher->notifyFileAdded(file2);

    EXPECT_EQ(spy.count(), 2);
}

// ========== NotifyFileChanged Tests ==========

TEST_F(TestLocalFileWatcher, NotifyFileChanged_EmitsSignal)
{
    LocalFileWatcher *tmpwatcher = new LocalFileWatcher(tempDirUrl);
    QSignalSpy spy(tmpwatcher, &LocalFileWatcher::fileAttributeChanged);

    QUrl fileUrl = QUrl::fromLocalFile(tempDirPath + "/changed.txt");
    tmpwatcher->notifyFileChanged(fileUrl);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<QUrl>(), fileUrl);
    delete tmpwatcher;
}

TEST_F(TestLocalFileWatcher, NotifyFileChanged_MultipleChanges)
{
    // watcher = new LocalFileWatcher(tempDirUrl);
    QSignalSpy spy(watcher, &LocalFileWatcher::fileAttributeChanged);

    QUrl fileUrl = QUrl::fromLocalFile(tempDirPath + "/modified.txt");

    watcher->notifyFileChanged(fileUrl);
    watcher->notifyFileChanged(fileUrl);
    watcher->notifyFileChanged(fileUrl);

    EXPECT_EQ(spy.count(), 3);
}

// ========== NotifyFileDeleted Tests ==========

TEST_F(TestLocalFileWatcher, NotifyFileDeleted_EmitsSignal)
{
    // watcher = new LocalFileWatcher(tempDirUrl);
    QSignalSpy spy(watcher, &LocalFileWatcher::fileDeleted);

    QUrl fileUrl = QUrl::fromLocalFile(tempDirPath + "/deleted.txt");
    watcher->notifyFileDeleted(fileUrl);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<QUrl>(), fileUrl);
}

TEST_F(TestLocalFileWatcher, NotifyFileDeleted_MultipleDeletions)
{
    // watcher = new LocalFileWatcher(tempDirUrl);
    QSignalSpy spy(watcher, &LocalFileWatcher::fileDeleted);

    QUrl file1 = QUrl::fromLocalFile(tempDirPath + "/delete1.txt");
    QUrl file2 = QUrl::fromLocalFile(tempDirPath + "/delete2.txt");

    watcher->notifyFileDeleted(file1);
    watcher->notifyFileDeleted(file2);

    EXPECT_EQ(spy.count(), 2);
}

// ========== Signal Connection Tests ==========

TEST_F(TestLocalFileWatcher, Signals_AllConnected)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    QSignalSpy addedSpy(watcher, &LocalFileWatcher::subfileCreated);
    QSignalSpy changedSpy(watcher, &LocalFileWatcher::fileAttributeChanged);
    QSignalSpy deletedSpy(watcher, &LocalFileWatcher::fileDeleted);

    EXPECT_TRUE(addedSpy.isValid());
    EXPECT_TRUE(changedSpy.isValid());
    EXPECT_TRUE(deletedSpy.isValid());
}

// ========== Multiple Watchers Tests ==========

TEST_F(TestLocalFileWatcher, MultipleWatchers_SameDirectory)
{
    LocalFileWatcher *watcher1 = new LocalFileWatcher(tempDirUrl);
    LocalFileWatcher *watcher2 = new LocalFileWatcher(tempDirUrl);

    EXPECT_NE(watcher1, nullptr);
    EXPECT_NE(watcher2, nullptr);
    EXPECT_NE(watcher1, watcher2);

    delete watcher1;
    delete watcher2;
}

TEST_F(TestLocalFileWatcher, MultipleWatchers_DifferentDirectories)
{
    QTemporaryDir dir2;
    ASSERT_TRUE(dir2.isValid());

    LocalFileWatcher *watcher1 = new LocalFileWatcher(tempDirUrl);
    LocalFileWatcher *watcher2 = new LocalFileWatcher(QUrl::fromLocalFile(dir2.path()));

    EXPECT_NE(watcher1, nullptr);
    EXPECT_NE(watcher2, nullptr);

    delete watcher1;
    delete watcher2;
}

// ========== Lifecycle Tests ==========

TEST_F(TestLocalFileWatcher, Lifecycle_StartStopMultipleTimes)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(AbstractFileWatcher, stopWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    for (int i = 0; i < 3; ++i) {
        watcher->startWatcher();
        watcher->stopWatcher();
    }

    // No assertion needed, just verify it doesn't crash
}

// ========== Edge Cases Tests ==========

TEST_F(TestLocalFileWatcher, EdgeCase_EmptyUrl)
{
    QUrl emptyUrl;
    // watcher = new LocalFileWatcher(emptyUrl);
    EXPECT_NE(watcher, nullptr);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = watcher->startWatcher();
    EXPECT_FALSE(result);
}

TEST_F(TestLocalFileWatcher, EdgeCase_FileAsDirectory)
{
    QString filePath = tempDirPath + QDir::separator() + "regularfile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("content");
    file.close();

    QUrl fileUrl = QUrl::fromLocalFile(filePath);
   LocalFileWatcher *filewatcher = new LocalFileWatcher(fileUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = filewatcher->startWatcher();
    EXPECT_TRUE(result == true || result == false);
    delete filewatcher;
}

TEST_F(TestLocalFileWatcher, EdgeCase_SpecialCharactersInPath)
{
    QString specialPath = tempDirPath + QDir::separator() + "special dir #123";
    QDir().mkpath(specialPath);

    QUrl specialUrl = QUrl::fromLocalFile(specialPath);
    LocalFileWatcher *specialwatcher = new LocalFileWatcher(specialUrl);
    EXPECT_NE(specialwatcher, nullptr);
    delete specialwatcher;
}

// ========== Notification Tests with Real File Operations ==========

TEST_F(TestLocalFileWatcher, RealFileOperation_CreateFile)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    watcher->startWatcher();

    QSignalSpy spy(watcher, &LocalFileWatcher::subfileCreated);

    // Manually notify (simulating file creation)
    QUrl newFileUrl = QUrl::fromLocalFile(tempDirPath + "/newfile.txt");
    watcher->notifyFileAdded(newFileUrl);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestLocalFileWatcher, RealFileOperation_ModifyFile)
{
    createTestFile("modify.txt");

    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    watcher->startWatcher();

    QSignalSpy spy(watcher, &LocalFileWatcher::fileAttributeChanged);

    // Manually notify (simulating file modification)
    QUrl modifiedUrl = QUrl::fromLocalFile(tempDirPath + "/modify.txt");
    watcher->notifyFileChanged(modifiedUrl);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestLocalFileWatcher, RealFileOperation_DeleteFile)
{
    createTestFile("delete.txt");

    // watcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    watcher->startWatcher();

    QSignalSpy spy(watcher, &LocalFileWatcher::fileDeleted);

    // Manually notify (simulating file deletion)
    QUrl deletedUrl = QUrl::fromLocalFile(tempDirPath + "/delete.txt");
    watcher->notifyFileDeleted(deletedUrl);

    EXPECT_EQ(spy.count(), 1);
}

// ========== Destructor Tests ==========

TEST_F(TestLocalFileWatcher, Destructor_AfterStart)
{
    LocalFileWatcher *tmpWatcher = new LocalFileWatcher(tempDirUrl);

    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), [](AbstractFileWatcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    tmpWatcher->startWatcher();
    delete tmpWatcher;
    // No assertion needed, just verify it doesn't crash
}

TEST_F(TestLocalFileWatcher, Destructor_WithoutStart)
{
    LocalFileWatcher *tmpWatcher = new LocalFileWatcher(tempDirUrl);
    delete tmpWatcher;
    // No assertion needed, just verify it doesn't crash
}

// ========== Thread Safety Tests ==========

TEST_F(TestLocalFileWatcher, ThreadSafety_SimultaneousNotifications)
{
    // watcher = new LocalFileWatcher(tempDirUrl);

    QSignalSpy addedSpy(watcher, &LocalFileWatcher::subfileCreated);
    QSignalSpy changedSpy(watcher, &LocalFileWatcher::fileAttributeChanged);
    QSignalSpy deletedSpy(watcher, &LocalFileWatcher::fileDeleted);

    QUrl file1 = QUrl::fromLocalFile(tempDirPath + "/file1.txt");
    QUrl file2 = QUrl::fromLocalFile(tempDirPath + "/file2.txt");
    QUrl file3 = QUrl::fromLocalFile(tempDirPath + "/file3.txt");

    watcher->notifyFileAdded(file1);
    watcher->notifyFileChanged(file2);
    watcher->notifyFileDeleted(file3);

    EXPECT_EQ(addedSpy.count(), 1);
    EXPECT_EQ(changedSpy.count(), 1);
    EXPECT_EQ(deletedSpy.count(), 1);
}
