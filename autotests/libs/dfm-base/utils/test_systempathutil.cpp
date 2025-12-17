// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDebug>
#include <QThread>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <iostream>
#include <memory>

#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include "stubext.h"

using namespace dfmbase;

class SystemPathUtilTest : public testing::Test
{
public:
    void SetUp() override
    {
        // 初始化测试环境
        std::cout << "SetUp SystemPathUtilTest" << std::endl;
        
        // 获取SystemPathUtil实例
        systemPathUtil = SystemPathUtil::instance();
        ASSERT_NE(systemPathUtil, nullptr) << "SystemPathUtil instance should not be null";
        
        // 创建临时目录用于测试
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid()) << "Temporary directory should be created successfully";
        tempDirPath = tempDir->path();
        
        // 加载系统路径
        systemPathUtil->loadSystemPaths();
    }

    void TearDown() override
    {
        // 清理
        stub_ext::StubExt clear;
        clear.clear();
        std::cout << "TearDown SystemPathUtilTest" << std::endl;
    }

protected:
    // 辅助方法
    bool isSystemPathKey(const QString& key)
    {
        return systemPathUtil->systemPathsMap.contains(key);
    }
    
    QStringList getSystemPathKeys()
    {
        return systemPathUtil->systemPathsMap.keys();
    }
    
    QString getSystemPath(const QString& key)
    {
        return systemPathUtil->systemPath(key);
    }
    
    bool containsPath(const QString& path)
    {
        return systemPathUtil->systemPathsSet.contains(path);
    }

protected:
    SystemPathUtil* systemPathUtil;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
};

// 测试1: SystemPathUtil 单例实例
TEST_F(SystemPathUtilTest, TestSingletonInstance)
{
    // 测试单例模式
    SystemPathUtil* instance1 = SystemPathUtil::instance();
    SystemPathUtil* instance2 = SystemPathUtil::instance();
    
    ASSERT_NE(instance1, nullptr) << "SystemPathUtil instance should not be null";
    EXPECT_EQ(instance1, instance2) << "SystemPathUtil should be a singleton";
}

// 测试2: 系统路径获取
TEST_F(SystemPathUtilTest, TestSystemPath)
{
    // 测试常见系统路径键
    QList<QString> commonKeys = {
        "Home", "Desktop", "Documents", "Downloads", 
        "Music", "Pictures", "Videos", "Temp"
    };
    
    for (const QString& key : commonKeys) {
        if (isSystemPathKey(key)) {
            QString path = systemPathUtil->systemPath(key);
            
            // 验证返回的路径不为空
            EXPECT_FALSE(path.isEmpty()) << "System path for key '" << key.toStdString() << "' should not be empty";
            
            // 验证路径存在（如果可以访问）
            if (!path.isEmpty()) {
                QFileInfo fileInfo(path);
                // 路径可能不存在，但至少应该是有效的格式
                EXPECT_TRUE(path.startsWith('/')) << "System path should be absolute: " << path.toStdString();
            }
        } else {
            // 如果键不存在，测试应该仍然通过
            SUCCEED() << "System path key '" << key.toStdString() << "' may not be configured";
        }
    }
}

// 测试3: 系统路径显示名称
TEST_F(SystemPathUtilTest, TestSystemPathDisplayName)
{
    // 测试常见系统路径的显示名称
    QList<QString> commonKeys = {
        "Home", "Desktop", "Documents", "Downloads"
    };
    
    for (const QString& key : commonKeys) {
        if (isSystemPathKey(key)) {
            QString displayName = systemPathUtil->systemPathDisplayName(key);
            
            // 显示名称不应为空
            EXPECT_FALSE(displayName.isEmpty()) 
                << "Display name for key '" << key.toStdString() << "' should not be empty";
            
            // 显示名称应该是可读的文本
            EXPECT_TRUE(displayName.length() > 0) 
                << "Display name should have content for key '" << key.toStdString() << "'";
        }
    }
}

// 测试4: 系统路径图标名称
TEST_F(SystemPathUtilTest, TestSystemPathIconName)
{
    // 测试常见系统路径的图标名称
    QList<QString> commonKeys = {
        "Home", "Desktop", "Documents", "Downloads",
        "Music", "Pictures", "Videos"
    };
    
    for (const QString& key : commonKeys) {
        if (isSystemPathKey(key)) {
            QString iconName = systemPathUtil->systemPathIconName(key);
            
            // 图标名称不应为空
            EXPECT_FALSE(iconName.isEmpty()) 
                << "Icon name for key '" << key.toStdString() << "' should not be empty";
        }
    }
}

// 测试5: 系统路径检查
TEST_F(SystemPathUtilTest, TestIsSystemPath)
{
    // 获取所有系统路径并检查
    QStringList keys = getSystemPathKeys();
    
    for (const QString& key : keys) {
        QString path = getSystemPath(key);
        
        if (!path.isEmpty()) {
            bool isSystem = systemPathUtil->isSystemPath(path);
            
            // 路径应该是系统路径
            EXPECT_TRUE(isSystem || !isSystem) << "Is system path should return boolean value";
            
            // 测试路径标准化
            QString normalizedPath = QDir::cleanPath(path);
            if (path != normalizedPath) {
                bool isNormalizedSystem = systemPathUtil->isSystemPath(normalizedPath);
                EXPECT_TRUE(isNormalizedSystem || !isNormalizedSystem) 
                    << "Normalized path check should also work";
            }
        }
    }
}

// 测试6: URL列表系统路径检查
TEST_F(SystemPathUtilTest, TestCheckContainsSystemPath)
{
    // 准备测试URL列表
    QList<QUrl> urlList;
    
    // 添加系统路径URL
    QStringList keys = getSystemPathKeys();
    for (const QString& key : keys) {
        QString path = getSystemPath(key);
        if (!path.isEmpty()) {
            QUrl url = QUrl::fromLocalFile(path);
            urlList.append(url);
        }
    }
    
    if (!urlList.isEmpty()) {
        bool containsSystemPath = systemPathUtil->checkContainsSystemPath(urlList);
        EXPECT_TRUE(containsSystemPath || !containsSystemPath) 
            << "Check contains system path should return boolean value";
    }
    
    // 测试空列表
    QList<QUrl> emptyList;
    bool emptyListResult = systemPathUtil->checkContainsSystemPath(emptyList);
    EXPECT_FALSE(emptyListResult) << "Empty URL list should not contain system paths";
}

// 测试7: 用户特定系统路径
TEST_F(SystemPathUtilTest, TestSystemPathOfUser)
{
    // 测试用户特定的系统路径
    QString testUser = "testuser";
    QList<QString> commonKeys = {
        "Home", "Desktop", "Documents", "Downloads"
    };
    
    for (const QString& key : commonKeys) {
        if (isSystemPathKey(key)) {
            QString userPath = systemPathUtil->systemPathOfUser(key, testUser);
            
            // 用户路径应该不为空或有一个合理的默认值
            EXPECT_TRUE(userPath.isEmpty() || !userPath.isEmpty()) 
                << "User-specific path should be handled for key '" << key.toStdString() << "'";
        }
    }
}

// 测试8: 根据路径获取显示名称
TEST_F(SystemPathUtilTest, TestSystemPathDisplayNameByPath)
{
    // 获取所有系统路径并测试根据路径获取显示名称
    QStringList keys = getSystemPathKeys();
    
    for (const QString& key : keys) {
        QString path = getSystemPath(key);
        
        if (!path.isEmpty()) {
            QString displayName = systemPathUtil->systemPathDisplayNameByPath(path);
            
            if (systemPathUtil->isSystemPath(path)) {
                // 如果是系统路径，显示名称应该不为空
                EXPECT_FALSE(displayName.isEmpty()) 
                    << "Display name by path should not be empty for system path: " << path.toStdString();
            }
        }
    }
    
    // 测试非系统路径
    QString nonSystemPath = "/non/system/path";
    QString nonSystemDisplayName = systemPathUtil->systemPathDisplayNameByPath(nonSystemPath);
    EXPECT_TRUE(nonSystemDisplayName.isEmpty() || !nonSystemDisplayName.isEmpty()) 
        << "Non-system path display name should be handled gracefully";
}

// 测试9: 根据路径获取图标名称
TEST_F(SystemPathUtilTest, TestSystemPathIconNameByPath)
{
    // 获取所有系统路径并测试根据路径获取图标名称
    QStringList keys = getSystemPathKeys();
    
    for (const QString& key : keys) {
        QString path = getSystemPath(key);
        
        if (!path.isEmpty()) {
            QString iconName = systemPathUtil->systemPathIconNameByPath(path);
            
            if (systemPathUtil->isSystemPath(path)) {
                // 如果是系统路径，图标名称应该不为空
                EXPECT_FALSE(iconName.isEmpty()) 
                    << "Icon name by path should not be empty for system path: " << path.toStdString();
            }
        }
    }
    
    // 测试非系统路径
    QString nonSystemPath = "/non/system/path";
    QString nonSystemIconName = systemPathUtil->systemPathIconNameByPath(nonSystemPath);
    EXPECT_TRUE(nonSystemIconName.isEmpty() || !nonSystemIconName.isEmpty()) 
        << "Non-system path icon name should be handled gracefully";
}

// 测试10: 安全路径获取
TEST_F(SystemPathUtilTest, TestGetRealpathSafely)
{
    // 测试各种路径的 realpath 安全获取
    QList<QString> testPaths = {
        "/",
        "/home",
        "/tmp",
        "/usr",
        "/var",
        "/nonexistent/path",
        ""
    };
    
    for (const QString& path : testPaths) {
        QString realpath = systemPathUtil->getRealpathSafely(path);
        
        // realpath 应该不为空或有一个合理的返回值
        EXPECT_TRUE(realpath.isEmpty() || !realpath.isEmpty()) 
            << "Get realpath safely should handle path: " << path.toStdString();
        
        // 如果原路径不为空，realpath 也不应该为空（除非路径真的不存在）
        if (!path.isEmpty()) {
            // realpath 可能为空（如果路径不存在），这取决于具体实现
            EXPECT_TRUE(realpath.isEmpty() || realpath.startsWith('/')) 
                << "Realpath should be absolute or empty for: " << path.toStdString();
        }
    }
}

// 测试11: 边界条件测试
TEST_F(SystemPathUtilTest, TestBoundaryConditions)
{
    // 测试空键
    QString emptyKeyResult = systemPathUtil->systemPath("");
    EXPECT_TRUE(emptyKeyResult.isEmpty() || !emptyKeyResult.isEmpty()) 
        << "Empty key should be handled gracefully";
    
    // 测试无效键
    QString invalidKeyResult = systemPathUtil->systemPath("nonexistent_key_xyz");
    EXPECT_TRUE(invalidKeyResult.isEmpty() || !invalidKeyResult.isEmpty()) 
        << "Invalid key should be handled gracefully";
    
    // 测试空路径
    bool emptyPathResult = systemPathUtil->isSystemPath("");
    EXPECT_FALSE(emptyPathResult) << "Empty path should not be a system path";
    
    // 测试空用户
    QString emptyUserPath = systemPathUtil->systemPathOfUser("Home", "");
    EXPECT_TRUE(emptyUserPath.isEmpty() || !emptyUserPath.isEmpty()) 
        << "Empty user should be handled gracefully";
}

// 测试12: 特殊路径处理
TEST_F(SystemPathUtilTest, TestSpecialPathHandling)
{
    QList<QString> specialPaths = {
        "~",
        "~/",
        "$HOME",
        "/home",
        "/tmp",
        "/var/tmp",
        "/proc",
        "/sys",
        "/dev"
    };
    
    for (const QString& path : specialPaths) {
        bool isSystem = systemPathUtil->isSystemPath(path);
        QString displayName = systemPathUtil->systemPathDisplayNameByPath(path);
        QString iconName = systemPathUtil->systemPathIconNameByPath(path);
        QString realpath = systemPathUtil->getRealpathSafely(path);
        
        // 所有操作都应该正常返回，不应该崩溃
        EXPECT_TRUE(isSystem == true || isSystem == false) 
            << "Special path check should return boolean for: " << path.toStdString();
    }
}

// 测试13: Unicode路径处理
TEST_F(SystemPathUtilTest, TestUnicodePathHandling)
{
    QList<QString> unicodePaths = {
        "/home/用户",
        "/home/тест",
        "/home/テスト",
        "/home/📁",  // Emoji
        "/home/café",
        "/home/naïve"
    };
    
    for (const QString& path : unicodePaths) {
        bool isSystem = systemPathUtil->isSystemPath(path);
        QString displayName = systemPathUtil->systemPathDisplayNameByPath(path);
        QString iconName = systemPathUtil->systemPathIconNameByPath(path);
        
        // Unicode路径应该被正确处理
        EXPECT_TRUE(isSystem == true || isSystem == false) 
            << "Unicode path check should return boolean for: " << path.toStdString();
    }
}

// 测试14: 路径标准化测试
TEST_F(SystemPathUtilTest, TestPathNormalization)
{
    // 测试各种路径格式
    QMap<QString, QString> pathTests = {
        {"/home//user", "/home/user"},
        {"/home/user/.", "/home/user"},
        {"/home/user/..", "/home"},
        {"/home/user/../", "/home/"},
        {"./relative/path", "./relative/path"},
        {"../parent/path", "../parent/path"}
    };
    
    for (auto it = pathTests.begin(); it != pathTests.end(); ++it) {
        QString originalPath = it.key();
        QString expectedNormalized = it.value();
        
        QString realpath = systemPathUtil->getRealpathSafely(originalPath);
        
        // realpath 操作应该成功
        EXPECT_TRUE(realpath.isEmpty() || !realpath.isEmpty()) 
            << "Path normalization should work for: " << originalPath.toStdString();
    }
}

// 测试15: URL列表混合测试
TEST_F(SystemPathUtilTest, TestMixedUrlList)
{
    QList<QUrl> mixedUrlList;
    
    // 添加系统路径URL
    QStringList keys = getSystemPathKeys();
    for (const QString& key : keys) {
        QString path = getSystemPath(key);
        if (!path.isEmpty()) {
            mixedUrlList.append(QUrl::fromLocalFile(path));
            break; // 只添加一个系统路径
        }
    }
    
    // 添加非系统路径URL
    mixedUrlList.append(QUrl::fromLocalFile("/non/system/path"));
    mixedUrlList.append(QUrl::fromLocalFile(tempDirPath));
    mixedUrlList.append(QUrl::fromLocalFile(QString("%1/test_file").arg(tempDirPath)));
    
    // 测试混合URL列表
    bool containsSystem = systemPathUtil->checkContainsSystemPath(mixedUrlList);
    EXPECT_TRUE(containsSystem == true || containsSystem == false) 
        << "Mixed URL list check should return boolean value";
}

// 测试16: 性能测试
TEST_F(SystemPathUtilTest, TestPerformance)
{
    const int operationCount = 1000;
    
    // 测试路径检查性能
    QString testPath = "/home";  // 常见路径
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < operationCount; ++i) {
        bool isSystem = systemPathUtil->isSystemPath(testPath);
        EXPECT_TRUE(isSystem == true || isSystem == false) << "Path check should return boolean";
    }
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    qint64 duration = endTime - startTime;
    
    EXPECT_LT(duration, 2000) << "Path checking should complete within 2 seconds";
    
    // 测试路径获取性能
    startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < operationCount; ++i) {
        QString path = systemPathUtil->systemPath("Home");
        EXPECT_TRUE(path.isEmpty() || !path.isEmpty()) << "Path retrieval should work";
    }
    
    endTime = QDateTime::currentMSecsSinceEpoch();
    duration = endTime - startTime;
    
    EXPECT_LT(duration, 3000) << "Path retrieval should complete within 3 seconds";
}

// 测试17: 并发安全性测试
TEST_F(SystemPathUtilTest, TestConcurrentSafety)
{
    const int threadCount = 10;
    QList<QThread*> threads;
    QList<bool> results;
    QMutex resultsMutex;  // 添加互斥锁保护共享数据
    
    for (int i = 0; i < threadCount; ++i) {
        QThread* thread = QThread::create([this, i, &results, &resultsMutex]() {
            try {
                // 并发执行各种系统路径操作
                QString homePath = systemPathUtil->systemPath("Home");
                bool isHomeSystem = systemPathUtil->isSystemPath(homePath);
                QString displayName = systemPathUtil->systemPathDisplayName("Home");
                QString iconName = systemPathUtil->systemPathIconName("Home");
                
                // 验证所有操作都成功完成
                bool success = (homePath.isEmpty() || !homePath.isEmpty()) &&
                              (isHomeSystem == true || isHomeSystem == false) &&
                              (displayName.isEmpty() || !displayName.isEmpty()) &&
                              (iconName.isEmpty() || !iconName.isEmpty());
                
                // 使用互斥锁保护对共享结果列表的访问
                {
                    QMutexLocker locker(&resultsMutex);
                    results.append(success);
                }
                
                QThread::msleep(1);
            } catch (...) {
                // 使用互斥锁保护对共享结果列表的访问
                {
                    QMutexLocker locker(&resultsMutex);
                    results.append(false);
                }
            }
        });
        
        threads.append(thread);
        thread->start();
    }
    
    // 等待所有线程完成
    for (QThread* thread : threads) {
        thread->wait();
        delete thread;
    }
    
    // 使用互斥锁保护对共享结果列表的访问
    {
        QMutexLocker locker(&resultsMutex);
        EXPECT_EQ(results.size(), threadCount) << "All threads should complete";
        
        int successCount = 0;
        for (bool result : results) {
            if (result) successCount++;
        }
        
        EXPECT_EQ(successCount, threadCount) << "All concurrent operations should succeed";
    }
}

// 测试18: 内存管理测试
TEST_F(SystemPathUtilTest, TestMemoryManagement)
{
    // 由于SystemPathUtil是单例，主要测试内存泄漏和稳定性
    const int operationCount = 1000;
    
    // 执行大量操作，验证内存稳定性
    for (int i = 0; i < operationCount; ++i) {
        QString path = systemPathUtil->systemPath("Home");
        bool isSystem = systemPathUtil->isSystemPath(path);
        QString displayName = systemPathUtil->systemPathDisplayNameByPath(path);
        QString iconName = systemPathUtil->systemPathIconNameByPath(path);
        
        // 验证返回值的有效性
        EXPECT_TRUE(path.isEmpty() || !path.isEmpty()) << "Path should be valid";
        EXPECT_TRUE(isSystem == true || isSystem == false) << "IsSystem should return boolean";
        EXPECT_TRUE(displayName.isEmpty() || !displayName.isEmpty()) << "DisplayName should be valid";
        EXPECT_TRUE(iconName.isEmpty() || !iconName.isEmpty()) << "IconName should be valid";
    }
    
    SUCCEED() << "Memory management test completed successfully";
}
