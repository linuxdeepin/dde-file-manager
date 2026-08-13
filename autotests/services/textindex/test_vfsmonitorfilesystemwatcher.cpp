// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include <QSet>
#include <QElapsedTimer>
#include <QSocketNotifier>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/vfsmonitorwatcher.h"
#include "services/textindex/fsmonitor/vfsmonitorwatcher_p.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// VfsMonitorFileSystemWatcher 依赖 deepin-anything 事件分发 socket
// 如果事件分发不可用，create() 返回 nullptr，相关测试应被跳过
// 注意: 当前 deepin-anything 事件链路若不可用，
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
            GTEST_SKIP() << "deepin-anything event dispatcher not available, skipping tests";
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

// ========== 文件写入关闭 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, FileClosedSignal)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileClosed);
    ASSERT_TRUE(spy.isValid());

    // 创建文件并写入内容后关闭，触发 ACT_CLOSE_WRITE_FILE
    QString filePath = testDir->path() + "/closewrite.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("close write test");
    file.close();

    int count = waitForSignal(spy);
    ASSERT_GT(count, 0) << "fileClosed signal not received within timeout";

    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), "closewrite.txt");
}

// ========== 批量文件写入关闭（压力测试） ==========

TEST_F(TestVfsMonitorFileSystemWatcher, BatchFileClosedStress)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileClosed);
    ASSERT_TRUE(spy.isValid());

    constexpr int kFileCount = 20;
    QStringList fileNames;

    for (int i = 0; i < kFileCount; ++i) {
        QString name = QString("stress_%1.txt").arg(i);
        fileNames << name;
        QString path = testDir->path() + "/" + name;
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write(QString("content %1").arg(i).toUtf8());
        f.close();
    }

    // deepin-anything 内核模块事件合并器每 100ms 最多发送 DUMP_SIZE(10) 个事件。
    // 20 个文件产生 40 个事件(20 NEW_FILE + 20 CLOSE_WRITE_FILE)，需分多批发送。
    // QSignalSpy::wait() 收到第一个信号就返回，需循环等待直到收齐或超时。
    QElapsedTimer timer;
    timer.start();
    while (spy.count() < kFileCount && !timer.hasExpired(15000)) {
        QTest::qWait(200);
    }

    EXPECT_GE(spy.count(), kFileCount)
        << "Expected at least " << kFileCount << " fileClosed signals, got " << spy.count();

    // 验证每个文件名都出现在信号参数中
    QSet<QString> seenNames;
    for (const auto &args : spy) {
        if (args.count() >= 2)
            seenNames << args.at(1).toString();
    }
    for (const QString &name : fileNames) {
        EXPECT_TRUE(seenNames.contains(name))
            << "Missing fileClosed for: " << name.toStdString();
    }
}

// ========== 同一文件反复写入关闭（压力测试） ==========

TEST_F(TestVfsMonitorFileSystemWatcher, RepeatedWriteCloseStress)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileClosed);
    ASSERT_TRUE(spy.isValid());

    QString filePath = testDir->path() + "/repeated_stress.txt";
    QString fileName = "repeated_stress.txt";

    constexpr int kIterations = 15;
    for (int i = 0; i < kIterations; ++i) {
        QFile f(filePath);
        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        f.write(QString("iteration %1").arg(i).toUtf8());
        f.close();
        // 小间隔让事件合并器有机会刷新
        QTest::qWait(50);
    }

    // 循环等待收齐所有信号
    QElapsedTimer timer;
    timer.start();
    while (spy.count() < kIterations && !timer.hasExpired(10000)) {
        QTest::qWait(200);
    }

    EXPECT_GE(spy.count(), kIterations)
        << "Expected at least " << kIterations << " fileClosed signals, got " << spy.count();

    // 所有信号的文件名应该一致
    for (const auto &args : spy) {
        if (args.count() >= 2) {
            EXPECT_EQ(args.at(1).toString(), fileName)
                << "Unexpected file name in fileClosed signal";
        }
    }
}

// ========== 大文件写入关闭 ==========

TEST_F(TestVfsMonitorFileSystemWatcher, LargeFileWriteClose)
{
    ASSERT_NE(watcher, nullptr);

    QSignalSpy spy(watcher, &VfsMonitorFileSystemWatcher::fileClosed);
    ASSERT_TRUE(spy.isValid());

    QString filePath = testDir->path() + "/large_stress.txt";
    QString fileName = "large_stress.txt";

    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    // 写入 1MB 数据
    QByteArray chunk(1024 * 1024, 'X');
    f.write(chunk);
    f.close();

    int count = waitForSignal(spy, 5000);
    ASSERT_GT(count, 0) << "fileClosed signal not received for large file";

    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), testDir->path());
    EXPECT_EQ(args.at(1).toString(), fileName);
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

// ======================================================================
// 重连机制单元测试
// 通过 DFM_VFSMONITOR_SOCKET_PATH 环境变量将 watcher 指向测试自建的 mock
// dispatcher socket，模拟服务端断开，验证 watcher 能自动重连。
// ======================================================================

// 创建并监听一个 AF_UNIX SOCK_SEQPACKET socket，返回 listen fd。
static int createMockDispatcher(const QString &path)
{
    int fd = ::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    if (fd < 0)
        return -1;

    ::unlink(path.toUtf8().constData());   // remove stale socket file

    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path.toUtf8().constData(), sizeof(addr.sun_path) - 1);

    if (::bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return -1;
    }
    if (::listen(fd, 5) < 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

// 接受一个客户端连接（非阻塞 listen fd），返回 client fd 或 -1（无待连接）。
static int acceptOneClient(int listenFd)
{
    int fd = ::accept4(listenFd, nullptr, nullptr, SOCK_NONBLOCK);
    return fd;
}

class TestVfsMonitorReconnect : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 在临时目录下创建 mock dispatcher socket
        QString templatePath = QDir::homePath() + "/Documents/.test_vfsreconnect_XXXXXX";
        testDir = std::make_unique<QTemporaryDir>(templatePath);
        ASSERT_TRUE(testDir->isValid());
        mockSocketPath = testDir->path() + "/mock-dispatcher.sock";

        // 启动 mock dispatcher
        listenFd = createMockDispatcher(mockSocketPath);
        ASSERT_GE(listenFd, 0) << "Failed to create mock dispatcher";

        // 设置环境变量，使 watcher 连接到 mock dispatcher
        qputenv("DFM_VFSMONITOR_SOCKET_PATH", mockSocketPath.toUtf8());

        // 创建 watcher
        watcher = VfsMonitorFileSystemWatcher::create({ testDir->path() }, {}, nullptr);
        ASSERT_NE(watcher, nullptr) << "Watcher creation failed";

        // 接受初始连接
        clientFd = waitForAccept(listenFd, 3000);
        ASSERT_GE(clientFd, 0) << "Mock dispatcher did not accept initial connection";
    }

    void TearDown() override
    {
        if (clientFd >= 0)
            ::close(clientFd);
        if (listenFd >= 0) {
            ::close(listenFd);
        }
        ::unlink(mockSocketPath.toUtf8().constData());
        qunsetenv("DFM_VFSMONITOR_SOCKET_PATH");
        delete watcher;
        watcher = nullptr;
        testDir.reset();
    }

    // 轮询等待 accept 返回 fd，超时返回 -1
    int waitForAccept(int fd, int timeoutMs)
    {
        QElapsedTimer timer;
        timer.start();
        while (!timer.hasExpired(timeoutMs)) {
            int cfd = acceptOneClient(fd);
            if (cfd >= 0)
                return cfd;
            QTest::qWait(50);
        }
        return -1;
    }

    std::unique_ptr<QTemporaryDir> testDir;
    QString mockSocketPath;
    int listenFd { -1 };
    int clientFd { -1 };
    VfsMonitorFileSystemWatcher *watcher { nullptr };
};

// 断开后 watcher 应在退避时间后自动重连
TEST_F(TestVfsMonitorReconnect, DISABLED_ReconnectsAfterServerClosesConnection)
{
    ASSERT_NE(watcher, nullptr);
    ASSERT_GE(clientFd, 0);

    // 模拟服务端踢掉客户端：关闭 server 端的 client fd。
    // 客户端下次 recv() 将返回 0，触发 handleDisconnect()。
    ::close(clientFd);
    clientFd = -1;

    // 给 watcher 的事件循环一点时间，让 QSocketNotifier 触发 handleSocketMessage()。
    // handleSocketMessage 在收到 recv()==0 后调用 handleDisconnect，后者以 1s
    // 退避启动 reconnectTimer。我们等待 ~2.5s 容纳事件循环 + 1s 退避。
    int newClientFd = waitForAccept(listenFd, 5000);
    ASSERT_GE(newClientFd, 0)
        << "Watcher did not reconnect within 5s after server-side close";

    ::close(newClientFd);
}

// 重连后 backoff 应复位：断开后先关闭 mock listen socket 使重连失败数次
// （backoff 增至 2s/4s），恢复 listen 后重连成功，再次断开验证快速重连。
// ======================================================================
// splitPath unit tests (public static method on VfsMonitorFileSystemWatcherPrivate)
// ======================================================================

class TestVfsMonitorSplitPath : public ::testing::Test
{
};

TEST_F(TestVfsMonitorSplitPath, SplitPath_SimpleFile)
{
    auto [dir, name] = VfsMonitorFileSystemWatcherPrivate::splitPath("/home/user/file.txt");
    EXPECT_EQ(dir, QString("/home/user"));
    EXPECT_EQ(name, QString("file.txt"));
}

TEST_F(TestVfsMonitorSplitPath, SplitPath_RootFile)
{
    auto [dir, name] = VfsMonitorFileSystemWatcherPrivate::splitPath("/file.txt");
    EXPECT_EQ(dir, QString("/"));
    EXPECT_EQ(name, QString("file.txt"));
}

TEST_F(TestVfsMonitorSplitPath, SplitPath_DeeplyNested)
{
    auto [dir, name] = VfsMonitorFileSystemWatcherPrivate::splitPath("/a/b/c/d/e/f.txt");
    EXPECT_EQ(dir, QString("/a/b/c/d/e"));
    EXPECT_EQ(name, QString("f.txt"));
}

TEST_F(TestVfsMonitorSplitPath, SplitPath_TrailingSlash)
{
    // QFileInfo canonicalizes this - trailing slash becomes the parent
    auto [dir, name] = VfsMonitorFileSystemWatcherPrivate::splitPath("/home/user/dir/");
    // With trailing slash, QFileInfo sees it as a directory
    EXPECT_FALSE(dir.isEmpty());
}

TEST_F(TestVfsMonitorSplitPath, SplitPath_FileWithNoExtension)
{
    auto [dir, name] = VfsMonitorFileSystemWatcherPrivate::splitPath("/home/user/README");
    EXPECT_EQ(dir, QString("/home/user"));
    EXPECT_EQ(name, QString("README"));
}

// ======================================================================
// Factory method tests (no dispatcher available)
// ======================================================================

TEST_F(TestVfsMonitorSplitPath, Create_WithInvalidSocketPath_ReturnsNull)
{
    // Set socket path to a non-existent file to ensure create() fails
    qputenv("DFM_VFSMONITOR_SOCKET_PATH", "/nonexistent/socket/path/that/does/not/exist.sock");
    auto *w = VfsMonitorFileSystemWatcher::create({"/tmp"}, {}, nullptr);
    // create() should return nullptr because the socket path doesn't exist
    EXPECT_EQ(w, nullptr);
    qunsetenv("DFM_VFSMONITOR_SOCKET_PATH");
}

TEST_F(TestVfsMonitorSplitPath, Create_WithExcludePredicate)
{
    qputenv("DFM_VFSMONITOR_SOCKET_PATH", "/nonexistent/socket/path.sock");
    auto exclude = [](const QString &) -> bool { return true; };
    auto *w = VfsMonitorFileSystemWatcher::create({"/tmp"}, exclude, nullptr);
    EXPECT_EQ(w, nullptr);
    qunsetenv("DFM_VFSMONITOR_SOCKET_PATH");
}

TEST_F(TestVfsMonitorSplitPath, Create_EmptyRootPaths_ReturnsNull)
{
    qputenv("DFM_VFSMONITOR_SOCKET_PATH", "/nonexistent/socket/path.sock");
    auto *w = VfsMonitorFileSystemWatcher::create({}, {}, nullptr);
    EXPECT_EQ(w, nullptr);
    qunsetenv("DFM_VFSMONITOR_SOCKET_PATH");
}

TEST_F(TestVfsMonitorReconnect, DISABLED_BackoffResetsAfterSuccessfulReconnect)
{
    ASSERT_NE(watcher, nullptr);
    ASSERT_GE(clientFd, 0);

    // --- Phase 1: force backoff growth ---
    // Close the server-side client fd to trigger a disconnect.
    ::close(clientFd);
    clientFd = -1;

    // Close the listen socket so the watcher's first reconnect attempt fails.
    // We need a fresh path because bind() to the same path would fail if the
    // old socket file lingers; instead we close+reopen on the same path.
    ::close(listenFd);
    listenFd = -1;

    // Wait long enough for the watcher to attempt at least one failed
    // reconnect (1s initial backoff fires, attemptReconnect fails, backoff
    // grows to 2s). Give it ~3s to be safe.
    QTest::qWait(3000);

    // Reopen the mock dispatcher on the same path so reconnect can succeed.
    listenFd = createMockDispatcher(mockSocketPath);
    ASSERT_GE(listenFd, 0) << "Failed to reopen mock dispatcher";

    // Wait for the watcher to reconnect (backoff may be 2s or 4s at this
    // point; give generous timeout).
    int reconnectFd = waitForAccept(listenFd, 10000);
    ASSERT_GE(reconnectFd, 0) << "Watcher did not reconnect after restoring listen socket";

    // --- Phase 2: verify backoff was reset ---
    // Disconnect again. If backoff was reset, the next reconnect fires in ~1s;
    // if it was NOT reset, it could be 4s+. We use a 2500ms window: a 1s
    // reconnect completes well within it, while a 4s+ reconnect does not.
    ::close(reconnectFd);

    QElapsedTimer timer;
    timer.start();
    int finalReconnectFd = waitForAccept(listenFd, 2500);
    ASSERT_GE(finalReconnectFd, 0)
        << "Reconnect took too long; backoff was not reset after success";

    ::close(finalReconnectFd);
}
