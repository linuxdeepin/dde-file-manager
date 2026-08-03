// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSignalSpy>
#include <QThread>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/inotifyfilesystemwatcher.h"
#include "services/textindex/fsmonitor/inotifyfilesystemwatcher_p.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// InotifyFileSystemWatcher 使用 inotify 监控文件系统变化
// 通过 QSocketNotifier 异步投递事件，需要用 QSignalSpy::wait() 等待信号
class TestInotifyFileSystemWatcher : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 在 ~/Documents/ 下创建隐藏临时目录
        QString templatePath = QDir::homePath() + "/Documents/.test_inotify_XXXXXX";
        testDir = std::make_unique<QTemporaryDir>(templatePath);
        ASSERT_TRUE(testDir->isValid());
        // 确保目录存在
        QDir().mkpath(testDir->path());
    }

    void TearDown() override
    {
        testDir.reset();
    }

    // 辅助：等待信号并返回信号计数，超时返回 0
    int waitForSignal(QSignalSpy &spy, int timeoutMs = 3000)
    {
        spy.wait(timeoutMs);
        return spy.count();
    }

    std::unique_ptr<QTemporaryDir> testDir;
};

// ========== 构造与基础功能 ==========

TEST_F(TestInotifyFileSystemWatcher, Construction)
{
    InotifyFileSystemWatcher watcher;
    EXPECT_EQ(watcher.watchFlags(), InotifyFileSystemWatcher::AllEvents);
    EXPECT_TRUE(watcher.files().isEmpty());
    EXPECT_TRUE(watcher.directories().isEmpty());
}

TEST_F(TestInotifyFileSystemWatcher, AddDirectoryPath)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();

    bool result = watcher.addPath(dirPath);
    EXPECT_TRUE(result);
    EXPECT_TRUE(watcher.directories().contains(dirPath));
    EXPECT_TRUE(watcher.files().isEmpty());
}

TEST_F(TestInotifyFileSystemWatcher, AddFilePath)
{
    InotifyFileSystemWatcher watcher;
    QString filePath = testDir->path() + "/testfile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("hello");
    file.close();

    bool result = watcher.addPath(filePath);
    EXPECT_TRUE(result);
    EXPECT_TRUE(watcher.files().contains(filePath));
    EXPECT_TRUE(watcher.directories().isEmpty());
}

TEST_F(TestInotifyFileSystemWatcher, RemovePath)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();

    watcher.addPath(dirPath);
    EXPECT_TRUE(watcher.directories().contains(dirPath));

    bool result = watcher.removePath(dirPath);
    EXPECT_TRUE(result);
    EXPECT_FALSE(watcher.directories().contains(dirPath));
}

TEST_F(TestInotifyFileSystemWatcher, DuplicatePathIsIdempotent)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();

    bool r1 = watcher.addPath(dirPath);
    bool r2 = watcher.addPath(dirPath);
    // 第一次添加成功返回 true
    EXPECT_TRUE(r1);
    // 第二次重复添加返回 false（私有 addPaths 将重复路径视为"未添加"保留在返回列表中）
    EXPECT_FALSE(r2);
    // 目录列表中只有一个条目，不会重复
    EXPECT_EQ(watcher.directories().size(), 1);
}

TEST_F(TestInotifyFileSystemWatcher, AddPathsBatch)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    QString filePath = dirPath + "/batchfile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.close();

    QStringList paths = { dirPath, filePath };
    QStringList failed = watcher.addPaths(paths);
    EXPECT_TRUE(failed.isEmpty());
    EXPECT_TRUE(watcher.directories().contains(dirPath));
    EXPECT_TRUE(watcher.files().contains(filePath));
}

TEST_F(TestInotifyFileSystemWatcher, RemovePathsBatch)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QStringList removed = watcher.removePaths({ dirPath });
    EXPECT_TRUE(removed.isEmpty());
    EXPECT_FALSE(watcher.directories().contains(dirPath));
}

TEST_F(TestInotifyFileSystemWatcher, AddEmptyPathReturnsFalse)
{
    InotifyFileSystemWatcher watcher;
    bool result = watcher.addPath("");
    // 空路径被过滤，addPath 返回 false（列表不为空说明路径未被添加）
    EXPECT_FALSE(result);
}

TEST_F(TestInotifyFileSystemWatcher, SetWatchFlags)
{
    InotifyFileSystemWatcher watcher;
    watcher.setWatchFlags(InotifyFileSystemWatcher::FileClose);
    EXPECT_EQ(watcher.watchFlags(), InotifyFileSystemWatcher::FileClose);
}

// ========== 文件创建信号 ==========

TEST_F(TestInotifyFileSystemWatcher, FileCreatedInWatchedDirectory)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy spy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    ASSERT_TRUE(spy.isValid());

    // 创建文件
    QString filePath = dirPath + "/newfile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("test content");
    file.close();

    int count = waitForSignal(spy);
    EXPECT_EQ(count, 1);

    // 验证信号参数
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);
    EXPECT_EQ(args.at(1).toString(), "newfile.txt");
}

TEST_F(TestInotifyFileSystemWatcher, SubdirectoryCreatedInWatchedDirectory)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy spy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    ASSERT_TRUE(spy.isValid());

    // 创建子目录
    QString subDirPath = dirPath + "/subdir";
    QDir().mkpath(subDirPath);

    int count = waitForSignal(spy);
    EXPECT_EQ(count, 1);

    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);
    EXPECT_EQ(args.at(1).toString(), "subdir");
}

// ========== 文件删除信号 ==========

TEST_F(TestInotifyFileSystemWatcher, FileDeletedInWatchedDirectory)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    // 先创建文件
    QString filePath = dirPath + "/todelete.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("delete me");
    file.close();

    // 清除创建阶段的信号
    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    QSignalSpy deleteSpy(&watcher, &InotifyFileSystemWatcher::fileDeleted);
    ASSERT_TRUE(deleteSpy.isValid());

    // 删除文件
    QFile::remove(filePath);

    int count = waitForSignal(deleteSpy);
    EXPECT_EQ(count, 1);

    QList<QVariant> args = deleteSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);
    EXPECT_EQ(args.at(1).toString(), "todelete.txt");
}

TEST_F(TestInotifyFileSystemWatcher, DirectoryDeletedInWatchedDirectory)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    // 先创建子目录
    QString subDirPath = dirPath + "/todeletedir";
    QDir().mkpath(subDirPath);

    // 等待创建信号
    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    QSignalSpy deleteSpy(&watcher, &InotifyFileSystemWatcher::fileDeleted);
    ASSERT_TRUE(deleteSpy.isValid());

    // 删除子目录
    QDir().rmdir(subDirPath);

    int count = waitForSignal(deleteSpy);
    EXPECT_EQ(count, 1);

    QList<QVariant> args = deleteSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);
    EXPECT_EQ(args.at(1).toString(), "todeletedir");
}

// ========== 文件修改信号 ==========

TEST_F(TestInotifyFileSystemWatcher, FileModifiedInWatchedDirectory)
{
    InotifyFileSystemWatcher watcher;
    watcher.setWatchFlags(InotifyFileSystemWatcher::FileModify);
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy spy(&watcher, &InotifyFileSystemWatcher::fileModified);
    ASSERT_TRUE(spy.isValid());

    // 创建并写入文件
    QString filePath = dirPath + "/modify_test.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("modify me");
    file.flush();

    int count = waitForSignal(spy);
    EXPECT_GE(count, 1);
}

// ========== 文件关闭信号 ==========

TEST_F(TestInotifyFileSystemWatcher, FileClosedAfterWrite)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy spy(&watcher, &InotifyFileSystemWatcher::fileClosed);
    ASSERT_TRUE(spy.isValid());

    // 创建并写入后关闭文件
    QString filePath = dirPath + "/close_test.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("close me");
    file.close();

    int count = waitForSignal(spy);
    EXPECT_EQ(count, 1);

    // 对于目录监控，name 是被关闭的子文件名
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);
    EXPECT_EQ(args.at(1).toString(), "close_test.txt");
}

// ========== 属性变更信号 ==========

TEST_F(TestInotifyFileSystemWatcher, FileAttributeChanged)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    // 先创建文件并等待创建完成
    QString filePath = dirPath + "/chmod_test.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("chmod me");
    file.close();

    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    QSignalSpy spy(&watcher, &InotifyFileSystemWatcher::fileAttributeChanged);
    ASSERT_TRUE(spy.isValid());

    // 修改文件权限
    QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner);

    int count = waitForSignal(spy);
    EXPECT_EQ(count, 1);

    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);
    EXPECT_EQ(args.at(1).toString(), "chmod_test.txt");
}

// ========== 文件重命名/移动信号 ==========

TEST_F(TestInotifyFileSystemWatcher, FileRenameWithinWatchedDirectory)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    // 先创建文件
    QString oldPath = dirPath + "/oldname.txt";
    QFile file(oldPath);
    file.open(QIODevice::WriteOnly);
    file.write("rename me");
    file.close();

    // 等待创建完成
    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    QSignalSpy moveSpy(&watcher, &InotifyFileSystemWatcher::fileMoved);
    ASSERT_TRUE(moveSpy.isValid());

    // 重命名文件
    QString newPath = dirPath + "/newname.txt";
    QFile::rename(oldPath, newPath);

    int count = waitForSignal(moveSpy);
    // inotify 在同一目录中重命名时，MOVED_FROM 和 MOVED_TO 通过 cookie 配对
    EXPECT_EQ(count, 1);

    QList<QVariant> args = moveSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirPath);   // fromPath
    EXPECT_EQ(args.at(1).toString(), "oldname.txt"); // fromName
    EXPECT_EQ(args.at(2).toString(), dirPath);    // toPath
    EXPECT_EQ(args.at(3).toString(), "newname.txt"); // toName
}

TEST_F(TestInotifyFileSystemWatcher, FileMoveAcrossWatchedDirectories)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();

    // 创建两个子目录
    QString dirA = dirPath + "/dirA";
    QString dirB = dirPath + "/dirB";
    QDir().mkpath(dirA);
    QDir().mkpath(dirB);

    watcher.addPath(dirA);
    watcher.addPath(dirB);

    // 等待目录创建信号
    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    while (createSpy.count() < 2 && createSpy.wait(1000))
        ;

    // 在 dirA 中创建文件
    QString filePath = dirA + "/movefile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("move me");
    file.close();

    waitForSignal(createSpy);

    QSignalSpy moveSpy(&watcher, &InotifyFileSystemWatcher::fileMoved);
    ASSERT_TRUE(moveSpy.isValid());

    // 移动文件到 dirB
    QFile::rename(filePath, dirB + "/movefile.txt");

    int count = waitForSignal(moveSpy);
    EXPECT_EQ(count, 1);

    QList<QVariant> args = moveSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), dirA);           // fromPath
    EXPECT_EQ(args.at(1).toString(), "movefile.txt"); // fromName
    EXPECT_EQ(args.at(2).toString(), dirB);           // toPath
    EXPECT_EQ(args.at(3).toString(), "movefile.txt"); // toName
}

// ========== WatchFlags 过滤 ==========

TEST_F(TestInotifyFileSystemWatcher, WatchFlagsFiltering_OnlyClose)
{
    InotifyFileSystemWatcher watcher;
    // 只监控 FileClose 事件
    watcher.setWatchFlags(InotifyFileSystemWatcher::FileClose);
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy closeSpy(&watcher, &InotifyFileSystemWatcher::fileClosed);
    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    QSignalSpy modifySpy(&watcher, &InotifyFileSystemWatcher::fileModified);
    ASSERT_TRUE(closeSpy.isValid());
    ASSERT_TRUE(createSpy.isValid());
    ASSERT_TRUE(modifySpy.isValid());

    // 创建文件并写入关闭
    QString filePath = dirPath + "/filter_test.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("content");
    file.close();

    waitForSignal(closeSpy);

    // 只有 fileClosed 信号应该触发
    // fileCreated 使用 IN_CREATE，由于 AllEvents 不包含 FileCreate 时不会触发
    // 但默认的 mask 始终包含 IN_DELETE_SELF | IN_MOVE_SELF
    // FileClose 过滤下只应有 fileClosed
    EXPECT_GE(closeSpy.count(), 1);
    EXPECT_EQ(createSpy.count(), 0);
    EXPECT_EQ(modifySpy.count(), 0);
}

// ========== 信号去重 ==========

TEST_F(TestInotifyFileSystemWatcher, NoDuplicateSignalsForSingleOperation)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    ASSERT_TRUE(createSpy.isValid());

    // 创建单个文件
    QString filePath = dirPath + "/unique_create.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("unique");
    file.close();

    waitForSignal(createSpy);

    // 额外等待一小段时间确认不会有重复信号
    createSpy.wait(500);

    // 应该只收到一次 fileCreated 信号
    EXPECT_EQ(createSpy.count(), 1);
}

// ========== 监控自身被删除 ==========

TEST_F(TestInotifyFileSystemWatcher, WatchedPathSelfDeleted)
{
    InotifyFileSystemWatcher watcher;
    QString subDirPath = testDir->path() + "/selfdelete_dir";
    QDir().mkpath(subDirPath);

    watcher.addPath(subDirPath);

    QSignalSpy deleteSpy(&watcher, &InotifyFileSystemWatcher::fileDeleted);
    ASSERT_TRUE(deleteSpy.isValid());

    // 删除被监控的目录本身
    QDir().rmdir(subDirPath);

    int count = waitForSignal(deleteSpy);
    EXPECT_EQ(count, 1);

    // 自删除信号：path 是目录本身，name 为空
    QList<QVariant> args = deleteSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), subDirPath);
    EXPECT_EQ(args.at(1).toString(), "");
}

// ========== 文件监控 ==========

TEST_F(TestInotifyFileSystemWatcher, FileWatchReceivesAttributeChange)
{
    InotifyFileSystemWatcher watcher;
    QString filePath = testDir->path() + "/watched_file.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("watched");
    file.close();

    watcher.addPath(filePath);
    ASSERT_TRUE(watcher.files().contains(filePath));

    QSignalSpy attribSpy(&watcher, &InotifyFileSystemWatcher::fileAttributeChanged);
    ASSERT_TRUE(attribSpy.isValid());

    // 修改被监控文件的权限
    QFile::setPermissions(filePath, QFile::ReadOwner);

    int count = waitForSignal(attribSpy);
    EXPECT_EQ(count, 1);
}

// ========== 并发操作不产生重复信号 ==========

TEST_F(TestInotifyFileSystemWatcher, RapidOperationsNoDuplicates)
{
    InotifyFileSystemWatcher watcher;
    QString dirPath = testDir->path();
    watcher.addPath(dirPath);

    QSignalSpy createSpy(&watcher, &InotifyFileSystemWatcher::fileCreated);
    ASSERT_TRUE(createSpy.isValid());

    // 快速创建多个文件
    for (int i = 0; i < 5; ++i) {
        QString filePath = dirPath + QString("/rapid_%1.txt").arg(i);
        QFile file(filePath);
        file.open(QIODevice::WriteOnly);
        file.write("data");
        file.close();
    }

    // 等待所有信号
    waitForSignal(createSpy, 3000);
    // 额外等待确保所有异步信号到达
    createSpy.wait(1000);

    // 应该恰好收到 5 次 fileCreated
    EXPECT_EQ(createSpy.count(), 5);
}
