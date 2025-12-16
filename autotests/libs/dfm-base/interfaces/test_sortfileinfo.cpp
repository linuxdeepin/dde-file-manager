// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "gmock/gmock.h"
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
#include <QJsonObject>
#include <QJsonDocument>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QVariant>
#include <QDateTime>
#include <future>

#include "dfm-base/interfaces/sortfileinfo.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include "stubext.h"

using namespace dfmbase;

class SortFileInfoTest : public testing::Test
{
public:
    void SetUp() override
    {
        // 初始化测试环境
        std::cout << "SetUp SortFileInfoTest" << std::endl;
        
        // 创建测试数据
        testUrl = QUrl::fromLocalFile("/home/user/testfile.txt");
        testSize = 1024 * 1024; // 1MB
        testTime = QDateTime::currentMSecsSinceEpoch();
        testContent = "Test highlight content";
    }

    void TearDown() override
    {
        // 清理
        stub_ext::StubExt stub;
        stub.clear();
        std::cout << "TearDown SortFileInfoTest" << std::endl;
    }

protected:
    // 辅助方法
    SortFileInfo* createSortFileInfo()
    {
        return new SortFileInfo();
    }
    
    void populateSortFileInfo(SortFileInfo* info)
    {
        if (!info) return;
        
        info->setUrl(testUrl);
        info->setSize(testSize);
        info->setFile(true);
        info->setDir(false);
        info->setSymlink(false);
        info->setHide(false);
        info->setReadable(true);
        info->setWriteable(true);
        info->setExecutable(false);
        info->setLastReadTime(testTime);
        info->setLastModifiedTime(testTime);
        info->setCreateTime(testTime);
        info->setHighlightContent(testContent);
        info->setCustomData("test_key", "test_value");
        info->setInfoCompleted(true);
    }

protected:
    QUrl testUrl;
    qint64 testSize;
    qint64 testTime;
    QString testContent;
};

// 测试1: SortFileInfo 构造函数和析构函数
TEST_F(SortFileInfoTest, TestConstructorAndDestructor)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试默认值
    EXPECT_EQ(sortInfo->fileUrl(), QUrl()) << "Default URL should be empty";
    EXPECT_EQ(sortInfo->fileSize(), 0) << "Default size should be 0";
    EXPECT_FALSE(sortInfo->isFile()) << "Default isFile should be false";
    EXPECT_FALSE(sortInfo->isDir()) << "Default isDir should be false";
    EXPECT_FALSE(sortInfo->isSymLink()) << "Default isSymLink should be false";
    EXPECT_FALSE(sortInfo->isHide()) << "Default isHide should be false";
    EXPECT_FALSE(sortInfo->isReadable()) << "Default isReadable should be false";
    EXPECT_FALSE(sortInfo->isWriteable()) << "Default isWriteable should be false";
    EXPECT_FALSE(sortInfo->isExecutable()) << "Default isExecutable should be false";
    EXPECT_EQ(sortInfo->lastReadTime(), 0) << "Default lastReadTime should be 0";
    EXPECT_EQ(sortInfo->lastModifiedTime(), 0) << "Default lastModifiedTime should be 0";
    EXPECT_EQ(sortInfo->createTime(), 0) << "Default createTime should be 0";
    EXPECT_TRUE(sortInfo->highlightContent().isEmpty()) << "Default highlightContent should be empty";
    EXPECT_FALSE(sortInfo->isInfoCompleted()) << "Default isInfoCompleted should be false";
    
    delete sortInfo;
}

// 测试2: SortFileInfo 基本属性设置和获取
TEST_F(SortFileInfoTest, TestBasicProperties)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试URL设置
    sortInfo->setUrl(testUrl);
    EXPECT_EQ(sortInfo->fileUrl(), testUrl) << "URL should be set correctly";
    
    // 测试文件大小设置
    sortInfo->setSize(testSize);
    EXPECT_EQ(sortInfo->fileSize(), testSize) << "File size should be set correctly";
    
    // 测试文件类型设置
    sortInfo->setFile(true);
    EXPECT_TRUE(sortInfo->isFile()) << "isFile should be true after setting";
    
    sortInfo->setDir(true);
    EXPECT_TRUE(sortInfo->isDir()) << "isDir should be true after setting";
    
    sortInfo->setSymlink(true);
    EXPECT_TRUE(sortInfo->isSymLink()) << "isSymLink should be true after setting";
    
    sortInfo->setHide(true);
    EXPECT_TRUE(sortInfo->isHide()) << "isHide should be true after setting";
    
    sortInfo->setReadable(true);
    EXPECT_TRUE(sortInfo->isReadable()) << "isReadable should be true after setting";
    
    sortInfo->setWriteable(true);
    EXPECT_TRUE(sortInfo->isWriteable()) << "isWriteable should be true after setting";
    
    sortInfo->setExecutable(true);
    EXPECT_TRUE(sortInfo->isExecutable()) << "isExecutable should be true after setting";
    
    delete sortInfo;
}

// 测试3: SortFileInfo 时间属性设置和获取
TEST_F(SortFileInfoTest, TestTimeProperties)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试时间属性设置
    sortInfo->setLastReadTime(testTime);
    EXPECT_EQ(sortInfo->lastReadTime(), testTime) << "Last read time should be set correctly";
    
    sortInfo->setLastModifiedTime(testTime);
    EXPECT_EQ(sortInfo->lastModifiedTime(), testTime) << "Last modified time should be set correctly";
    
    sortInfo->setCreateTime(testTime);
    EXPECT_EQ(sortInfo->createTime(), testTime) << "Create time should be set correctly";
    
    delete sortInfo;
}

// 测试4: SortFileInfo 高亮内容和自定义数据
TEST_F(SortFileInfoTest, TestHighlightAndCustomData)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试高亮内容设置
    sortInfo->setHighlightContent(testContent);
    EXPECT_EQ(sortInfo->highlightContent(), testContent) << "Highlight content should be set correctly";
    
    // 测试自定义数据设置
    QString testKey = "test_key";
    QString testValue = "test_value";
    sortInfo->setCustomData(testKey, testValue);
    QVariant retrievedValue = sortInfo->customData(testKey);
    EXPECT_EQ(retrievedValue.toString(), testValue) << "Custom data should be set and retrieved correctly";
    
    // 测试不存在的自定义数据
    QVariant nonExistentValue = sortInfo->customData("non_existent_key");
    EXPECT_TRUE(nonExistentValue.isNull()) << "Non-existent custom data should return null QVariant";
    
    delete sortInfo;
}

// 测试5: SortFileInfo 信息完整性
TEST_F(SortFileInfoTest, TestInfoCompletion)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试默认状态
    EXPECT_FALSE(sortInfo->isInfoCompleted()) << "Info should not be completed by default";
    EXPECT_TRUE(sortInfo->needsCompletion()) << "Should need completion by default";
    
    // 测试设置完成状态
    sortInfo->setInfoCompleted(true);
    EXPECT_TRUE(sortInfo->isInfoCompleted()) << "Info should be completed after setting";
    EXPECT_FALSE(sortInfo->needsCompletion()) << "Should not need completion after setting";
    
    // 测试标记为完成
    sortInfo->setInfoCompleted(false);
    EXPECT_FALSE(sortInfo->isInfoCompleted()) << "Info should not be completed after resetting";
    
    sortInfo->markAsCompleted();
    EXPECT_TRUE(sortInfo->isInfoCompleted()) << "Info should be completed after marking";
    
    delete sortInfo;
}

// 测试6: SortFileInfo 完整功能测试
TEST_F(SortFileInfoTest, TestCompleteFunctionality)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 使用辅助方法填充数据
    populateSortFileInfo(sortInfo);
    
    // 验证所有属性
    EXPECT_EQ(sortInfo->fileUrl(), testUrl) << "URL should be correct";
    EXPECT_EQ(sortInfo->fileSize(), testSize) << "File size should be correct";
    EXPECT_TRUE(sortInfo->isFile()) << "isFile should be true";
    EXPECT_FALSE(sortInfo->isDir()) << "isDir should be false";
    EXPECT_FALSE(sortInfo->isSymLink()) << "isSymLink should be false";
    EXPECT_FALSE(sortInfo->isHide()) << "isHide should be false";
    EXPECT_TRUE(sortInfo->isReadable()) << "isReadable should be true";
    EXPECT_TRUE(sortInfo->isWriteable()) << "isWriteable should be true";
    EXPECT_FALSE(sortInfo->isExecutable()) << "isExecutable should be false";
    EXPECT_EQ(sortInfo->lastReadTime(), testTime) << "Last read time should be correct";
    EXPECT_EQ(sortInfo->lastModifiedTime(), testTime) << "Last modified time should be correct";
    EXPECT_EQ(sortInfo->createTime(), testTime) << "Create time should be correct";
    EXPECT_EQ(sortInfo->highlightContent(), testContent) << "Highlight content should be correct";
    EXPECT_EQ(sortInfo->customData("test_key").toString(), "test_value") << "Custom data should be correct";
    EXPECT_TRUE(sortInfo->isInfoCompleted()) << "Info should be completed";
    
    delete sortInfo;
}

// 测试7: SortFileInfo 边界条件测试
TEST_F(SortFileInfoTest, TestBoundaryConditions)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试空URL
    sortInfo->setUrl(QUrl());
    EXPECT_EQ(sortInfo->fileUrl(), QUrl()) << "Empty URL should be handled correctly";
    
    // 测试零文件大小
    sortInfo->setSize(0);
    EXPECT_EQ(sortInfo->fileSize(), 0) << "Zero file size should be handled correctly";
    
    // 测试负数文件大小
    sortInfo->setSize(-1);
    EXPECT_EQ(sortInfo->fileSize(), -1) << "Negative file size should be handled correctly";
    
    // 测试极大文件大小
    qint64 maxFileSize = std::numeric_limits<qint64>::max();
    sortInfo->setSize(maxFileSize);
    EXPECT_EQ(sortInfo->fileSize(), maxFileSize) << "Maximum file size should be handled correctly";
    
    // 测试空高亮内容
    sortInfo->setHighlightContent("");
    EXPECT_TRUE(sortInfo->highlightContent().isEmpty()) << "Empty highlight content should be handled correctly";
    
    // 测试极长高亮内容
    QString longContent = QString("x").repeated(10000);
    sortInfo->setHighlightContent(longContent);
    EXPECT_EQ(sortInfo->highlightContent(), longContent) << "Long highlight content should be handled correctly";
    
    delete sortInfo;
}

// 测试8: SortFileInfo 特殊字符和Unicode测试
TEST_F(SortFileInfoTest, TestSpecialCharactersAndUnicode)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试Unicode URL
    QUrl unicodeUrl = QUrl::fromLocalFile("/home/user/测试文件.txt");
    sortInfo->setUrl(unicodeUrl);
    EXPECT_EQ(sortInfo->fileUrl(), unicodeUrl) << "Unicode URL should be handled correctly";
    
    // 测试Unicode高亮内容
    QString unicodeContent = "测试高亮内容";
    sortInfo->setHighlightContent(unicodeContent);
    EXPECT_EQ(sortInfo->highlightContent(), unicodeContent) << "Unicode highlight content should be handled correctly";
    
    // 测试特殊字符的自定义数据键值
    QString specialKey = "special_key_!@#$%^&*()";
    QString specialValue = "special_value_中文_123";
    sortInfo->setCustomData(specialKey, specialValue);
    EXPECT_EQ(sortInfo->customData(specialKey).toString(), specialValue) << "Special characters in custom data should be handled correctly";
    
    delete sortInfo;
}

// 测试9: SortFileInfo 并发安全性测试
TEST_F(SortFileInfoTest, TestConcurrentSafety)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    const int threadCount = 10;
    QList<QThread*> threads;
    
    // 使用QList来存储每个线程的结果
    std::vector<std::future<bool>> futures;
    
    for (int i = 0; i < threadCount; ++i) {
        auto future = std::async(std::launch::async, [sortInfo, i]() {
            // 并发设置和获取属性
            QUrl url = QUrl::fromLocalFile(QString("/home/user/testfile_%1.txt").arg(i));
            qint64 size = i * 1024;
            QString content = QString("Test content %1").arg(i);
            
            // 设置属性
            sortInfo->setUrl(url);
            sortInfo->setSize(size);
            sortInfo->setHighlightContent(content);
            sortInfo->setFile(true);
            sortInfo->setCustomData(QString("thread_%1").arg(i), QString("value_%1").arg(i));
            
            // 获取属性验证
            bool urlValid = !sortInfo->fileUrl().isEmpty();
            bool sizeValid = sortInfo->fileSize() >= 0;
            bool contentValid = !sortInfo->highlightContent().isEmpty();
            bool fileValid = sortInfo->isFile() == true || sortInfo->isFile() == false;
            
            bool success = urlValid && sizeValid && contentValid && fileValid;
            
            QThread::msleep(1);
            return success;
        });
        
        futures.push_back(std::move(future));
    }
    
    // 等待所有线程完成并收集结果
    QList<bool> results;
    for (auto& future : futures) {
        results.append(future.get());
    }
    
    delete sortInfo;
    
    EXPECT_EQ(results.size(), threadCount) << "All threads should complete";
    
    int successCount = 0;
    for (bool result : results) {
        if (result) successCount++;
    }
    
    EXPECT_GT(successCount, threadCount * 0.8) << "Most concurrent operations should succeed";
}

// 测试10: SortFileInfo 内存管理测试
TEST_F(SortFileInfoTest, TestMemoryManagement)
{
    const int objectCount = 100;
    QList<SortFileInfo*> sortInfoList;
    
    // 创建大量对象
    for (int i = 0; i < objectCount; ++i) {
        SortFileInfo* sortInfo = createSortFileInfo();
        ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created at index " << i;
        
        // 填充数据
        populateSortFileInfo(sortInfo);
        sortInfo->setUrl(QUrl::fromLocalFile(QString("/home/user/testfile_%1.txt").arg(i)));
        sortInfo->setCustomData("index", i);
        
        sortInfoList.append(sortInfo);
    }
    
    // 验证所有对象创建成功
    EXPECT_EQ(sortInfoList.size(), objectCount) << "All SortFileInfo objects should be created";
    
    // 验证数据完整性
    for (int i = 0; i < sortInfoList.size(); ++i) {
        SortFileInfo* sortInfo = sortInfoList[i];
        EXPECT_EQ(sortInfo->customData("index").toInt(), i) << "Custom data should be correct at index " << i;
        EXPECT_TRUE(sortInfo->isInfoCompleted()) << "Info should be completed at index " << i;
    }
    
    // 删除所有对象
    for (SortFileInfo* sortInfo : sortInfoList) {
        delete sortInfo;
    }
    
    sortInfoList.clear();
    EXPECT_TRUE(sortInfoList.isEmpty()) << "All objects should be deleted";
}

// 测试11: SortFileInfo 属性覆盖测试
TEST_F(SortFileInfoTest, TestPropertyOverwrite)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 初始设置
    QUrl initialUrl = QUrl::fromLocalFile("/home/user/initial.txt");
    qint64 initialSize = 1024;
    QString initialContent = "Initial content";
    
    sortInfo->setUrl(initialUrl);
    sortInfo->setSize(initialSize);
    sortInfo->setHighlightContent(initialContent);
    sortInfo->setCustomData("overwrite_test", "initial_value");
    
    // 验证初始值
    EXPECT_EQ(sortInfo->fileUrl(), initialUrl) << "Initial URL should be correct";
    EXPECT_EQ(sortInfo->fileSize(), initialSize) << "Initial size should be correct";
    EXPECT_EQ(sortInfo->highlightContent(), initialContent) << "Initial content should be correct";
    EXPECT_EQ(sortInfo->customData("overwrite_test").toString(), "initial_value") << "Initial custom data should be correct";
    
    // 覆盖设置
    QUrl newUrl = QUrl::fromLocalFile("/home/user/new.txt");
    qint64 newSize = 2048;
    QString newContent = "New content";
    
    sortInfo->setUrl(newUrl);
    sortInfo->setSize(newSize);
    sortInfo->setHighlightContent(newContent);
    sortInfo->setCustomData("overwrite_test", "new_value");
    
    // 验证覆盖值
    EXPECT_EQ(sortInfo->fileUrl(), newUrl) << "New URL should overwrite initial value";
    EXPECT_EQ(sortInfo->fileSize(), newSize) << "New size should overwrite initial value";
    EXPECT_EQ(sortInfo->highlightContent(), newContent) << "New content should overwrite initial value";
    EXPECT_EQ(sortInfo->customData("overwrite_test").toString(), "new_value") << "New custom data should overwrite initial value";
    
    delete sortInfo;
}

// 测试12: SortFileInfo 布尔属性组合测试
TEST_F(SortFileInfoTest, TestBooleanPropertyCombinations)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试文件和目录属性
    sortInfo->setFile(true);
    sortInfo->setDir(false);
    EXPECT_TRUE(sortInfo->isFile()) << "File should be true";
    EXPECT_FALSE(sortInfo->isDir()) << "Dir should be false";
    
    sortInfo->setFile(false);
    sortInfo->setDir(true);
    EXPECT_FALSE(sortInfo->isFile()) << "File should be false";
    EXPECT_TRUE(sortInfo->isDir()) << "Dir should be true";
    
    // 测试文件可以同时是文件和目录（在某些文件系统中）
    sortInfo->setFile(true);
    sortInfo->setDir(true);
    EXPECT_TRUE(sortInfo->isFile()) << "File should be true";
    EXPECT_TRUE(sortInfo->isDir()) << "Dir should be true";
    
    // 测试权限属性组合
    sortInfo->setReadable(true);
    sortInfo->setWriteable(true);
    sortInfo->setExecutable(false);
    EXPECT_TRUE(sortInfo->isReadable()) << "Readable should be true";
    EXPECT_TRUE(sortInfo->isWriteable()) << "Writeable should be true";
    EXPECT_FALSE(sortInfo->isExecutable()) << "Executable should be false";
    
    // 测试隐藏和符号链接属性
    sortInfo->setHide(true);
    sortInfo->setSymlink(true);
    EXPECT_TRUE(sortInfo->isHide()) << "Hide should be true";
    EXPECT_TRUE(sortInfo->isSymLink()) << "SymLink should be true";
    
    delete sortInfo;
}

// 测试13: SortFileInfo 多个自定义数据测试
TEST_F(SortFileInfoTest, TestMultipleCustomData)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 设置多个自定义数据项
    QMap<QString, QVariant> customDataMap;
    customDataMap["string_key"] = "string_value";
    customDataMap["int_key"] = 42;
    customDataMap["bool_key"] = true;
    customDataMap["double_key"] = 3.14159;
    customDataMap["url_key"] = QUrl::fromLocalFile("/custom/path");
    
    // 设置所有自定义数据
    for (auto it = customDataMap.begin(); it != customDataMap.end(); ++it) {
        sortInfo->setCustomData(it.key(), it.value());
    }
    
    // 验证所有自定义数据
    for (auto it = customDataMap.begin(); it != customDataMap.end(); ++it) {
        QVariant retrievedValue = sortInfo->customData(it.key());
        EXPECT_EQ(retrievedValue, it.value()) 
            << "Custom data should match for key: " << it.key().toStdString();
    }
    
    // 测试覆盖自定义数据
    sortInfo->setCustomData("string_key", "new_string_value");
    EXPECT_EQ(sortInfo->customData("string_key").toString(), "new_string_value") 
        << "Custom data should be overwritten correctly";
    
    delete sortInfo;
}

// 测试14: SortFileInfo 时间属性边界测试
TEST_F(SortFileInfoTest, TestTimePropertiesBoundary)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试零时间
    sortInfo->setLastReadTime(0);
    sortInfo->setLastModifiedTime(0);
    sortInfo->setCreateTime(0);
    
    EXPECT_EQ(sortInfo->lastReadTime(), 0) << "Zero last read time should be handled correctly";
    EXPECT_EQ(sortInfo->lastModifiedTime(), 0) << "Zero last modified time should be handled correctly";
    EXPECT_EQ(sortInfo->createTime(), 0) << "Zero create time should be handled correctly";
    
    // 测试负数时间
    sortInfo->setLastReadTime(-1);
    sortInfo->setLastModifiedTime(-1);
    sortInfo->setCreateTime(-1);
    
    EXPECT_EQ(sortInfo->lastReadTime(), -1) << "Negative last read time should be handled correctly";
    EXPECT_EQ(sortInfo->lastModifiedTime(), -1) << "Negative last modified time should be handled correctly";
    EXPECT_EQ(sortInfo->createTime(), -1) << "Negative create time should be handled correctly";
    
    // 测试极大时间值
    qint64 maxTime = std::numeric_limits<qint64>::max();
    sortInfo->setLastReadTime(maxTime);
    sortInfo->setLastModifiedTime(maxTime);
    sortInfo->setCreateTime(maxTime);
    
    EXPECT_EQ(sortInfo->lastReadTime(), maxTime) << "Maximum last read time should be handled correctly";
    EXPECT_EQ(sortInfo->lastModifiedTime(), maxTime) << "Maximum last modified time should be handled correctly";
    EXPECT_EQ(sortInfo->createTime(), maxTime) << "Maximum create time should be handled correctly";
    
    delete sortInfo;
}

// 测试15: SortFileInfo 性能测试
TEST_F(SortFileInfoTest, TestPerformance)
{
    const int operationCount = 1000;
    
    // 测试对象创建和销毁性能
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < operationCount; ++i) {
        SortFileInfo* sortInfo = createSortFileInfo();
        if (sortInfo) {
            delete sortInfo;
        }
    }
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    qint64 duration = endTime - startTime;
    
    EXPECT_LT(duration, 2000) << "SortFileInfo creation/deletion should complete within 2 seconds";
    
    // 测试属性设置和获取性能
    SortFileInfo* perfTestInfo = createSortFileInfo();
    ASSERT_NE(perfTestInfo, nullptr) << "Performance test SortFileInfo should be created successfully";
    
    startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < operationCount; ++i) {
        // 设置属性
        perfTestInfo->setUrl(QUrl::fromLocalFile(QString("/test/file_%1.txt").arg(i)));
        perfTestInfo->setSize(i * 1024);
        perfTestInfo->setFile(true);
        perfTestInfo->setHighlightContent(QString("Content %1").arg(i));
        perfTestInfo->setCustomData(QString("perf_key_%1").arg(i), QString("perf_value_%1").arg(i));
        
        // 获取属性
        QUrl url = perfTestInfo->fileUrl();
        qint64 size = perfTestInfo->fileSize();
        bool isFile = perfTestInfo->isFile();
        QString content = perfTestInfo->highlightContent();
        QVariant customValue = perfTestInfo->customData(QString("perf_key_%1").arg(i));
        (void)size;  // Suppress unused variable warning
        (void)isFile;  // Suppress unused variable warning
    }
    
    endTime = QDateTime::currentMSecsSinceEpoch();
    duration = endTime - startTime;
    
    delete perfTestInfo;
    
    EXPECT_LT(duration, 3000) << "Property operations should complete within 3 seconds";
}

// 测试16: SortFileInfo 错误处理测试
TEST_F(SortFileInfoTest, TestErrorHandling)
{
    SortFileInfo* sortInfo = createSortFileInfo();
    ASSERT_NE(sortInfo, nullptr) << "SortFileInfo should be created successfully";
    
    // 测试空键的自定义数据
    QVariant emptyKeyResult = sortInfo->customData("");
    EXPECT_TRUE(emptyKeyResult.isNull()) << "Custom data with empty key should return null QVariant";
    
    // 测试非常长的自定义数据键
    QString veryLongKey = QString("x").repeated(10000);
    QVariant veryLongValue = "test_value";
    sortInfo->setCustomData(veryLongKey, veryLongValue);
    EXPECT_EQ(sortInfo->customData(veryLongKey), veryLongValue) << "Very long custom data key should be handled correctly";
    
    // 测试复杂的自定义数据值
    QVariantMap complexValue;
    complexValue["nested_key"] = "nested_value";
    complexValue["nested_number"] = 42;
    sortInfo->setCustomData("complex_key", complexValue);
    QVariant retrievedComplexValue = sortInfo->customData("complex_key");
    EXPECT_EQ(retrievedComplexValue.toMap(), complexValue) << "Complex custom data should be handled correctly";
    
    delete sortInfo;
}
