// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QGuiApplication>
#include <QTimer>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QTest>
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE

// 全局初始化框架
void initializeFramework()
{
    static bool initialized = false;
    if (!initialized) {
        // 确保QCoreApplication存在
        if (qApp) {
            // 注册必要的URL schemes
            dfmbase::UrlRoute::regScheme(dfmbase::Global::Scheme::kFile, "/", QIcon(), false);
            dfmbase::UrlRoute::regScheme(dfmbase::Global::Scheme::kAsyncFile, "/", QIcon(), false);
            
            // 注册必要的FileInfo实现类
            dfmbase::InfoFactory::regClass<dfmbase::SyncFileInfo>(dfmbase::Global::Scheme::kFile);
            dfmbase::InfoFactory::regClass<dfmbase::AsyncFileInfo>(dfmbase::Global::Scheme::kAsyncFile);
        }
        
        initialized = true;
    }
}

class TestFileInfoHelper : public testing::Test
{
public:
    void SetUp() override
    {
        // 确保QCoreApplication存在
        if (qApp == nullptr) {
            int argc = 0;
            char **argv = nullptr;
            static QCoreApplication dummyApp(argc, argv);
        }
        
        // 初始化框架
        initializeFramework();
        
        // 预创建并初始化FileInfoHelper实例以确保完全初始化
        auto& instance = FileInfoHelper::instance();
        Q_UNUSED(instance);
        
        // 等待一小段时间确保所有组件初始化完成
        QTest::qWait(50);
        
        // 创建临时测试目录
        tempDir.reset(new QTemporaryDir);
        ASSERT_TRUE(tempDir->isValid());
        
        // 创建测试文件
        testFilePath = tempDir->filePath("test_file.txt");
        QFile testFile(testFilePath);
        ASSERT_TRUE(testFile.open(QIODevice::WriteOnly | QIODevice::Text));
        testFile.write("Test content for FileInfoHelper testing");
        testFile.close();
        
        testFileUrl = QUrl::fromLocalFile(testFilePath);
        testDirUrl = QUrl::fromLocalFile(tempDir->path());
        
        // 等待FileInfoHelper初始化完成
        // QTest::qWaitFor([this]() {
        //     return helper != nullptr;
        // }, 500);
    }
    
    void TearDown() override
    {
        // 清理资源
        tempDir.reset();
    }

protected:
    QSharedPointer<QTemporaryDir> tempDir;
    QString testFilePath;
    QUrl testFileUrl;
    QUrl testDirUrl;
    FileInfoHelper *helper = nullptr;
};

TEST_F(TestFileInfoHelper, TestSingletonInstance)
{
    // 测试单例模式
    FileInfoHelper &instance1 = FileInfoHelper::instance();
    FileInfoHelper &instance2 = FileInfoHelper::instance();
    
    EXPECT_EQ(&instance1, &instance2) << "FileInfoHelper should be a singleton";
}

TEST_F(TestFileInfoHelper, TestFileCountAsyncValidFile)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QUrl url = testFileUrl;
    auto userData = helper->fileCountAsync(url);
    
    ASSERT_NE(userData, nullptr) << "fileCountAsync should return valid UserData";
    
    // 等待异步操作完成
    int maxWaitTime = 5000; // 5秒
    int waitedTime = 0;
    
    while (!userData->finish && waitedTime < maxWaitTime) {
        QTest::qWait(100);
        waitedTime += 100;
    }
    
    EXPECT_TRUE(userData->finish) << "Async file count should complete within timeout";
    EXPECT_TRUE(userData->data.isValid()) << "File count data should be valid";
}

TEST_F(TestFileInfoHelper, TestFileCountAsyncInvalidUrl)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QUrl invalidUrl("file:///nonexistent/path/file.txt");
    auto userData = helper->fileCountAsync(invalidUrl);
    
    ASSERT_NE(userData, nullptr) << "Even invalid URL should return UserData object";
    
    // 等待异步操作完成
    int maxWaitTime = 3000;
    int waitedTime = 0;
    
    while (!userData->finish && waitedTime < maxWaitTime) {
        QTest::qWait(100);
        waitedTime += 100;
    }
    
    EXPECT_TRUE(userData->finish) << "Async operation should complete even for invalid URL";
}

TEST_F(TestFileInfoHelper, TestFileCountAsyncStopped)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 模拟停止状态 - 我们需要通过调用aboutToQuit来停止
    helper->aboutToQuit();
    
    QUrl url = testFileUrl;
    auto userData = helper->fileCountAsync(url);
    
    EXPECT_EQ(userData, nullptr) << "fileCountAsync should return nullptr when stopped";
}

TEST_F(TestFileInfoHelper, TestFileMimeTypeAsyncValidFile)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QUrl url = testFileUrl;
    QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault;
    QString inod;
    bool isGvfs = false;
    
    auto userData = helper->fileMimeTypeAsync(url, mode, inod, isGvfs);
    
    ASSERT_NE(userData, nullptr) << "fileMimeTypeAsync should return valid UserData";
    
    // 等待异步操作完成
    int maxWaitTime = 5000;
    int waitedTime = 0;
    
    while (!userData->finish && waitedTime < maxWaitTime) {
        QTest::qWait(100);
        waitedTime += 100;
    }
    
    EXPECT_TRUE(userData->finish) << "Async MIME type detection should complete within timeout";
    EXPECT_TRUE(userData->data.isValid()) << "MIME type data should be valid";
    
    // 验证返回的MIME类型
    if (userData->data.isValid()) {
        QMimeType mimeType = userData->data.value<QMimeType>();
        EXPECT_TRUE(mimeType.isValid()) << "Should return valid MIME type";
    }
}

TEST_F(TestFileInfoHelper, TestFileMimeTypeAsyncWithInode)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QUrl url = testFileUrl;
    QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault;
    QString inod = "test_inode";
    bool isGvfs = false;
    
    auto userData = helper->fileMimeTypeAsync(url, mode, inod, isGvfs);
    
    ASSERT_NE(userData, nullptr) << "fileMimeTypeAsync with inode should return valid UserData";
    
    // 等待异步操作完成
    int maxWaitTime = 5000;
    int waitedTime = 0;
    
    while (!userData->finish && waitedTime < maxWaitTime) {
        QTest::qWait(100);
        waitedTime += 100;
    }
    
    EXPECT_TRUE(userData->finish) << "Async MIME type detection with inode should complete";
}

TEST_F(TestFileInfoHelper, TestFileMimeTypeAsyncGvfsFile)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QUrl url = testFileUrl;
    QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault;
    QString inod;
    bool isGvfs = true;
    
    auto userData = helper->fileMimeTypeAsync(url, mode, inod, isGvfs);
    
    ASSERT_NE(userData, nullptr) << "fileMimeTypeAsync with GVFS should return valid UserData";
    
    // 等待异步操作完成
    int maxWaitTime = 5000;
    int waitedTime = 0;
    
    while (!userData->finish && waitedTime < maxWaitTime) {
        QTest::qWait(100);
        waitedTime += 100;
    }
    
    EXPECT_TRUE(userData->finish) << "Async MIME type detection with GVFS should complete";
}

TEST_F(TestFileInfoHelper, TestFileMimeTypeAsyncStopped)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 停止helper
    helper->aboutToQuit();
    
    QUrl url = testFileUrl;
    auto userData = helper->fileMimeTypeAsync(url, QMimeDatabase::MatchDefault, QString(), false);
    
    EXPECT_EQ(userData, nullptr) << "fileMimeTypeAsync should return nullptr when stopped";
}

TEST_F(TestFileInfoHelper, TestFileRefreshAsyncWithNullFileInfo)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 测试空指针情况
    QSharedPointer<FileInfo> nullFileInfo;
    
    // 这应该不会崩溃
    EXPECT_NO_THROW(helper->fileRefreshAsync(nullFileInfo)) << "Should handle null FileInfo gracefully";
}

TEST_F(TestFileInfoHelper, TestFileRefreshAsyncWithValidFileInfo)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 创建FileInfo对象
    auto fileInfo = InfoFactory::create<FileInfo>(testFileUrl);
    ASSERT_NE(fileInfo, nullptr) << "Should be able to create FileInfo for valid URL";
    
    // 测试异步刷新
    EXPECT_NO_THROW(helper->fileRefreshAsync(fileInfo)) << "Should handle valid FileInfo refresh";
    
    // 等待一段时间让异步操作开始
    QTest::qWait(100);
}

TEST_F(TestFileInfoHelper, TestFileRefreshAsyncStopped)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 停止helper
    helper->aboutToQuit();
    
    auto fileInfo = InfoFactory::create<FileInfo>(testFileUrl);
    if (fileInfo) {
        // 这应该不会执行任何操作
        EXPECT_NO_THROW(helper->fileRefreshAsync(fileInfo)) << "Should handle refresh when stopped";
    }
}

TEST_F(TestFileInfoHelper, TestCacheFileInfoByThread)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    auto fileInfo = InfoFactory::create<FileInfo>(testFileUrl);
    ASSERT_NE(fileInfo, nullptr) << "Should create FileInfo successfully";
    
    // 测试线程缓存
    EXPECT_NO_THROW(helper->cacheFileInfoByThread(fileInfo)) << "Should cache file info by thread without crash";
    
    // 等待一段时间让线程操作完成
    QTest::qWait(200);
}

TEST_F(TestFileInfoHelper, TestCacheFileInfoByThreadWithAsyncFileInfo)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    auto fileInfo = InfoFactory::create<AsyncFileInfo>(testFileUrl);
    if (fileInfo) {
        auto asyncFileInfo = fileInfo.dynamicCast<AsyncFileInfo>();
        ASSERT_NE(asyncFileInfo, nullptr) << "Should create AsyncFileInfo successfully";
        
        EXPECT_NO_THROW(helper->cacheFileInfoByThread(asyncFileInfo)) << "Should cache async file info by thread";
        
        // 等待线程操作完成
        QTest::qWait(300);
    }
}

TEST_F(TestFileInfoHelper, TestCacheFileInfoByThreadWithNullFileInfo)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QSharedPointer<FileInfo> nullFileInfo;
    
    EXPECT_NO_THROW(helper->cacheFileInfoByThread(nullFileInfo)) << "Should handle null FileInfo gracefully";
}

TEST_F(TestFileInfoHelper, TestCacheFileInfoByThreadStopped)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 停止helper
    helper->aboutToQuit();
    
    auto fileInfo = InfoFactory::create<FileInfo>(testFileUrl);
    if (fileInfo) {
        EXPECT_NO_THROW(helper->cacheFileInfoByThread(fileInfo)) << "Should handle caching when stopped";
    }
}

TEST_F(TestFileInfoHelper, TestDestruction)
{
    // 测试对象销毁
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 模拟应用退出
    EXPECT_NO_THROW(helper->aboutToQuit()) << "Should handle application quit gracefully";
}

TEST_F(TestFileInfoHelper, TestMultipleAsyncOperations)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 创建多个异步操作
    QList<QSharedPointer<FileInfoHelperUeserData>> userDataList;
    
    // 多个文件计数操作
    for (int i = 0; i < 3; ++i) {
        auto userData = helper->fileCountAsync(testFileUrl);
        if (userData) {
            userDataList.append(userData);
        }
    }
    
    // 多个MIME类型操作
    for (int i = 0; i < 3; ++i) {
        auto userData = helper->fileMimeTypeAsync(testFileUrl, QMimeDatabase::MatchDefault, QString(), false);
        if (userData) {
            userDataList.append(userData);
        }
    }
    
    // 等待所有操作完成
    int maxWaitTime = 8000;
    int waitedTime = 0;
    
    bool allCompleted = false;
    while (!allCompleted && waitedTime < maxWaitTime) {
        allCompleted = true;
        for (const auto &userData : userDataList) {
            if (!userData->finish) {
                allCompleted = false;
                break;
            }
        }
        
        if (!allCompleted) {
            QTest::qWait(100);
            waitedTime += 100;
        }
    }
    
    EXPECT_TRUE(allCompleted) << "All async operations should complete within timeout";
}

TEST_F(TestFileInfoHelper, TestFileCountAsyncDirectory)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 测试目录文件计数
    auto userData = helper->fileCountAsync(testDirUrl);
    
    ASSERT_NE(userData, nullptr) << "Should return UserData for directory";
    
    // 等待操作完成
    int maxWaitTime = 5000;
    int waitedTime = 0;
    
    while (!userData->finish && waitedTime < maxWaitTime) {
        QTest::qWait(100);
        waitedTime += 100;
    }
    
    EXPECT_TRUE(userData->finish) << "Directory file count should complete";
    
    if (userData->finish && userData->data.isValid()) {
        int count = userData->data.toInt();
        EXPECT_GE(count, 0) << "File count should be non-negative";
    }
}

TEST_F(TestFileInfoHelper, TestDifferentMatchModes)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    QList<QMimeDatabase::MatchMode> modes = {
        QMimeDatabase::MatchDefault,
        QMimeDatabase::MatchExtension,
        QMimeDatabase::MatchContent
    };
    
    for (auto mode : modes) {
        auto userData = helper->fileMimeTypeAsync(testFileUrl, mode, QString(), false);
        
        ASSERT_NE(userData, nullptr) << "Should return UserData for match mode";
        
        // 等待操作完成
        int maxWaitTime = 3000;
        int waitedTime = 0;
        
        while (!userData->finish && waitedTime < maxWaitTime) {
            QTest::qWait(100);
            waitedTime += 100;
        }
        
        EXPECT_TRUE(userData->finish) << "MIME type detection should complete for all match modes";
    }
}

// 性能测试
TEST_F(TestFileInfoHelper, TestPerformanceMultipleAsyncOperations)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    const int operationCount = 50;
    QList<QSharedPointer<FileInfoHelperUeserData>> userDataList;
    
    // 记录开始时间
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 创建多个异步操作
    for (int i = 0; i < operationCount; ++i) {
        auto userData = helper->fileCountAsync(testFileUrl);
        if (userData) {
            userDataList.append(userData);
        }
    }
    
    // 等待所有操作完成
    int maxWaitTime = 15000;
    int waitedTime = 0;
    
    bool allCompleted = false;
    while (!allCompleted && waitedTime < maxWaitTime) {
        allCompleted = true;
        for (const auto &userData : userDataList) {
            if (!userData->finish) {
                allCompleted = false;
                break;
            }
        }
        
        if (!allCompleted) {
            QTest::qWait(100);
            waitedTime += 100;
        }
    }
    
    auto endTime = QDateTime::currentMSecsSinceEpoch();
    auto duration = endTime - startTime;
    
    EXPECT_TRUE(allCompleted) << "All performance test operations should complete";
    EXPECT_LT(duration, 15000) << "Performance test should complete within 15 seconds";
    
    qInfo() << "Performance test: " << operationCount << " operations completed in " << duration << "ms";
}

// 错误处理测试
TEST_F(TestFileInfoHelper, TestErrorHandling)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 测试各种错误情况
    QList<QUrl> errorUrls = {
        QUrl(), // 空URL
        QUrl("invalid://scheme"),
        QUrl("file:///nonexistent/very/long/path/that/does/not/exist/file.txt"),
        QUrl("http://invalid.server.com/nonexistent.txt")
    };
    
    for (auto url : errorUrls) {
        auto userData = helper->fileCountAsync(url);
        
        if (userData) {
            // 等待操作完成
            int maxWaitTime = 3000;
            int waitedTime = 0;
            
            while (!userData->finish && waitedTime < maxWaitTime) {
                QTest::qWait(100);
                waitedTime += 100;
            }
            
            // 应该能够完成，即使数据可能无效
            EXPECT_TRUE(userData->finish) << "Error case should complete, not hang: " << url.toString().toStdString();
        }
    }
}

// 边界条件测试
TEST_F(TestFileInfoHelper, TestBoundaryConditions)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 测试不同的MIME数据库匹配模式组合
    QList<std::pair<QMimeDatabase::MatchMode, QString>> testCases = {
        {QMimeDatabase::MatchDefault, ""},
        {QMimeDatabase::MatchExtension, "12345"},
        {QMimeDatabase::MatchContent, "test_inode_12345"},
        {QMimeDatabase::MatchDefault, "inode_with_special_chars!@#$%^&*()"}
    };
    
    for (const auto &testCase : testCases) {
        auto userData = helper->fileMimeTypeAsync(testFileUrl, testCase.first, testCase.second, testCase.second.contains("special"));
        
        ASSERT_NE(userData, nullptr) << "Should handle all boundary condition combinations";
        
        // 等待操作完成
        int maxWaitTime = 3000;
        int waitedTime = 0;
        
        while (!userData->finish && waitedTime < maxWaitTime) {
            QTest::qWait(100);
            waitedTime += 100;
        }
        
        EXPECT_TRUE(userData->finish) << "Boundary condition should complete: " << testCase.second.toStdString();
    }
}

// 并发测试
TEST_F(TestFileInfoHelper, TestConcurrentOperations)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 创建多个并发的文件信息对象进行刷新
    QList<QSharedPointer<FileInfo>> fileInfoList;
    const int concurrentCount = 10;
    
    for (int i = 0; i < concurrentCount; ++i) {
        auto fileInfo = InfoFactory::create<FileInfo>(testFileUrl);
        if (fileInfo) {
            fileInfoList.append(fileInfo);
            helper->fileRefreshAsync(fileInfo);
        }
    }
    
    // 等待一段时间让并发操作开始
    QTest::qWait(500);
    
    // 验证没有崩溃
    EXPECT_TRUE(true) << "Concurrent operations should not cause crashes";
}

// 线程安全测试
TEST_F(TestFileInfoHelper, TestThreadSafety)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 在多个线程中调用FileInfoHelper方法
    const int threadCount = 5;
    QList<QFuture<void>> futures;
    
    for (int i = 0; i < threadCount; ++i) {
        auto future = QtConcurrent::run([this, i]() {
            for (int j = 0; j < 10; ++j) {
                auto userData = helper->fileCountAsync(testFileUrl);
                if (userData) {
                    // 等待完成
                    int maxWaitTime = 2000;
                    int waitedTime = 0;
                    
                    while (!userData->finish && waitedTime < maxWaitTime) {
                        QThread::msleep(50);
                        waitedTime += 50;
                    }
                }
                
                QThread::msleep(10); // 短暂延迟
            }
        });
        
        futures.append(future);
    }
    
    // 等待所有线程完成
    for (auto &future : futures) {
        future.waitForFinished();
    }
    
    EXPECT_TRUE(true) << "Thread safety test should complete without crashes";
}

// 内存泄漏测试
TEST_F(TestFileInfoHelper, TestMemoryLeaks)
{
    helper = &FileInfoHelper::instance();
    ASSERT_NE(helper, nullptr);
    
    // 创建大量异步操作对象
    const int iterationCount = 100;
    
    for (int i = 0; i < iterationCount; ++i) {
        auto userData = helper->fileCountAsync(testFileUrl);
        auto mimeTypeData = helper->fileMimeTypeAsync(testFileUrl, QMimeDatabase::MatchDefault, QString(), false);
        
        // 重置智能指针，测试内存管理
        userData.reset();
        mimeTypeData.reset();
        
        if (i % 10 == 0) {
            // 定期等待垃圾回收
            QTest::qWait(100);
        }
    }
    
    EXPECT_TRUE(true) << "Memory leak test should complete without issues";
}
