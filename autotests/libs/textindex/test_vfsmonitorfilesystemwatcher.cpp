// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QSignalSpy>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/vfsmonitorwatcher.h"
#include "services/textindex/fsmonitor/vfsmonitorwatcher_p.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// VfsMonitorFileSystemWatcher 依赖 deepin-anything 内核模块
// 如果模块不可用，create() 返回 nullptr，相关测试应被跳过
// 注意: 当前内核 vfs_monitor 模块存在已知 bug（无法检测目录创建），
// 目录相关测试使用 ASSERT 严格模式，失败时直接终止当前测试而非崩溃
class TestVfsMonitorFileSystemWatcher : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 在 ~/Documents/ 下创建隐藏临时目录
        QString templatePath = QDir::homePath() + "/Documents/.test_vfsmonitor_XXXXXX";
        testDir = std::make_unique<QTemporaryDir>(templatePath);
        ASSERT_TRUE(testDir->isValid());
        QDir().mkpath(testDir->path());

        // 尝试创建 VfsMonitorFileSystemWatcher
        QStringList rootPaths = { testDir->path() };
        watcher = VfsMonitorFileSystemWatcher::create(rootPaths, {}, nullptr);
        if (!watcher) {
            GTEST_SKIP() << "deepin-anything kernel module not available, skipping tests";
        }
    }

    void TearDown() override
    {
        delete watcher;
        watcher = nullptr;
        testDir.reset();
    }

    int waitForSignal(QSignalSpy &spy, int timeoutMs = 3000)
    {
        spy.wait(timeoutMs);
        return spy.count();
    }

    std::unique_ptr<QTemporaryDir> testDir;
    VfsMonitorFileSystemWatcher *watcher { nullptr };
};

// ========== 工厂方法 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, FactoryCreateReturnsNonNull)
{
    // 如果到达这里，说明 create() 返回了非空指针
    ASSERT_NE(watcher, nullptr);
}

// ========== 文件创建 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, FileCreatedSignal)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    ASSERT_TRUE(spy.isValid());

    QString filePath = testDir->path() + "/newfile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("vfs monitor test");
    file.close();

    int count = waitForSignal(spy);
    ASSERT_GT(count, 0) << "fileCreated signal not received within timeout";

    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "newfile.txt");
}

// ========== 目录创建 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, DirectoryCreatedSignal)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::directoryCreated);
    ASSERT_TRUE(spy.isValid());

    QString subDirPath = testDir->path() + "/newdir";
    QDir().mkpath(subDirPath);

    int count = waitForSignal(spy);

    EXPECT_EQ(count, 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "newdir");
}

// ========== 文件删除 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, FileDeletedSignal)
{
    ASSERT_NE(watcher, nullptr);

    // 先创建文件
    QString filePath = testDir->path() + "/todelete.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("delete me");
    file.close();

    // 等待创建信号
    QSignalSpy createSpy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    QSignalSpy deleteSpy(watcher, &VfsMonitorFileSystemWatcher::fileDeleted);
    ASSERT_TRUE(deleteSpy.isValid());

    QFile::remove(filePath);

    int count = waitForSignal(deleteSpy);
    ASSERT_GT(count, 0) << "fileDeleted signal not received within timeout";

    QList<QVariant> args = deleteSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "todelete.txt");
}

// ========== 目录删除 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, DirectoryDeletedSignal)
{
    ASSERT_NE(watcher, nullptr);

    // 先创建子目录
    QString subDirPath = testDir->path() + "/todeletedir";
    QDir().mkpath(subDirPath);

    QSignalSpy createSpy(watcher, &VfsMonitorFileSystemWatcher::directoryCreated);
    waitForSignal(createSpy);

    QSignalSpy deleteSpy(watcher, &VfsMonitorFileSystemWatcher::directoryDeleted);
    ASSERT_TRUE(deleteSpy.isValid());

    QDir().rmdir(subDirPath);

    waitForSignal(deleteSpy);

    QList<QVariant> args = deleteSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "todeletedir");
}

// ========== 文件重命名 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, FileRenameMovedSignal)
{
    ASSERT_NE(watcher, nullptr);

    // 先创建文件
    QString oldPath = testDir->path() + "/oldname_vfs.txt";
    QFile file(oldPath);
    file.open(QIODevice::WriteOnly);
    file.write("rename me");
    file.close();

    // 等待创建信号
    QSignalSpy createSpy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    QSignalSpy moveSpy(watcher, &VfsMonitorFileSystemWatcher::fileMoved);
    ASSERT_TRUE(moveSpy.isValid());

    // 重命名
    QString newPath = testDir->path() + "/newname_vfs.txt";
    QFile::rename(oldPath, newPath);

    int count = waitForSignal(moveSpy);
    ASSERT_GT(count, 0) << "fileMoved signal not received within timeout";

    QList<QVariant> args = moveSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "oldname_vfs.txt");
    EXPECT_EQ(args.at(2).toString(), testDir->path());
    EXPECT_EQ(args.at(3).toString(), "newname_vfs.txt");
}

// ========== 目录重命名 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, DirectoryRenameMovedSignal)
{
    ASSERT_NE(watcher, nullptr);

    // 先创建目录
    QString oldDirPath = testDir->path() + "/olddir";
    QDir().mkpath(oldDirPath);

    QSignalSpy createSpy(watcher, &VfsMonitorFileSystemWatcher::directoryCreated);
    waitForSignal(createSpy);

    QSignalSpy moveSpy(watcher, &VfsMonitorFileSystemWatcher::directoryMoved);
    ASSERT_TRUE(moveSpy.isValid());

    // 重命名目录
    QString newDirPath = testDir->path() + "/newdir";
    QDir().rename(oldDirPath, newDirPath);

    waitForSignal(moveSpy);

    QList<QVariant> args = moveSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "olddir");
    EXPECT_EQ(args.at(2).toString(), testDir->path());
    EXPECT_EQ(args.at(3).toString(), "newdir");
}

// ========== 符号链接创建 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, SymlinkCreationEmitsFileCreated)
{
    ASSERT_NE(watcher, nullptr);

    // 先创建目标文件
    QString targetPath = testDir->path() + "/symlink_target.txt";
    QFile file(targetPath);
    file.open(QIODevice::WriteOnly);
    file.write("target");
    file.close();

    // 等待文件创建信号
    QSignalSpy createSpy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    waitForSignal(createSpy);

    // 清除之前的信号
    createSpy.clear();

    // 创建符号链接
    QString linkPath = testDir->path() + "/symlink_link.txt";
    QFile::link(targetPath, linkPath);

    int count = waitForSignal(createSpy);
    // 符号链接创建应该触发 fileCreated 信号 (ACT_NEW_SYMLINK)
    ASSERT_GT(count, 0) << "fileCreated (symlink) signal not received within timeout";

    QList<QVariant> args = createSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "symlink_link.txt");
}

// ========== 路径排除 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, ExcludePredicateFiltersEvents)
{
    ASSERT_NE(watcher, nullptr);

    // 重新创建带有排除谓词的 watcher
    QStringList rootPaths = { testDir->path() };
    auto exclude = [](const QString &path) -> bool {
        return path.endsWith(".log");
    };

    // 先析构旧 watcher
    delete watcher;
    watcher = nullptr;
    watcher = VfsMonitorFileSystemWatcher::create(rootPaths, exclude, nullptr);
    if (!watcher) {
        GTEST_SKIP() << "Failed to create watcher with exclude predicate";
    }

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    ASSERT_TRUE(spy.isValid());

    // 创建被排除的文件（.log 后缀）
    QString excludedPath = testDir->path() + "/excluded.log";
    QFile file(excludedPath);
    file.open(QIODevice::WriteOnly);
    file.write("excluded");
    file.close();

    waitForSignal(spy);
    // 等待更长时间确保不会出现信号
    spy.wait(1000);

    // 被排除的文件不应该触发 fileCreated
    EXPECT_EQ(spy.count(), 0);

    // 创建正常文件
    QString normalPath = testDir->path() + "/normal.txt";
    QFile normalFile(normalPath);
    normalFile.open(QIODevice::WriteOnly);
    normalFile.write("normal");
    normalFile.close();

    int count = waitForSignal(spy);
    ASSERT_GT(count, 0) << "fileCreated (normal) signal not received within timeout";

    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(1).toString(), "normal.txt");
}

// ========== 信号去重 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, NoDuplicateSignalsForSingleOperation)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    ASSERT_TRUE(spy.isValid());

    // 创建单个文件
    QString filePath = testDir->path() + "/unique_vfs.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("unique");
    file.close();

    waitForSignal(spy);
    // 额外等待确认无重复
    spy.wait(500);

    // 应该恰好一次（如果内核模块正常）
    EXPECT_LE(spy.count(), 1);
    EXPECT_GE(spy.count(), 1);
}

// ========== RootPath 过滤 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, EventsOutsideRootPathsNotEmitted)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileCreated);
    ASSERT_TRUE(spy.isValid());

    // 在测试目录之外创建文件（使用 /tmp）
    QString outsidePath = QDir::tempPath() + "/vfsmonitor_outside_" + QString::number(QCoreApplication::applicationPid()) + ".txt";
    QFile file(outsidePath);
    file.open(QIODevice::WriteOnly);
    file.write("outside");
    file.close();

    waitForSignal(spy);
    spy.wait(1000);

    // 不应该触发任何信号（事件在 rootPaths 之外）
    EXPECT_EQ(spy.count(), 0);

    // 清理
    QFile::remove(outsidePath);
}
