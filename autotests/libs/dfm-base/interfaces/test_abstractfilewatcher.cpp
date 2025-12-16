// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

#include <QUrl>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QTimer>
#include <QSignalSpy>

#include <memory>

DFMBASE_USE_NAMESPACE

// Simple private class for testing
class SimpleFileWatcherPrivate : public AbstractFileWatcherPrivate
{
    Q_OBJECT
    friend class SimpleFileWatcher;

public:
    explicit SimpleFileWatcherPrivate(const QUrl &fileUrl, AbstractFileWatcher *qq)
        : AbstractFileWatcherPrivate(fileUrl, qq)
    {
    }

    bool start() override {
        started = true;
        return true;
    }

    bool stop() override {
        started = false;
        return true;
    }
};

class SimpleFileWatcher : public AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit SimpleFileWatcher(const QUrl &url, QObject *parent = nullptr)
        : AbstractFileWatcher(new SimpleFileWatcherPrivate(url, this), parent)
    {
    }

    // Implement the virtual methods
    bool startWatcher() override {
        return AbstractFileWatcher::startWatcher();
    }

    bool stopWatcher() override {
        return AbstractFileWatcher::stopWatcher();
    }

    bool restartWatcher() override {
        return AbstractFileWatcher::restartWatcher();
    }

    // url() method is inherited from base class
    using AbstractFileWatcher::url;

    // Helper methods for testing
    void emitFileAttributeChanged(const QUrl &url) {
        emit fileAttributeChanged(url);
    }

    void emitFileDeleted(const QUrl &url) {
        emit fileDeleted(url);
    }

    void emitSubfileCreated(const QUrl &url) {
        emit subfileCreated(url);
    }

    void emitFileRename(const QUrl &oldUrl, const QUrl &newUrl) {
        emit fileRename(oldUrl, newUrl);
    }

};

class AbstractFileWatcherTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QCoreApplication exists
        if (!QCoreApplication::instance()) {
            app.reset(new QCoreApplication(argc, argv));
        }

        // Create a temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        testDirPath = tempDir->path();
        testDirUrl = QUrl::fromLocalFile(testDirPath);

        testFilePath = tempDir->filePath("test_file.txt");
        testFileUrl = QUrl::fromLocalFile(testFilePath);
    }

    void TearDown() override
    {
        app.reset();
        tempDir.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    int argc { 0 };
    char **argv { nullptr };
    std::unique_ptr<QTemporaryDir> tempDir;
    QString testDirPath;
    QString testFilePath;
    QUrl testDirUrl;
    QUrl testFileUrl;
};

TEST_F(AbstractFileWatcherTest, Constructor)
{
    auto watcher = std::make_unique<SimpleFileWatcher>(testFileUrl);
    
    EXPECT_NE(watcher, nullptr);
    EXPECT_EQ(watcher->url(), testFileUrl);
}

TEST_F(AbstractFileWatcherTest, StartStopWatcher)
{
    auto watcher = std::make_unique<SimpleFileWatcher>(testFileUrl);
    
    // Test start watcher
    EXPECT_TRUE(watcher->startWatcher());
    
    // Test stop watcher
    EXPECT_TRUE(watcher->stopWatcher());
    
    // Test restart watcher
    EXPECT_TRUE(watcher->restartWatcher());
}

TEST_F(AbstractFileWatcherTest, SignalEmission)
{
    auto watcher = std::make_unique<SimpleFileWatcher>(testFileUrl);
    
    // Test file attribute changed signal
    QSignalSpy attributeSpy(watcher.get(), &AbstractFileWatcher::fileAttributeChanged);
    watcher->emitFileAttributeChanged(testFileUrl);
    EXPECT_EQ(attributeSpy.count(), 1);
    
    // Test file deleted signal
    QSignalSpy deletedSpy(watcher.get(), &AbstractFileWatcher::fileDeleted);
    watcher->emitFileDeleted(testFileUrl);
    EXPECT_EQ(deletedSpy.count(), 1);
    
    // Test subfile created signal
    QSignalSpy createdSpy(watcher.get(), &AbstractFileWatcher::subfileCreated);
    watcher->emitSubfileCreated(testFileUrl);
    EXPECT_EQ(createdSpy.count(), 1);
    
    // Test file rename signal
    QUrl newUrl = QUrl::fromLocalFile(tempDir->filePath("renamed_file.txt"));
    QSignalSpy renameSpy(watcher.get(), &AbstractFileWatcher::fileRename);
    watcher->emitFileRename(testFileUrl, newUrl);
    EXPECT_EQ(renameSpy.count(), 1);
    
    QList<QVariant> arguments = renameSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).toUrl(), testFileUrl);
    EXPECT_EQ(arguments.at(1).toUrl(), newUrl);
}

TEST_F(AbstractFileWatcherTest, MultipleWatchers)
{
    // Create multiple watchers for different URLs
    auto watcher1 = std::make_unique<SimpleFileWatcher>(testFileUrl);
    auto watcher2 = std::make_unique<SimpleFileWatcher>(testDirUrl);
    
    // Verify they have different URLs
    EXPECT_NE(watcher1->url(), watcher2->url());
    EXPECT_EQ(watcher1->url(), testFileUrl);
    EXPECT_EQ(watcher2->url(), testDirUrl);
    
    // Both should be able to start/stop independently
    EXPECT_TRUE(watcher1->startWatcher());
    EXPECT_TRUE(watcher2->startWatcher());
    
    EXPECT_TRUE(watcher1->stopWatcher());
    EXPECT_TRUE(watcher2->stopWatcher());
}

TEST_F(AbstractFileWatcherTest, WatcherState)
{
    auto watcher = std::make_unique<SimpleFileWatcher>(testFileUrl);
    
    // Test that watcher can be started and stopped multiple times
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(watcher->startWatcher());
        EXPECT_TRUE(watcher->stopWatcher());
    }
    
    // Test restart
    EXPECT_TRUE(watcher->restartWatcher());
}

// Test to verify the fix for null pointer access issue
TEST_F(AbstractFileWatcherTest, NullPointerAccessFix)
{
    // Create a mock file watcher with a proper private implementation to avoid null pointer access
    auto watcher = std::make_unique<SimpleFileWatcher>(testFileUrl);
    
    // Test all public methods that could potentially access null d pointer
    EXPECT_EQ(watcher->url(), testFileUrl);
    EXPECT_TRUE(watcher->startWatcher());
    EXPECT_TRUE(watcher->stopWatcher());
    EXPECT_TRUE(watcher->restartWatcher());
    EXPECT_GE(watcher->getCacheInfoConnectSize(), 0);
    
    // Test cache info connect size methods
    int originalSize = watcher->getCacheInfoConnectSize();
    watcher->addCacheInfoConnectSize();
    EXPECT_EQ(watcher->getCacheInfoConnectSize(), originalSize + 1);
    watcher->reduceCacheInfoConnectSize();
    EXPECT_EQ(watcher->getCacheInfoConnectSize(), originalSize);
}

#include "test_abstractfilewatcher.moc"