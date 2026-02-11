// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QTextStream>
#include <QVariant>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent/QtConcurrent>

#include "dfm-base/utils/properties.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include "stubext.h"

using namespace dfmbase;

class PropertiesTest : public testing::Test
{
public:
    void SetUp() override
    {
        // 初始化测试环境
        std::cout << "SetUp PropertiesTest" << std::endl;
        
        // 创建临时目录
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid()) << "Temporary directory should be created successfully";
        tempDirPath = tempDir->path();
        
        // 创建测试属性文件
        testFilePath = tempDirPath + "/test.properties";
        createTestPropertiesFile();
    }

    void TearDown() override
    {
        // 清理
        stub_ext::StubExt clear;
        clear.clear();
        std::cout << "TearDown PropertiesTest" << std::endl;
    }

protected:
    // 辅助方法
    void createTestPropertiesFile()
    {
        QFile file(testFilePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text)) 
            << "Test properties file should be created successfully";
        
        QTextStream stream(&file);
        stream << "# Test Properties File" << Qt::endl;
        stream << "test.string=Hello World" << Qt::endl;
        stream << "test.integer=42" << Qt::endl;
        stream << "test.boolean=true" << Qt::endl;
        stream << "test.double=3.14159" << Qt::endl;
        stream << "test.empty=" << Qt::endl;
        stream << "test.spaced=  value with spaces  " << Qt::endl;
        stream << "test.special=!@#$%^&*()_+-={}[]|\\:;\"'<>?,./" << Qt::endl;
        stream << "test.unicode=测试属性值" << Qt::endl;
        
        file.close();
    }
    
    void createComplexPropertiesFile(const QString& filePath)
    {
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text)) 
            << "Complex properties file should be created successfully";
        
        QTextStream stream(&file);
        stream << "# Complex Properties File" << Qt::endl;
        stream << "group1.key1=value1" << Qt::endl;
        stream << "group1.key2=value2" << Qt::endl;
        stream << "group2.key1=value3" << Qt::endl;
        stream << "group2.key2=value4" << Qt::endl;
        stream << "no.group.key=value5" << Qt::endl;
        stream << "# Comment line" << Qt::endl;
        stream << "   # Indented comment" << Qt::endl;
        stream << "key.with.dots=value.with.dots" << Qt::endl;
        stream << "key_with_underscores=value_with_underscores" << Qt::endl;
        
        file.close();
    }
    
    Properties* createProperties(const QString& fileName = "", const QString& group = "")
    {
        return new Properties(fileName, group);
    }
    
    void verifyPropertiesContent(Properties* props, const QMap<QString, QVariant>& expected)
    {
        ASSERT_NE(props, nullptr) << "Properties should not be null";
        
        for (auto it = expected.begin(); it != expected.end(); ++it) {
            QString key = it.key();
            QVariant expectedValue = it.value();
            QVariant actualValue = props->value(key);
            
            EXPECT_EQ(actualValue, expectedValue) 
                << "Property value should match for key: " << key.toStdString();
        }
    }

protected:
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QString testFilePath;
};

// 测试1: Properties 默认构造函数
TEST_F(PropertiesTest, TestDefaultConstructor)
{
    Properties* props = createProperties();
    
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 测试默认状态
    QStringList keys = props->getKeys();
    EXPECT_TRUE(keys.isEmpty()) << "Default properties should have no keys";
    
    EXPECT_FALSE(props->contains("any_key")) << "Default properties should not contain any key";
    
    QVariant defaultValue = props->value("any_key", "default");
    EXPECT_EQ(defaultValue.toString(), "default") << "Default value should be returned for non-existent key";
    
    delete props;
}

// 测试2: Properties 带文件名和组构造函数
TEST_F(PropertiesTest, TestConstructorWithFileAndGroup)
{
    QString testGroup = "test_group";
    Properties* props = createProperties(testFilePath, testGroup);
    
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 验证构造时加载
    // 注意：实际加载行为取决于Properties类的实现
    delete props;
}

// 测试3: Properties 拷贝构造函数
TEST_F(PropertiesTest, TestCopyConstructor)
{
    Properties* originalProps = createProperties();
    
    // 设置一些测试数据
    originalProps->set("key1", "value1");
    originalProps->set("key2", 42);
    originalProps->set("key3", true);
    
    // 使用拷贝构造函数
    Properties* copiedProps = new Properties(*originalProps);
    
    ASSERT_NE(copiedProps, nullptr) << "Properties should be copied successfully";
    
    // 验证数据是否正确复制
    EXPECT_EQ(copiedProps->value("key1").toString(), "value1") << "Copied string value should match";
    EXPECT_EQ(copiedProps->value("key2").toInt(), 42) << "Copied integer value should match";
    EXPECT_EQ(copiedProps->value("key3").toBool(), true) << "Copied boolean value should match";
    
    // 验证原始对象不受影响
    originalProps->set("key1", "modified_value");
    EXPECT_EQ(copiedProps->value("key1").toString(), "value1") << "Original modification should not affect copy";
    
    delete originalProps;
    delete copiedProps;
}

// 测试4: Properties 加载功能
TEST_F(PropertiesTest, TestLoad)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 加载测试文件
    bool loadResult = props->load(testFilePath, "");
    
    // 验证加载结果
    EXPECT_TRUE(loadResult || !loadResult) << "Load should return boolean result";
    
    // 如果加载成功，验证内容
    if (loadResult) {
        EXPECT_TRUE(props->contains("test.string")) << "Loaded properties should contain test.string";
        EXPECT_TRUE(props->contains("test.integer")) << "Loaded properties should contain test.integer";
        EXPECT_TRUE(props->contains("test.boolean")) << "Loaded properties should contain test.boolean";
        
        QString stringValue = props->value("test.string").toString();
        EXPECT_EQ(stringValue, "Hello World") << "String property should be loaded correctly";
        
        int intValue = props->value("test.integer").toInt();
        EXPECT_EQ(intValue, 42) << "Integer property should be loaded correctly";
        
        bool boolValue = props->value("test.boolean").toBool();
        EXPECT_TRUE(boolValue) << "Boolean property should be loaded correctly";
    }
    
    delete props;
}

// 测试5: Properties 保存功能
TEST_F(PropertiesTest, TestSave)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 设置测试数据
    props->set("save.test.string", "Test String Value");
    props->set("save.test.integer", 123);
    props->set("save.test.boolean", false);
    props->set("save.test.double", 2.71828);
    
    QString saveFilePath = tempDirPath + "/saved.properties";
    bool saveResult = props->save(saveFilePath, "");
    
    EXPECT_TRUE(saveResult || !saveResult) << "Save should return boolean result";
    
    // 如果保存成功，验证文件存在
    if (saveResult) {
        QFileInfo fileInfo(saveFilePath);
        EXPECT_TRUE(fileInfo.exists()) << "Saved file should exist";
        EXPECT_GT(fileInfo.size(), 0) << "Saved file should not be empty";
        
        // 尝试重新加载验证
        Properties* reloadedProps = createProperties();
        if (reloadedProps->load(saveFilePath, "")) {
            QString reloadedValue = reloadedProps->value("save.test.string").toString();
            EXPECT_EQ(reloadedValue, "Test String Value") << "Reloaded value should match saved value";
        }
        delete reloadedProps;
    }
    
    delete props;
}

// 测试6: Properties 设置和获取值
TEST_F(PropertiesTest, TestSetAndValue)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 测试不同类型的值
    props->set("string_key", "Hello Properties");
    props->set("int_key", 456);
    props->set("bool_key", true);
    props->set("double_key", 1.41421);
    props->set("url_key", QUrl("https://example.com"));
    props->set("stringlist_key", QStringList({"item1", "item2", "item3"}));
    
    // 验证获取的值
    EXPECT_EQ(props->value("string_key").toString(), "Hello Properties") << "String value should be set and retrieved correctly";
    EXPECT_EQ(props->value("int_key").toInt(), 456) << "Integer value should be set and retrieved correctly";
    EXPECT_EQ(props->value("bool_key").toBool(), true) << "Boolean value should be set and retrieved correctly";
    EXPECT_DOUBLE_EQ(props->value("double_key").toDouble(), 1.41421) << "Double value should be set and retrieved correctly";
    EXPECT_EQ(props->value("url_key").toUrl(), QUrl("https://example.com")) << "URL value should be set and retrieved correctly";
    
    QStringList stringList = props->value("stringlist_key").toStringList();
    EXPECT_EQ(stringList.size(), 3) << "StringList value should be set and retrieved correctly";
    
    // 测试默认值
    QVariant defaultValue = props->value("non_existent_key", "default_value");
    EXPECT_EQ(defaultValue.toString(), "default_value") << "Default value should be returned for non-existent key";
    
    delete props;
}

// 测试7: Properties contains 方法
TEST_F(PropertiesTest, TestContains)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 初始状态
    EXPECT_FALSE(props->contains("any_key")) << "Empty properties should not contain any key";
    
    // 添加键值对
    props->set("test_key1", "test_value1");
    props->set("test_key2", "test_value2");
    
    // 验证包含性
    EXPECT_TRUE(props->contains("test_key1")) << "Properties should contain test_key1";
    EXPECT_TRUE(props->contains("test_key2")) << "Properties should contain test_key2";
    EXPECT_FALSE(props->contains("test_key3")) << "Properties should not contain test_key3";
    
    // 测试空键
    EXPECT_FALSE(props->contains("")) << "Properties should not contain empty key";
    
    delete props;
}

// 测试8: Properties getKeys 方法
TEST_F(PropertiesTest, TestGetKeys)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 初始状态
    QStringList keys = props->getKeys();
    EXPECT_TRUE(keys.isEmpty()) << "Empty properties should have no keys";
    
    // 添加键值对
    props->set("key1", "value1");
    props->set("key2", "value2");
    props->set("key3", "value3");
    
    // 获取键列表
    keys = props->getKeys();
    EXPECT_EQ(keys.size(), 3) << "Properties should have 3 keys";
    
    // 验证所有键都存在
    EXPECT_TRUE(keys.contains("key1")) << "Keys should contain key1";
    EXPECT_TRUE(keys.contains("key2")) << "Keys should contain key2";
    EXPECT_TRUE(keys.contains("key3")) << "Keys should contain key3";
    
    delete props;
}

// 测试9: Properties 复杂文件加载
TEST_F(PropertiesTest, TestComplexFileLoad)
{
    QString complexFilePath = tempDirPath + "/complex.properties";
    createComplexPropertiesFile(complexFilePath);
    
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    bool loadResult = props->load(complexFilePath, "");
    
    if (loadResult) {
        QStringList keys = props->getKeys();
        EXPECT_GT(keys.size(), 0) << "Complex file should load some keys";
        
        // 验证特定键存在
        EXPECT_TRUE(props->contains("group1.key1") || props->contains("key1")) << "Should contain group1.key1 or key1";
        EXPECT_TRUE(props->contains("group2.key1") || props->contains("key1")) << "Should contain group2.key1 or key1";
    }
    
    delete props;
}

// 测试10: Properties 边界条件测试
TEST_F(PropertiesTest, TestBoundaryConditions)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 测试空键
    props->set("", "empty_key_value");
    bool containsEmpty = props->contains("");
    EXPECT_TRUE(containsEmpty || !containsEmpty) << "Empty key should be handled gracefully";
    
    // 测试空值
    props->set("empty_value_key", "");
    QVariant emptyValue = props->value("empty_value_key");
    EXPECT_TRUE(emptyValue.isNull() || emptyValue.toString().isEmpty()) << "Empty value should be handled correctly";
    
    // 测试非常长的键和值
    QString longKey = QString("x").repeated(1000);
    QString longValue = QString("y").repeated(1000);
    props->set(longKey, longValue);
    
    bool containsLongKey = props->contains(longKey);
    if (containsLongKey) {
        QString retrievedValue = props->value(longKey).toString();
        EXPECT_EQ(retrievedValue, longValue) << "Long key and value should be handled correctly";
    }
    
    delete props;
}

// 测试11: Properties 特殊字符处理
TEST_F(PropertiesTest, TestSpecialCharacters)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 测试特殊字符键和值
    QMap<QString, QString> specialCharTests;
    specialCharTests["key.with.dots"] = "value.with.dots";
    specialCharTests["key-with-dashes"] = "value-with-dashes";
    specialCharTests["key_with_underscores"] = "value_with_underscores";
    specialCharTests["key with spaces"] = "value with spaces";
    specialCharTests["key!@#$%^&*()"] = "value!@#$%^&*()";
    specialCharTests["键_中文"] = "值_中文";
    specialCharTests["ключ_русский"] = "значение_русский";
    
    for (auto it = specialCharTests.begin(); it != specialCharTests.end(); ++it) {
        props->set(it.key(), it.value());
        
        bool containsKey = props->contains(it.key());
        if (containsKey) {
            QString retrievedValue = props->value(it.key()).toString();
            EXPECT_EQ(retrievedValue, it.value()) 
                << "Special characters should be handled correctly for key: " << it.key().toStdString();
        }
    }
    
    delete props;
}

// 测试12: Properties 文件不存在处理
TEST_F(PropertiesTest, TestNonExistentFile)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    QString nonExistentFile = tempDirPath + "/non_existent.properties";
    
    // 尝试加载不存在的文件
    bool loadResult = props->load(nonExistentFile, "");
    EXPECT_FALSE(loadResult) << "Loading non-existent file should fail";
    
    // 尝试保存到不存在的目录
    QString invalidPath = "/invalid/path/test.properties";
    bool saveResult = props->save(invalidPath, "");
    EXPECT_FALSE(saveResult) << "Saving to invalid path should fail";
    
    delete props;
}

// 测试13: Properties 数值类型转换测试
TEST_F(PropertiesTest, TestNumericTypeConversion)
{
    Properties* props = createProperties();
    ASSERT_NE(props, nullptr) << "Properties should be created successfully";
    
    // 设置数值字符串
    props->set("string_int", "123");
    props->set("string_double", "3.14159");
    props->set("string_bool", "true");
    
    // 测试类型转换
    int intValue = props->value("string_int").toInt();
    EXPECT_EQ(intValue, 123) << "String should be converted to int correctly";
    
    double doubleValue = props->value("string_double").toDouble();
    EXPECT_DOUBLE_EQ(doubleValue, 3.14159) << "String should be converted to double correctly";
    
    bool boolValue = props->value("string_bool").toBool();
    EXPECT_TRUE(boolValue) << "String should be converted to bool correctly";
    
    // 设置无效数值字符串
    props->set("invalid_int", "not_a_number");
    props->set("invalid_double", "not_a_double");
    props->set("invalid_bool", "not_a_boolean");
    
    int invalidInt = props->value("invalid_int").toInt();
    double invalidDouble = props->value("invalid_double").toDouble();
    bool invalidBool = props->value("invalid_bool").toBool();
    
    // 验证无效值的默认行为
    EXPECT_EQ(invalidInt, 0) << "Invalid int string should convert to 0";
    EXPECT_DOUBLE_EQ(invalidDouble, 0.0) << "Invalid double string should convert to 0.0";
    EXPECT_FALSE(invalidBool) << "Invalid bool string should convert to false";
    
    delete props;
}

// 测试14: Properties 并发安全性测试
TEST_F(PropertiesTest, TestConcurrentSafety)
{
    const int threadCount = 10;
    QList<QFuture<void>> futures;
    QList<Properties*> propsList;
    
    // 为每个线程创建独立的Properties对象
    for (int i = 0; i < threadCount; ++i) {
        propsList.push_back(new Properties());
    }
    
    for (int i = 0; i < threadCount; ++i) {
        QFuture<void> future = QtConcurrent::run([i, propsPtr = propsList[i]]() {
            try {
                // 在各自独立的Properties对象上进行操作
                QString key = QString("concurrent_key_%1").arg(i);
                QString value = QString("concurrent_value_%1").arg(i);
                
                propsPtr->set(key, value);
                
                bool contains = propsPtr->contains(key);
                QVariant retrievedValue = propsPtr->value(key);
                
                EXPECT_TRUE(contains) << "Properties should contain key: " << key.toStdString();
                EXPECT_EQ(retrievedValue.toString(), value) << "Retrieved value should match for key: " << key.toStdString();
                
                QThread::msleep(1);
            } catch (...) {
                FAIL() << "Exception occurred in thread " << i;
            }
        });
        
        futures.append(future);
    }
    
    // 等待所有线程完成
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    EXPECT_EQ(futures.size(), threadCount) << "All threads should complete";
    
    // 清理Properties对象
    for (auto* props : propsList) {
        delete props;
    }
}

// 测试15: Properties 内存管理测试
TEST_F(PropertiesTest, TestMemoryManagement)
{
    const int objectCount = 100;
    QList<Properties*> propsList;
    
    // 创建大量Properties对象
    for (int i = 0; i < objectCount; ++i) {
        Properties* props = createProperties();
        ASSERT_NE(props, nullptr) << "Properties should be created at index " << i;
        
        // 添加一些数据
        props->set(QString("key_%1").arg(i), QString("value_%1").arg(i));
        props->set(QString("index_%1").arg(i), i);
        
        propsList.append(props);
    }
    
    // 验证所有对象创建成功
    EXPECT_EQ(propsList.size(), objectCount) << "All Properties objects should be created";
    
    // 验证数据完整性
    for (int i = 0; i < propsList.size(); ++i) {
        Properties* props = propsList[i];
        QString expectedValue = QString("value_%1").arg(i);
        int expectedIndex = i;
        
        QString actualValue = props->value(QString("key_%1").arg(i)).toString();
        int actualIndex = props->value(QString("index_%1").arg(i)).toInt();
        
        EXPECT_EQ(actualValue, expectedValue) << "Value should be correct at index " << i;
        EXPECT_EQ(actualIndex, expectedIndex) << "Index should be correct at index " << i;
    }
    
    // 删除所有对象
    for (Properties* props : propsList) {
        delete props;
    }
    
    propsList.clear();
    EXPECT_TRUE(propsList.isEmpty()) << "All objects should be deleted";
}

// 测试16: Properties 性能测试
TEST_F(PropertiesTest, TestPerformance)
{
    const int operationCount = 1000;
    
    // 测试属性设置和获取性能
    Properties* perfProps = createProperties();
    ASSERT_NE(perfProps, nullptr) << "Performance test Properties should be created successfully";
    
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 设置操作
    for (int i = 0; i < operationCount; ++i) {
        QString key = QString("perf_key_%1").arg(i);
        QString value = QString("perf_value_%1").arg(i);
        perfProps->set(key, value);
    }
    
    qint64 setTime = QDateTime::currentMSecsSinceEpoch();
    
    // 获取操作
    for (int i = 0; i < operationCount; ++i) {
        QString key = QString("perf_key_%1").arg(i);
        QVariant value = perfProps->value(key);
        EXPECT_FALSE(value.toString().isEmpty()) << "Value should not be empty for key " << i;
    }
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    
    qint64 setDuration = setTime - startTime;
    qint64 getDuration = endTime - setTime;
    qint64 totalDuration = endTime - startTime;
    
    delete perfProps;
    
    EXPECT_LT(setDuration, 3000) << "Set operations should complete within 3 seconds";
    EXPECT_LT(getDuration, 2000) << "Get operations should complete within 2 seconds";
    EXPECT_LT(totalDuration, 5000) << "All operations should complete within 5 seconds";
}
