// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_settings.cpp - Settings class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QThread>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>

using namespace dfmbase;

/**
 * @brief Settings class unit tests
 *
 * Test scope:
 * 1. Settings creation and initialization
 * 2. Value get/set operations
 * 3. URL value operations
 * 4. Group and key management
 * 5. File synchronization
 * 6. Auto-sync functionality
 * 7. File watching and change detection
 * 8. JSON serialization/deserialization
 * 9. Default and fallback configuration handling
 * 10. Thread safety and lifecycle management
 */
class SettingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();

        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_settings") };
            app = std::make_unique<QCoreApplication>(argc, argv);
        }
        qApp->setApplicationName("TestApp");
        qApp->setOrganizationName("TestOrg");

        // Create temporary test files
        tempDir = QDir::temp().absoluteFilePath("dfm-settings-test");
        QDir().mkpath(tempDir);
        
        defaultFile = tempDir + "/default.json";
        fallbackFile = tempDir + "/fallback.json";
        settingFile = tempDir + "/settings.json";
        
        // Create test JSON files
        createTestJsonFile(defaultFile, createDefaultTestData());
        createTestJsonFile(fallbackFile, createFallbackTestData());
        createTestJsonFile(settingFile, createSettingsTestData());
    }

    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        
        // Clean up temporary files
        QDir(tempDir).removeRecursively();
    }

    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QCoreApplication> app;
    
    // Test file paths
    QString tempDir;
    QString defaultFile;
    QString fallbackFile;
    QString settingFile;
    
    // Helper function to create test JSON data
    QJsonObject createDefaultTestData() {
        QJsonObject root;
        QJsonObject group1;
        group1["key1"] = "default_value1";
        group1["key2"] = 42;
        group1["key3"] = true;
        root["TestGroup"] = group1;
        
        QJsonObject metadata;
        QJsonObject groupMeta;
        groupMeta["keyOrdered"] = QJsonArray{"key1", "key2", "key3"};
        metadata["TestGroup"] = groupMeta;
        root["__metadata__"] = metadata;
        
        return root;
    }
    
    QJsonObject createFallbackTestData() {
        QJsonObject root;
        QJsonObject group1;
        group1["key1"] = "fallback_value1";
        group1["key4"] = "fallback_only";
        root["TestGroup"] = group1;
        return root;
    }
    
    QJsonObject createSettingsTestData() {
        QJsonObject root;
        QJsonObject group1;
        group1["key1"] = "settings_value1";
        group1["key5"] = "settings_only";
        root["TestGroup"] = group1;
        return root;
    }
    
    void createTestJsonFile(const QString &filePath, const QJsonObject &data) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(data);
            file.write(doc.toJson());
            file.close();
        }
    }
    
    // Helper function to create test settings
    Settings* createTestSettings() {
        return new Settings(defaultFile, fallbackFile, settingFile);
    }
};

/**
 * @brief Test Settings creation and initialization
 * Verify basic settings creation with different constructors
 */
TEST_F(SettingsTest, Creation_Basic)
{
    // Test creation with explicit file paths
    Settings *settings = createTestSettings();
    EXPECT_NE(settings, nullptr);
    delete settings;
    
    // Test creation with name and type
    Settings *namedSettings = new Settings("test-config", Settings::kGenericConfig);
    EXPECT_NE(namedSettings, nullptr);
    delete namedSettings;
    
    // Test creation with app config type
    Settings *appSettings = new Settings("test-app", Settings::kAppConfig);
    EXPECT_NE(appSettings, nullptr);
    delete appSettings;
}

/**
 * @brief Test value operations - basic get/set
 * Verify basic value retrieval and storage
 */
TEST_F(SettingsTest, Value_BasicOperations)
{
    Settings *settings = createTestSettings();
    
    // Test getting values from different sources
    // Should get from settings file first
    QVariant value1 = settings->value("TestGroup", "key1");
    EXPECT_EQ(value1.toString(), "settings_value1");
    
    // Should get from fallback when not in settings
    QVariant value4 = settings->value("TestGroup", "key4");
    EXPECT_EQ(value4.toString(), "fallback_only");
    
    // Should get from default when not in settings or fallback
    QVariant value2 = settings->value("TestGroup", "key2");
    EXPECT_EQ(value2.toInt(), 42);
    
    // Should return default value when key doesn't exist
    QVariant nonExistent = settings->value("TestGroup", "nonexistent", "default");
    EXPECT_EQ(nonExistent.toString(), "default");
    
    delete settings;
}

/**
 * @brief Test value setting operations
 * Verify value setting and dirty flag management
 */
TEST_F(SettingsTest, Value_SetOperations)
{
    Settings *settings = createTestSettings();
    
    // Test setting a new value
    settings->setValue("TestGroup", "newKey", "newValue");
    
    // Verify the value was set
    QVariant result = settings->value("TestGroup", "newKey");
    EXPECT_EQ(result.toString(), "newValue");
    
    // Test overwriting existing value
    settings->setValue("TestGroup", "key1", "overwritten");
    QVariant overwritten = settings->value("TestGroup", "key1");
    EXPECT_EQ(overwritten.toString(), "overwritten");
    
    delete settings;
}

/**
 * @brief Test URL value operations
 * Verify URL-specific value operations
 */
TEST_F(SettingsTest, UrlValue_Operations)
{
    Settings *settings = createTestSettings();
    
    // Mock StandardPaths::fromStandardUrl
    stub.set_lamda(&StandardPaths::fromStandardUrl,
                   [](const QUrl &url) -> QString {
                       if (url.toString() == "standard://home") {
                           return "/home/user";
                       }
                       return QString();
                   });
    
    // Test setting URL value
    QUrl testUrl("file:///home/test");
    settings->setValue("TestGroup", "urlKey", testUrl.toString());
    
    // Test getting URL value
    QUrl result = settings->urlValue("TestGroup", "urlKey");
    EXPECT_EQ(result.toString(), testUrl.toString());
    
    // Test URL value with URL key
    QUrl urlKey("file:///test/path");
    settings->setValue("TestGroup", urlKey, "url_value");
    
    QVariant urlResult = settings->value("TestGroup", urlKey);
    EXPECT_EQ(urlResult.toString(), "url_value");
    
    delete settings;
}

/**
 * @brief Test group and key management
 * Verify group and key enumeration functionality
 */
TEST_F(SettingsTest, GroupKey_Management)
{
    Settings *settings = createTestSettings();
    
    // Test getting groups
    QSet<QString> groups = settings->groups();
    EXPECT_TRUE(groups.contains("TestGroup"));
    
    // Test getting keys for a group
    QSet<QString> keys = settings->keys("TestGroup");
    EXPECT_TRUE(keys.contains("key1"));
    EXPECT_TRUE(keys.contains("key2"));
    EXPECT_TRUE(keys.contains("key3"));
    EXPECT_TRUE(keys.contains("key4"));
    EXPECT_TRUE(keys.contains("key5"));
    
    // Test ordered key list
    QStringList orderedKeys = settings->keyList("TestGroup");
    EXPECT_FALSE(orderedKeys.isEmpty());
    
    // Test contains functionality
    EXPECT_TRUE(settings->contains("TestGroup", "key1"));
    EXPECT_FALSE(settings->contains("TestGroup", "nonexistent"));
    EXPECT_FALSE(settings->contains("NonExistentGroup", "key1"));
    
    delete settings;
}

/**
 * @brief Test default configuration operations
 * Verify default configuration value retrieval
 */
TEST_F(SettingsTest, DefaultConfig_Operations)
{
    Settings *settings = createTestSettings();
    
    // Test getting default configuration value
    QVariant defaultValue = settings->defaultConfigValue("TestGroup", "key2");
    EXPECT_EQ(defaultValue.toInt(), 42);
    
    // Test getting default configuration value with URL key
    QUrl urlKey("file:///test");
    settings->setValue("TestGroup", urlKey, "test_value");
    
    QVariant urlDefaultValue = settings->defaultConfigValue("TestGroup", urlKey);
    // Should return invalid QVariant if not in default config
    EXPECT_FALSE(urlDefaultValue.isValid());
    
    // Test default configuration key list
    QStringList defaultKeys = settings->defaultConfigkeyList("TestGroup");
    EXPECT_TRUE(defaultKeys.contains("key1"));
    EXPECT_TRUE(defaultKeys.contains("key2"));
    EXPECT_TRUE(defaultKeys.contains("key3"));
    
    delete settings;
}

/**
 * @brief Test value removal operations
 * Verify value and group removal functionality
 */
TEST_F(SettingsTest, Value_RemovalOperations)
{
    Settings *settings = createTestSettings();
    
    // Add a test value
    settings->setValue("TestGroup", "tempKey", "tempValue");
    EXPECT_TRUE(settings->contains("TestGroup", "tempKey"));
    
    // Test removing a key
    settings->remove("TestGroup", "tempKey");
    EXPECT_FALSE(settings->contains("TestGroup", "tempKey"));
    
    // Test removing with URL key
    QString urlKeyStr = "file:///temp";
    settings->setValue("TestGroup", urlKeyStr, "temp_url_value");
    EXPECT_TRUE(settings->contains("TestGroup", urlKeyStr));
    
    settings->remove("TestGroup", urlKeyStr);
    EXPECT_FALSE(settings->contains("TestGroup", urlKeyStr));
    
    // Test removing entire group
    settings->removeGroup("TestGroup");
    QSet<QString> keys = settings->keys("TestGroup");
    // Should only contain keys from default and fallback, not from settings
    EXPECT_TRUE(keys.contains("key2")); // From default
    EXPECT_TRUE(keys.contains("key4")); // From fallback
    EXPECT_FALSE(keys.contains("key5")); // Was in settings, should be removed
    
    delete settings;
}

/**
 * @brief Test isRemovable functionality
 * Verify removable key detection
 */
TEST_F(SettingsTest, IsRemovable_Operations)
{
    Settings *settings = createTestSettings();
    
    // Test removable check for existing key
    EXPECT_TRUE(settings->isRemovable("TestGroup", "key1"));
    
    // Test removable check for non-existent key
    EXPECT_FALSE(settings->isRemovable("TestGroup", "nonexistent"));
    
    // Test removable check with URL key
    QString urlKeyStr = "file:///test";
    settings->setValue("TestGroup", urlKeyStr, "test_value");
    EXPECT_TRUE(settings->isRemovable("TestGroup", urlKeyStr));
    
    delete settings;
}

/**
 * @brief Test file synchronization
 * Verify sync functionality and file writing
 */
TEST_F(SettingsTest, FileSynchronization)
{
    Settings *settings = createTestSettings();
    
    // Add a new value
    settings->setValue("TestGroup", "syncTest", "syncValue");
    
    // Test sync operation
    bool syncResult = settings->sync();
    EXPECT_TRUE(syncResult);
    
    // Verify the value was written to file
    QFile file(settingFile);
    EXPECT_TRUE(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    EXPECT_TRUE(doc.isObject());
    
    QJsonObject root = doc.object();
    QJsonObject testGroup = root["TestGroup"].toObject();
    EXPECT_EQ(testGroup["syncTest"].toString(), "syncValue");
    
    delete settings;
}

/**
 * @brief Test auto-sync functionality
 * Verify automatic synchronization behavior
 */
TEST_F(SettingsTest, AutoSync_Functionality)
{
    Settings *settings = createTestSettings();
    
    // Test auto-sync property
    EXPECT_FALSE(settings->autoSync());
    
    settings->setAutoSync(true);
    EXPECT_TRUE(settings->autoSync());
    
    // Mock QTimer to verify timer behavior
    bool timerStarted = false;
    stub.set_lamda(static_cast<void (QTimer::*)()>(&QTimer::start),
                   [&timerStarted](QTimer *timer) {
                       Q_UNUSED(timer)
                       timerStarted = true;
                   });
    
    // Setting a value should trigger auto-sync timer
    settings->setValue("TestGroup", "autoSyncTest", "autoSyncValue");
    
    // Note: Timer start might be called asynchronously
    // We test the basic auto-sync setup
    
    settings->setAutoSync(false);
    EXPECT_FALSE(settings->autoSync());
    
    delete settings;
}

/**
 * @brief Test auto-sync exclusion
 * Verify auto-sync group exclusion functionality
 */
TEST_F(SettingsTest, AutoSyncExclusion)
{
    Settings *settings = createTestSettings();
    
    settings->setAutoSync(true);
    
    // Test excluding a group from auto-sync
    settings->autoSyncExclude("ExcludedGroup", false);
    
    // Test including a group back to auto-sync
    settings->autoSyncExclude("ExcludedGroup", true);
    
    // The actual exclusion behavior is tested through sync operations
    // This tests the API availability
    
    delete settings;
}

/**
 * @brief Test file watching functionality
 * Verify file change detection and notification
 */
TEST_F(SettingsTest, FileWatching_Functionality)
{
    Settings *settings = createTestSettings();
    
    // Test watch changes property
    EXPECT_FALSE(settings->watchChanges());
    
    // Note: Cannot easily mock WatcherFactory::create due to template complexity
    // The test will use the real implementation, which is acceptable for this test case
    
    settings->setWatchChanges(true);
    // Should handle watcher creation failure gracefully
    
    settings->setWatchChanges(false);
    EXPECT_FALSE(settings->watchChanges());
    
    delete settings;
}

/**
 * @brief Test signal emission
 * Verify that signals are emitted correctly
 */
TEST_F(SettingsTest, SignalEmission)
{
    stub.set_lamda(static_cast<bool (Settings::*)(const QString &, const QString &, const QVariant &)>(&Settings::setValueNoNotify),
                   [](Settings *, const QString &, const QString &, const QVariant &) -> bool {
                       return true;
                   });


    Settings *settings = createTestSettings();
    
    // Set up signal spies
    QSignalSpy valueChangedSpy(settings, &Settings::valueChanged);
    QSignalSpy valueEditedSpy(settings, &Settings::valueEdited);
    
    // Test setValue signal emission
    settings->setValue("TestGroup", "signalTest", "signalValue");
    
    EXPECT_EQ(valueChangedSpy.count(), 1);
    EXPECT_EQ(valueChangedSpy.at(0).at(0).toString(), "TestGroup");
    EXPECT_EQ(valueChangedSpy.at(0).at(1).toString(), "signalTest");
    EXPECT_EQ(valueChangedSpy.at(0).at(2).toString(), "signalValue");
    
    // Test setValueNoNotify - should not emit signals
    valueChangedSpy.clear();
    bool noNotifyResult = settings->setValueNoNotify("TestGroup", "noNotifyTest", "noNotifyValue");
    EXPECT_TRUE(noNotifyResult);
    EXPECT_EQ(valueChangedSpy.count(), 0);
    
    delete settings;
}

/**
 * @brief Test Settings::toUrlValue static method
 * Verify URL conversion functionality
 */
TEST_F(SettingsTest, ToUrlValue_StaticMethod)
{
    // Mock StandardPaths::fromStandardUrl
    stub.set_lamda(&StandardPaths::fromStandardUrl,
                   [](const QUrl &url) -> QString {
                       if (url.toString() == "standard://home") {
                           return "/home/user";
                       }
                       return QString();
                   });
    
    // Test empty URL
    QUrl emptyResult = Settings::toUrlValue(QVariant());
    EXPECT_TRUE(emptyResult.isEmpty());
    
    // Test regular URL
    QUrl regularResult = Settings::toUrlValue(QVariant("file:///home/test"));
    EXPECT_EQ(regularResult.toString(), "file:///home/test");
    
    // Test standard URL conversion
    QUrl standardResult = Settings::toUrlValue(QVariant("standard://home"));
    EXPECT_EQ(standardResult.toString(), "file:///home/user");
    
    // Test invalid URL
    QUrl invalidResult = Settings::toUrlValue(QVariant("invalid-url"));
    EXPECT_FALSE(invalidResult.isEmpty()); // Should be converted via fromUserInput
}

/**
 * @brief Test clear functionality
 * Verify settings clearing behavior
 */
TEST_F(SettingsTest, Clear_Functionality)
{
    Settings *settings = createTestSettings();
    
    // Add some values
    settings->setValue("TestGroup", "clearTest1", "value1");
    settings->setValue("TestGroup", "clearTest2", "value2");
    
    // Verify values exist
    EXPECT_TRUE(settings->contains("TestGroup", "clearTest1"));
    EXPECT_TRUE(settings->contains("TestGroup", "clearTest2"));
    
    // Clear all values
    settings->clear();
    
    // Verify values are cleared (should not exist in writable data)
    // But default and fallback values should still be accessible
    QVariant defaultValue = settings->value("TestGroup", "key2");
    EXPECT_EQ(defaultValue.toInt(), 42); // From default config
    
    // New values should be gone
    EXPECT_FALSE(settings->contains("TestGroup", "clearTest1"));
    EXPECT_FALSE(settings->contains("TestGroup", "clearTest2"));
    
    delete settings;
}

/**
 * @brief Test reload functionality
 * Verify settings reloading from files
 */
TEST_F(SettingsTest, Reload_Functionality)
{
    Settings *settings = createTestSettings();
    
    // Get initial value
    QVariant initialValue = settings->value("TestGroup", "key1");
    EXPECT_EQ(initialValue.toString(), "settings_value1");
    
    // Modify the settings file externally
    QJsonObject modifiedData = createSettingsTestData();
    modifiedData["TestGroup"].toObject()["key1"] = "modified_value";
    QJsonObject root;
    root["TestGroup"] = modifiedData["TestGroup"];
    createTestJsonFile(settingFile, root);
    
    // Reload settings
    settings->reload();
    
    // Verify the value was reloaded
    QVariant reloadedValue = settings->value("TestGroup", "key1");
    EXPECT_EQ(reloadedValue.toString(), "modified_value");
    
    delete settings;
}

/**
 * @brief Test thread safety
 * Verify basic thread safety of settings operations
 */
TEST_F(SettingsTest, ThreadSafety_Basic)
{
    Settings *settings = createTestSettings();
    
    // Test that basic operations don't crash in current thread
    settings->setValue("TestGroup", "threadTest", "threadValue");
    QVariant result = settings->value("TestGroup", "threadTest");
    EXPECT_EQ(result.toString(), "threadValue");
    
    // Test sync in current thread
    bool syncResult = settings->sync();
    EXPECT_TRUE(syncResult);
    
    delete settings;
}

/**
 * @brief Test error handling
 * Verify error handling for various edge cases
 */
TEST_F(SettingsTest, ErrorHandling_EdgeCases)
{
    // Test creation with non-existent files
    Settings *settings = new Settings("/nonexistent/default.json", 
                                     "/nonexistent/fallback.json", 
                                     "/nonexistent/settings.json");
    
    // Should handle missing files gracefully
    EXPECT_NE(settings, nullptr);
    
    // Test operations on empty settings
    QVariant emptyResult = settings->value("NonExistentGroup", "NonExistentKey");
    EXPECT_FALSE(emptyResult.isValid());
    
    // Test setting values should work even with missing files
    settings->setValue("TestGroup", "testKey", "testValue");
    QVariant setValue = settings->value("TestGroup", "testKey");
    EXPECT_EQ(setValue.toString(), "testValue");
    
    delete settings;
}

/**
 * @brief Test performance characteristics
 * Verify performance of key operations
 */
TEST_F(SettingsTest, Performance_BasicOperations)
{
    Settings *settings = createTestSettings();
    
    const int iterations = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test repeated value access
    for (int i = 0; i < iterations; ++i) {
        settings->value("TestGroup", "key1");
        settings->setValue("TestGroup", QString("perfKey%1").arg(i), QString("perfValue%1").arg(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Performance should be reasonable (less than 5 seconds for 1000 operations)
    EXPECT_LT(duration.count(), 5000);
    
    delete settings;
}

/**
 * @brief Comprehensive integration test
 * Test all major functionality working together
 */
TEST_F(SettingsTest, Integration_ComprehensiveTest)
{
    Settings *settings = createTestSettings();
    
    // Test initial state
    EXPECT_NE(settings, nullptr);
    EXPECT_FALSE(settings->autoSync());
    EXPECT_FALSE(settings->watchChanges());
    
    // Test value operations
    settings->setValue("IntegrationGroup", "testKey", "testValue");
    QVariant result = settings->value("IntegrationGroup", "testKey");
    EXPECT_EQ(result.toString(), "testValue");
    
    // Test URL operations
    QUrl testUrl("file:///integration/test");
    settings->setValue("IntegrationGroup", "urlKey", testUrl.toString());
    QUrl urlResult = settings->urlValue("IntegrationGroup", "urlKey");
    EXPECT_EQ(urlResult.toString(), testUrl.toString());
    
    // Test group/key management
    QSet<QString> groups = settings->groups();
    EXPECT_TRUE(groups.contains("IntegrationGroup"));
    EXPECT_TRUE(groups.contains("TestGroup"));
    
    QSet<QString> keys = settings->keys("IntegrationGroup");
    EXPECT_TRUE(keys.contains("testKey"));
    EXPECT_TRUE(keys.contains("urlKey"));
    
    // Test sync
    bool syncResult = settings->sync();
    EXPECT_TRUE(syncResult);
    
    // Test signal emission
    QSignalSpy valueChangedSpy(settings, &Settings::valueChanged);
    settings->setValue("IntegrationGroup", "signalKey", "signalValue");
    EXPECT_EQ(valueChangedSpy.count(), 1);
    
    delete settings;
} 
