// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include <QThread>
#include <QStorageInfo>
#include <QTextStream>

#include "fsmonitor/fsmonitor.h"
#include "fsmonitor/fsmonitorworker.h"
#include "fsmonitor/fsmonitor_p.h"
#include "utils/textindexconfig.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-search/dsearch_global.h>
#include <DFileSystemWatcher>

SERVICETEXTINDEX_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE

class UT_FSMonitor : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        EXPECT_TRUE(tempDir->isValid());
        testPath = tempDir->path();

        // Create test directory structure
        createTestDirectories();

        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, folderExcludeFilters), [](TextIndexConfig *) -> QStringList {
            __DBG_STUB_INVOKE__
            QStringList filters;
            filters << ".git"
                    << ".cache"
                    << "node_modules";
            return filters;
        });

        stub.set_lamda(ADDR(TextIndexConfig, indexHiddenFiles), [this](TextIndexConfig *) -> bool {
            __DBG_STUB_INVOKE__
            return mockIndexHiddenFiles;
        });

        // Mock DFMSEARCH::Global
        stub.set_lamda(ADDR(DFMSEARCH::Global, isHiddenPathOrInHiddenDir), [](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.contains("/.hidden/");
        });

        // Mock ProtocolUtils
        stub.set_lamda(&ProtocolUtils::isLocalFile, [](const QUrl &url) -> bool {
            __DBG_STUB_INVOKE__
            return url.isLocalFile();
        });

        // Mock DFileSystemWatcher - use function pointer for overloaded functions
        using AddPathFunc = bool (DFileSystemWatcher::*)(const QString &);
        stub.set_lamda(static_cast<AddPathFunc>(&DFileSystemWatcher::addPath), [this](DFileSystemWatcher *, const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            if (mockWatcherFailPaths.contains(path)) {
                return false;
            }
            mockWatchedPaths.insert(path);
            return true;
        });

        using RemovePathFunc = bool (DFileSystemWatcher::*)(const QString &);
        stub.set_lamda(static_cast<RemovePathFunc>(&DFileSystemWatcher::removePath), [this](DFileSystemWatcher *, const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            mockWatchedPaths.remove(path);
            return true;
        });

        using RemovePathsFunc = QStringList (DFileSystemWatcher::*)(const QStringList &);
        stub.set_lamda(static_cast<RemovePathsFunc>(&DFileSystemWatcher::removePaths), [this](DFileSystemWatcher *, const QStringList &paths) -> QStringList {
            __DBG_STUB_INVOKE__
            for (const QString &path : paths) {
                mockWatchedPaths.remove(path);
            }
            return QStringList();
        });

        // Mock QStorageInfo
        stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
            __DBG_STUB_INVOKE__
            return "ext4";
        });

        // Mock QThread operations - handle overloaded functions
        using QThreadStartFunc = void (QThread::*)(QThread::Priority);
        stub.set_lamda(static_cast<QThreadStartFunc>(&QThread::start), [this](QThread *, QThread::Priority) {
            __DBG_STUB_INVOKE__
            mockThreadStarted = true;
        });

        stub.set_lamda(&QThread::quit, [this](QThread *) {
            __DBG_STUB_INVOKE__
            mockThreadStarted = false;
        });

        using QThreadWaitFunc = bool (QThread::*)(QDeadlineTimer);
        stub.set_lamda(static_cast<QThreadWaitFunc>(&QThread::wait), [](QThread *, QDeadlineTimer) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&QThread::isRunning, [this](QThread *) -> bool {
            __DBG_STUB_INVOKE__
            return mockThreadStarted;
        });

        // Mock FSMonitorWorker
        stub.set_lamda(ADDR(FSMonitorWorker, setExclusionChecker), [](FSMonitorWorker *, const std::function<bool(const QString &)> &) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(ADDR(FSMonitorWorker, setMaxFastScanResults), [](FSMonitorWorker *, int) {
            __DBG_STUB_INVOKE__
        });

        // Reset mock states
        mockIndexHiddenFiles = false;
        mockWatchedPaths.clear();
        mockWatcherFailPaths.clear();
        mockThreadStarted = false;
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    void createTestDirectories()
    {
        QDir root(testPath);

        // Create normal directories
        root.mkpath("docs");
        root.mkpath("images");
        root.mkpath("temp");
        root.mkpath(".hidden");
        root.mkpath(".git");

        // Create some test files
        createFile("docs/file1.txt", "content1");
        createFile("docs/file2.doc", "content2");
        createFile("images/photo.jpg", "binary content");
        createFile("temp/temp.txt", "temp content");
        createFile(".hidden/secret.txt", "hidden content");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QString fullPath = QDir(testPath).absoluteFilePath(relativePath);
        QFileInfo fileInfo(fullPath);
        QDir().mkpath(fileInfo.absolutePath());

        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << content;
        }
    }

    void mockMaxUserWatchesFile(int maxWatches)
    {
        using QTextStreamReadLineFunc = QString (QTextStream::*)(qint64);
        stub.set_lamda(static_cast<QTextStreamReadLineFunc>(&QTextStream::readLine), [maxWatches](QTextStream *, qint64) -> QString {
            __DBG_STUB_INVOKE__
            return QString::number(maxWatches);
        });
    }

protected:
    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    QString testPath;

    // Mock states
    bool mockIndexHiddenFiles = false;
    QSet<QString> mockWatchedPaths;
    QSet<QString> mockWatcherFailPaths;
    bool mockThreadStarted = false;
    bool mockProcFileOpened = false;
};

// Test FSMonitor singleton access
TEST_F(UT_FSMonitor, InstanceAccess_ShouldReturnSameInstance)
{
    FSMonitor &instance1 = FSMonitor::instance();
    FSMonitor &instance2 = FSMonitor::instance();

    EXPECT_EQ(&instance1, &instance2);
}

// Test initialization with valid paths
TEST_F(UT_FSMonitor, Initialize_WithValidPaths_ShouldSucceed)
{
    FSMonitor &monitor = FSMonitor::instance();

    QStringList rootPaths;
    rootPaths << testPath;
    bool result = monitor.initialize(rootPaths);

    EXPECT_TRUE(result);
    EXPECT_FALSE(monitor.isActive());
}

// Test initialization with empty paths
TEST_F(UT_FSMonitor, Initialize_WithEmptyPaths_ShouldFail)
{
    FSMonitor &monitor = FSMonitor::instance();

    QStringList rootPaths;
    bool result = monitor.initialize(rootPaths);

    EXPECT_FALSE(result);
}

// Test initialization with non-existent paths
TEST_F(UT_FSMonitor, Initialize_WithNonExistentPaths_ShouldFail)
{
    FSMonitor &monitor = FSMonitor::instance();

    QStringList rootPaths;
    rootPaths << "/non/existent/path";
    bool result = monitor.initialize(rootPaths);

    EXPECT_FALSE(result);
}

// Test start monitoring
TEST_F(UT_FSMonitor, Start_AfterInitialization_ShouldSucceed)
{
    mockMaxUserWatchesFile(8192);

    FSMonitor &monitor = FSMonitor::instance();
    QStringList rootPaths;
    rootPaths << testPath;
    monitor.initialize(rootPaths);

    bool result = monitor.start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(monitor.isActive());
    EXPECT_TRUE(mockThreadStarted);
}

// Test blacklisted paths management
TEST_F(UT_FSMonitor, BlacklistedPaths_AddAndRemove_ShouldWork)
{
    FSMonitor &monitor = FSMonitor::instance();

    // Initially should have default blacklisted paths
    QStringList initialPaths = monitor.blacklistedPaths();
    EXPECT_TRUE(initialPaths.contains(".git"));
    EXPECT_TRUE(initialPaths.contains(".cache"));

    // Add new blacklisted path
    monitor.addBlacklistedPath("/tmp/test");
    QStringList pathsAfterAdd = monitor.blacklistedPaths();
    EXPECT_TRUE(pathsAfterAdd.contains("/tmp/test"));

    // Add multiple paths
    QStringList multiplePaths;
    multiplePaths << "/tmp/test2"
                  << "/tmp/test3";
    monitor.addBlacklistedPaths(multiplePaths);
    QStringList pathsAfterMultiAdd = monitor.blacklistedPaths();
    EXPECT_TRUE(pathsAfterMultiAdd.contains("/tmp/test2"));
    EXPECT_TRUE(pathsAfterMultiAdd.contains("/tmp/test3"));

    // Remove path
    monitor.removeBlacklistedPath("/tmp/test");
    QStringList pathsAfterRemove = monitor.blacklistedPaths();
    EXPECT_FALSE(pathsAfterRemove.contains("/tmp/test"));
    EXPECT_TRUE(pathsAfterRemove.contains("/tmp/test2"));
}

// Test resource usage settings
TEST_F(UT_FSMonitor, ResourceUsage_SetAndGet_ShouldWork)
{
    FSMonitor &monitor = FSMonitor::instance();

    // Test default value
    EXPECT_DOUBLE_EQ(monitor.maxResourceUsage(), 0.5);

    // Test setting valid values
    monitor.setMaxResourceUsage(0.8);
    EXPECT_DOUBLE_EQ(monitor.maxResourceUsage(), 0.8);

    monitor.setMaxResourceUsage(0.1);
    EXPECT_DOUBLE_EQ(monitor.maxResourceUsage(), 0.1);

    monitor.setMaxResourceUsage(1.0);
    EXPECT_DOUBLE_EQ(monitor.maxResourceUsage(), 1.0);

    // Test clamping - values below 0.1
    monitor.setMaxResourceUsage(0.05);
    EXPECT_DOUBLE_EQ(monitor.maxResourceUsage(), 0.1);

    // Test clamping - values above 1.0
    monitor.setMaxResourceUsage(1.5);
    EXPECT_DOUBLE_EQ(monitor.maxResourceUsage(), 1.0);
}

// Test watch count information
TEST_F(UT_FSMonitor, WatchCount_Information_ShouldReturnCorrectValues)
{
    mockMaxUserWatchesFile(8192);

    FSMonitor &monitor = FSMonitor::instance();
    QStringList rootPaths;
    rootPaths << testPath;
    monitor.initialize(rootPaths);

    // Before starting, current count should be 0
    EXPECT_EQ(monitor.currentWatchCount(), 0);

    monitor.start();

    // Max available count should be system limit
    EXPECT_EQ(monitor.maxAvailableWatchCount(), 8192);
}

// Test fast scan settings
TEST_F(UT_FSMonitor, FastScan_SetAndGet_ShouldWork)
{
    FSMonitor &monitor = FSMonitor::instance();

    // Test default value
    EXPECT_TRUE(monitor.useFastScan());

    // Test setting when not active
    monitor.d_ptr->active = false;
    monitor.setUseFastScan(false);
    EXPECT_FALSE(monitor.useFastScan());

    monitor.setUseFastScan(true);
    EXPECT_TRUE(monitor.useFastScan());
}

// Test stop monitoring
TEST_F(UT_FSMonitor, Stop_WhenActive_ShouldStopMonitoring)
{
    mockMaxUserWatchesFile(8192);

    FSMonitor &monitor = FSMonitor::instance();
    QStringList rootPaths;
    rootPaths << testPath;
    monitor.initialize(rootPaths);
    monitor.start();

    EXPECT_TRUE(monitor.isActive());

    monitor.stop();

    EXPECT_FALSE(monitor.isActive());
}

// Test FSMonitorPrivate::isExternalMount with empty path
TEST_F(UT_FSMonitor, IsExternalMount_WithEmptyPath_ShouldReturnFalse)
{
    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount("");
    EXPECT_FALSE(result);
}

// Test FSMonitorPrivate::isExternalMount with invalid storage
TEST_F(UT_FSMonitor, IsExternalMount_WithInvalidStorage_ShouldReturnFalse)
{
    // Mock QStorageInfo to return invalid storage
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_FALSE(result);
}

// Test FSMonitorPrivate::isExternalMount with not ready storage
TEST_F(UT_FSMonitor, IsExternalMount_WithNotReadyStorage_ShouldReturnFalse)
{
    // Mock QStorageInfo to return not ready storage
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_FALSE(result);
}

// Test FSMonitorPrivate::isExternalMount with FUSE filesystem
TEST_F(UT_FSMonitor, IsExternalMount_WithFuseFilesystem_ShouldReturnTrue)
{
    // Mock QStorageInfo to return FUSE filesystem
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "fuse.sshfs";
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_TRUE(result);
}

// Test FSMonitorPrivate::isExternalMount with NFS filesystem
TEST_F(UT_FSMonitor, IsExternalMount_WithNfsFilesystem_ShouldReturnTrue)
{
    // Mock QStorageInfo to return NFS filesystem
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "nfs4";
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_TRUE(result);
}

// Test FSMonitorPrivate::isExternalMount with CIFS filesystem
TEST_F(UT_FSMonitor, IsExternalMount_WithCifsFilesystem_ShouldReturnTrue)
{
    // Mock QStorageInfo to return CIFS filesystem
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "cifs";
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_TRUE(result);
}

// Test FSMonitorPrivate::isExternalMount with various network filesystems
TEST_F(UT_FSMonitor, IsExternalMount_WithNetworkFilesystems_ShouldReturnTrue)
{
    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    // Mock QStorageInfo to return valid and ready storage
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Test various network filesystem types
    QStringList networkFsTypes = {
        "nfs", "nfs4", "cifs", "smb", "smb2", "smbfs", "webdav",
        "ceph", "glusterfs", "moosefs", "lustre", "overlay", "aufs", "9p",
        "sftp", "curlftpfs", "davfs"
    };

    for (const QString &fsType : networkFsTypes) {
        stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [fsType](QStorageInfo *) -> QByteArray {
            __DBG_STUB_INVOKE__
            return fsType.toUtf8();
        });

        bool result = d_ptr->isExternalMount(testPath);
        EXPECT_TRUE(result) << "Failed for filesystem type: " << fsType.toStdString();
    }
}

// Test FSMonitorPrivate::isExternalMount with local filesystem
TEST_F(UT_FSMonitor, IsExternalMount_WithLocalFilesystem_ShouldReturnFalse)
{
    // Mock QStorageInfo to return local filesystem
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "ext4";
    });

    // Mock ProtocolUtils to return local file
    stub.set_lamda(&ProtocolUtils::isLocalFile, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_FALSE(result);
}

// Test FSMonitorPrivate::isExternalMount with various local filesystems
TEST_F(UT_FSMonitor, IsExternalMount_WithVariousLocalFilesystems_ShouldReturnFalse)
{
    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    // Mock QStorageInfo to return valid and ready storage
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock ProtocolUtils to return local file
    stub.set_lamda(&ProtocolUtils::isLocalFile, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Test various local filesystem types
    QStringList localFsTypes = {
        "ext4", "ext3", "ext2", "xfs", "btrfs", "zfs", "reiserfs", 
        "jfs", "ntfs", "vfat", "fat32", "fat16", "exfat", "hfs", "hfsplus"
    };

    for (const QString &fsType : localFsTypes) {
        stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [fsType](QStorageInfo *) -> QByteArray {
            __DBG_STUB_INVOKE__
            return fsType.toUtf8();
        });

        bool result = d_ptr->isExternalMount(testPath);
        EXPECT_FALSE(result) << "Failed for local filesystem type: " << fsType.toStdString();
    }
}

// Test FSMonitorPrivate::isExternalMount with non-local file URL
TEST_F(UT_FSMonitor, IsExternalMount_WithNonLocalFileUrl_ShouldReturnTrue)
{
    // Mock QStorageInfo to return valid local filesystem
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "ext4";
    });

    // Mock ProtocolUtils to return non-local file
    stub.set_lamda(&ProtocolUtils::isLocalFile, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_TRUE(result);
}

// Test FSMonitorPrivate::isExternalMount with case-insensitive filesystem types
TEST_F(UT_FSMonitor, IsExternalMount_WithCaseInsensitiveFilesystemTypes_ShouldWork)
{
    FSMonitor &monitor = FSMonitor::instance();
    FSMonitorPrivate *d_ptr = monitor.d_ptr.get();

    // Mock QStorageInfo to return valid and ready storage
    stub.set_lamda(ADDR(QStorageInfo, isValid), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(QStorageInfo, isReady), [](QStorageInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Test case-insensitive matching for FUSE
    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "FUSE.SSHFS";  // uppercase
    });

    bool result = d_ptr->isExternalMount(testPath);
    EXPECT_TRUE(result);

    // Test case-insensitive matching for network filesystems
    stub.set_lamda(ADDR(QStorageInfo, fileSystemType), [](QStorageInfo *) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "NFS4";  // uppercase
    });

    result = d_ptr->isExternalMount(testPath);
    EXPECT_TRUE(result);
}
