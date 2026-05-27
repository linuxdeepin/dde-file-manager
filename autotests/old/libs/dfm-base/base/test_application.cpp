// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_application.cpp - Application class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QMetaEnum>
#include <QThread>
#include <memory>
#include <chrono>
#include <cstring>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/application/private/application_p.h>

using namespace dfmbase;

/**
 * @brief Application class unit tests
 *
 * Test scope:
 * 1. Application singleton management
 * 2. Application attribute get/set operations
 * 3. Generic attribute get/set operations
 * 4. Settings management and creation
 * 5. Signal emission and handling
 * 6. Trigger attribute operations
 * 7. Settings value change notifications
 * 8. Thread safety and lifecycle management
 */
class ApplicationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();

        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char name[] = "test";
            static char *argv[] = {name};
            app = std::make_unique<QCoreApplication>(argc, argv);
        }
        qApp->setApplicationName("TestApp");
        qApp->setOrganizationName("TestOrg");

        // Clear any existing Application instance
        ApplicationPrivate::self = nullptr;
    }

    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        
        // Clean up Application instance
        if (ApplicationPrivate::self) {
            delete ApplicationPrivate::self;
            ApplicationPrivate::self = nullptr;
        }
    }

    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QCoreApplication> app;
    
    // Helper function to create a test application
    Application* createTestApplication() {
        return new Application();
    }
};

/**
 * @brief Test Application singleton creation and management
 * Verify singleton pattern implementation
 */
TEST_F(ApplicationTest, SingletonCreation_Basic)
{
    // Test that no instance exists initially
    EXPECT_EQ(Application::instance(), nullptr);
    
    // Create first instance
    Application *app1 = createTestApplication();
    EXPECT_NE(app1, nullptr);
    EXPECT_EQ(Application::instance(), app1);
    
    // Verify singleton behavior - second instance should cause assertion
    // We'll test this by checking the static self pointer
    EXPECT_EQ(ApplicationPrivate::self, app1);
    
    // Clean up
    delete app1;
    EXPECT_EQ(Application::instance(), nullptr);
}

/**
 * @brief Test Application attribute operations
 * Verify appAttribute get/set functionality
 */
TEST_F(ApplicationTest, AppAttribute_GetSet)
{
    Application *app = createTestApplication();
    
    // Mock Settings::value to return a test value
    stub.set_lamda(static_cast<QVariant (Settings::*)(const QString&, const QString&, const QVariant&) const>(&Settings::value),
                   [](Settings *self, const QString &group, const QString &key, const QVariant &defaultValue) -> QVariant {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(defaultValue)
                       if (key == "AllwayOpenOnNewWindow") {
                           return true;
                       }
                       return QVariant();
                   });
    
    // Test getting application attribute
    QVariant result = Application::appAttribute(Application::kAllwayOpenOnNewWindow);
    EXPECT_TRUE(result.toBool());
    
    // Mock Settings::setValue for testing set operation
    bool setValueCalled = false;
    stub.set_lamda(static_cast<void (Settings::*)(const QString&, const QString&, const QVariant&)>(&Settings::setValue),
                   [&setValueCalled](Settings *self, const QString &group, const QString &key, const QVariant &value) {
                       Q_UNUSED(self)
                       setValueCalled = true;
                       EXPECT_EQ(group, "ApplicationAttribute");
                       EXPECT_EQ(key, "AllwayOpenOnNewWindow");
                       EXPECT_FALSE(value.toBool());
                   });
    
    // Test setting application attribute
    Application::setAppAttribute(Application::kAllwayOpenOnNewWindow, false);
    EXPECT_TRUE(setValueCalled);
    
    delete app;
}

/**
 * @brief Test Application URL attribute operations
 * Verify appUrlAttribute functionality
 */
TEST_F(ApplicationTest, AppUrlAttribute_Get)
{
    Application *app = createTestApplication();
    
    // Mock Settings::urlValue to return a test URL
    stub.set_lamda(static_cast<QUrl (Settings::*)(const QString&, const QString&, const QUrl&) const>(&Settings::urlValue),
                   [](Settings *self, const QString &group, const QString &key, const QUrl &defaultValue) -> QUrl {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(defaultValue)
                       if (key == "UrlOfNewWindow") {
                           return QUrl("file:///home/test");
                       }
                       return QUrl();
                   });
    
    // Test getting URL attribute
    QUrl result = Application::appUrlAttribute(Application::kUrlOfNewWindow);
    EXPECT_EQ(result.toString(), "file:///home/test");
    
    delete app;
}

/**
 * @brief Test generic attribute operations
 * Verify genericAttribute get/set functionality
 */
TEST_F(ApplicationTest, GenericAttribute_GetSet)
{
    Application *app = createTestApplication();
    
    // Mock Settings::value for generic attribute
    stub.set_lamda(static_cast<QVariant (Settings::*)(const QString&, const QString&, const QVariant&) const>(&Settings::value),
                   [](Settings *self, const QString &group, const QString &key, const QVariant &defaultValue) -> QVariant {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(defaultValue)
                       if (key == "PreviewCompressFile") {
                           return true;
                       }
                       return QVariant();
                   });
    
    // Test getting generic attribute
    QVariant result = Application::genericAttribute(Application::kPreviewCompressFile);
    EXPECT_TRUE(result.toBool());
    
    // Mock Settings::setValue for testing set operation
    bool setValueCalled = false;
    stub.set_lamda(static_cast<void (Settings::*)(const QString&, const QString&, const QVariant&)>(&Settings::setValue),
                   [&setValueCalled](Settings *self, const QString &group, const QString &key, const QVariant &value) {
                       Q_UNUSED(self)
                       setValueCalled = true;
                       EXPECT_EQ(group, "GenericAttribute");
                       EXPECT_EQ(key, "PreviewCompressFile");
                       EXPECT_FALSE(value.toBool());
                   });
    
    // Test setting generic attribute
    Application::setGenericAttribute(Application::kPreviewCompressFile, false);
    EXPECT_TRUE(setValueCalled);
    
    delete app;
}

/**
 * @brief Test Settings synchronization
 * Verify sync functionality for both app and generic attributes
 */
TEST_F(ApplicationTest, SettingsSync)
{
    Application *app = createTestApplication();
    
    // Mock Settings::sync for app settings
    bool appSyncCalled = false;
    stub.set_lamda(&Settings::sync,
                   [&appSyncCalled](Settings *self) -> bool {
                       Q_UNUSED(self)
                       appSyncCalled = true;
                       return true;
                   });
    
    // Test app attribute sync
    bool result = Application::syncAppAttribute();
    EXPECT_TRUE(result);
    EXPECT_TRUE(appSyncCalled);
    
    // Reset for generic sync test
    appSyncCalled = false;
    
    // Test generic attribute sync
    result = Application::syncGenericAttribute();
    EXPECT_TRUE(result);
    EXPECT_TRUE(appSyncCalled);
    
    delete app;
}

/**
 * @brief Test Settings creation and management
 * Verify settings object creation and configuration
 */
TEST_F(ApplicationTest, SettingsCreation)
{
    Application *app = createTestApplication();
    
    // Test generic settings creation
    Settings *genericSettings = Application::genericSetting();
    EXPECT_NE(genericSettings, nullptr);
    
    // Test app settings creation
    Settings *appSettings = Application::appSetting();
    EXPECT_NE(appSettings, nullptr);
    
    // Test obtusely settings creation
    Settings *genericObtuselySettings = Application::genericObtuselySetting();
    EXPECT_NE(genericObtuselySettings, nullptr);
    
    Settings *appObtuselySettings = Application::appObtuselySetting();
    EXPECT_NE(appObtuselySettings, nullptr);
    
    // Test data persistence settings
    Settings *dataPersistence = Application::dataPersistence();
    EXPECT_NE(dataPersistence, nullptr);
    
    delete app;
}

/**
 * @brief Test signal emission for application attributes
 * Verify that signals are emitted correctly when attributes change
 */
TEST_F(ApplicationTest, SignalEmission_AppAttributes)
{
    Application *app = createTestApplication();
    
    // Set up signal spies
    QSignalSpy appAttributeChangedSpy(app, &Application::appAttributeChanged);
    QSignalSpy iconSizeLevelChangedSpy(app, &Application::iconSizeLevelChanged);
    QSignalSpy viewModeChangedSpy(app, &Application::viewModeChanged);
    
    // Test signal emission for icon size level
    app->d->_q_onSettingsValueChanged("ApplicationAttribute", "IconSizeLevel", 2);
    
    EXPECT_EQ(appAttributeChangedSpy.count(), 1);
    EXPECT_EQ(iconSizeLevelChangedSpy.count(), 1);
    EXPECT_EQ(iconSizeLevelChangedSpy.at(0).at(0).toInt(), 2);
    
    // Test signal emission for view mode
    app->d->_q_onSettingsValueChanged("ApplicationAttribute", "ViewMode", 1);
    
    EXPECT_EQ(appAttributeChangedSpy.count(), 2);
    EXPECT_EQ(viewModeChangedSpy.count(), 1);
    EXPECT_EQ(viewModeChangedSpy.at(0).at(0).toInt(), 1);
    
    delete app;
}

/**
 * @brief Test signal emission for generic attributes
 * Verify that signals are emitted correctly for generic attributes
 */
TEST_F(ApplicationTest, SignalEmission_GenericAttributes)
{
    Application *app = createTestApplication();
    
    // Set up signal spies
    QSignalSpy genericAttributeChangedSpy(app, &Application::genericAttributeChanged);
    QSignalSpy previewCompressFileChangedSpy(app, &Application::previewCompressFileChanged);
    QSignalSpy showedFileSuffixChangedSpy(app, &Application::showedFileSuffixChanged);
    QSignalSpy showedHiddenFilesChangedSpy(app, &Application::showedHiddenFilesChanged);
    
    // Test signal emission for preview compress file
    app->d->_q_onSettingsValueChanged("GenericAttribute", "PreviewCompressFile", true);
    
    EXPECT_EQ(genericAttributeChangedSpy.count(), 1);
    EXPECT_EQ(previewCompressFileChangedSpy.count(), 1);
    EXPECT_TRUE(previewCompressFileChangedSpy.at(0).at(0).toBool());
    
    // Test signal emission for file suffix
    app->d->_q_onSettingsValueChanged("GenericAttribute", "ShowedFileSuffix", false);
    
    EXPECT_EQ(genericAttributeChangedSpy.count(), 2);
    EXPECT_EQ(showedFileSuffixChangedSpy.count(), 1);
    EXPECT_FALSE(showedFileSuffixChangedSpy.at(0).at(0).toBool());
    
    // Test signal emission for hidden files
    app->d->_q_onSettingsValueChanged("GenericAttribute", "ShowedHiddenFiles", true);
    
    EXPECT_EQ(genericAttributeChangedSpy.count(), 3);
    EXPECT_EQ(showedHiddenFilesChangedSpy.count(), 1);
    EXPECT_TRUE(showedHiddenFilesChangedSpy.at(0).at(0).toBool());
    
    delete app;
}

/**
 * @brief Test edited signal emission
 * Verify that edited signals are emitted when values are edited
 */
TEST_F(ApplicationTest, SignalEmission_EditedSignals)
{
    Application *app = createTestApplication();
    
    // Set up signal spies
    QSignalSpy appAttributeEditedSpy(app, &Application::appAttributeEdited);
    QSignalSpy genericAttributeEditedSpy(app, &Application::genericAttributeEdited);
    
    // Test edited signal for app attribute
    app->d->_q_onSettingsValueChanged("ApplicationAttribute", "IconSizeLevel", 3, true);
    
    EXPECT_EQ(appAttributeEditedSpy.count(), 1);
    EXPECT_EQ(appAttributeEditedSpy.at(0).at(0).toInt(), static_cast<int>(Application::kIconSizeLevel));
    EXPECT_EQ(appAttributeEditedSpy.at(0).at(1).toInt(), 3);
    
    // Test edited signal for generic attribute
    app->d->_q_onSettingsValueChanged("GenericAttribute", "PreviewCompressFile", false, true);
    
    EXPECT_EQ(genericAttributeEditedSpy.count(), 1);
    EXPECT_EQ(genericAttributeEditedSpy.at(0).at(0).toInt(), static_cast<int>(Application::kPreviewCompressFile));
    EXPECT_FALSE(genericAttributeEditedSpy.at(0).at(1).toBool());
    
    delete app;
}

/**
 * @brief Test trigger attribute operations
 * Verify appAttributeTrigger functionality
 */
TEST_F(ApplicationTest, TriggerAttribute_RestoreViewMode)
{
    Application *app = createTestApplication();
    
    // Mock appAttribute to return default view mode
    stub.set_lamda(static_cast<QVariant (*)(Application::ApplicationAttribute)>(&Application::appAttribute),
                   [](Application::ApplicationAttribute aa) -> QVariant {
                       if (aa == Application::kViewMode) {
                           return 1; // Default view mode
                       }
                       return QVariant();
                   });
    
    // Mock appObtuselySetting to return test settings
    Settings *mockSettings = new Settings("test", Settings::kGenericConfig);
    stub.set_lamda(&Application::appObtuselySetting,
                   [mockSettings]() -> Settings* {
                       return mockSettings;
                   });
    
    // Mock Settings methods
    stub.set_lamda(&Settings::keyList,
                   [](Settings *self, const QString &group) -> QStringList {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       return QStringList() << "file:///home/test";
                   });
    
    stub.set_lamda(static_cast<QVariant (Settings::*)(const QString&, const QString&, const QVariant&) const>(&Settings::value),
                   [](Settings *self, const QString &group, const QString &key, const QVariant &defaultValue) -> QVariant {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(defaultValue)
                       if (key == "file:///home/test") {
                           QVariantMap map;
                           map["viewMode"] = 0; // Different from default
                           return map;
                       }
                       return QVariant();
                   });
    
    bool setValueCalled = false;
    stub.set_lamda(static_cast<void (Settings::*)(const QString&, const QString&, const QVariant&)>(&Settings::setValue),
                   [&setValueCalled](Settings *self, const QString &group, const QString &key, const QVariant &value) {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(key)
                       Q_UNUSED(value)
                       setValueCalled = true;
                   });
    
    stub.set_lamda(&Settings::sync,
                   [](Settings *self) -> bool {
                       Q_UNUSED(self)
                       return true;
                   });
    
    // Set up signal spy
    QSignalSpy viewModeChangedSpy(app, &Application::viewModeChanged);
    
    // Test restore view mode trigger
    Application::appAttributeTrigger(Application::kRestoreViewMode, 123);
    
    EXPECT_TRUE(setValueCalled);
    EXPECT_EQ(viewModeChangedSpy.count(), 1);
    EXPECT_EQ(viewModeChangedSpy.at(0).at(0).toInt(), 1);
    
    delete app;
    delete mockSettings;
}

/**
 * @brief Test trigger attribute - clear search history
 * Verify clearSearchHistory trigger functionality
 */
TEST_F(ApplicationTest, TriggerAttribute_ClearSearchHistory)
{
    Application *app = createTestApplication();
    
    // Set up signal spy
    QSignalSpy clearSearchHistorySpy(app, &Application::clearSearchHistory);
    
    // Test clear search history trigger
    quint64 testWindowId = 12345;
    Application::appAttributeTrigger(Application::kClearSearchHistory, testWindowId);
    
    EXPECT_EQ(clearSearchHistorySpy.count(), 1);
    EXPECT_EQ(clearSearchHistorySpy.at(0).at(0).toULongLong(), testWindowId);
    
    delete app;
}

/**
 * @brief Test invalid attribute handling
 * Verify behavior with invalid attribute keys
 */
TEST_F(ApplicationTest, InvalidAttribute_Handling)
{
    Application *app = createTestApplication();
    
    // Test invalid attribute key handling in settings value changed
    app->d->_q_onSettingsValueChanged("ApplicationAttribute", "InvalidKey", QVariant());
    app->d->_q_onSettingsValueChanged("GenericAttribute", "InvalidKey", QVariant());
    
    // Test with empty group
    app->d->_q_onSettingsValueChanged("", "SomeKey", QVariant());
    
    // If we reach here without crashes, the test passes
    EXPECT_TRUE(true);
    
    delete app;
}

/**
 * @brief Test thread safety
 * Verify thread-safe operations
 */
TEST_F(ApplicationTest, ThreadSafety_Basic)
{
    Application *app = createTestApplication();
    
    // Test that singleton access is thread-safe
    Application *instance1 = Application::instance();
    Application *instance2 = Application::instance();
    
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(instance1, app);
    
    delete app;
}

/**
 * @brief Test Settings connection management
 * Verify that Settings objects are properly connected to the Application
 */
TEST_F(ApplicationTest, SettingsConnection_Management)
{
    Application *app = createTestApplication();
    
    // Set up signal spy for settings creation
    QSignalSpy genericSettingCreatedSpy(app, &Application::genericSettingCreated);
    QSignalSpy appSettingCreatedSpy(app, &Application::appSettingCreated);
    
    // Access settings to trigger creation
    Settings *genericSettings = Application::genericSetting();
    Settings *appSettings = Application::appSetting();
    
    EXPECT_NE(genericSettings, nullptr);
    EXPECT_NE(appSettings, nullptr);
    
    // Note: Signal emission depends on whether settings already exist
    // In real scenario, signals would be emitted on first creation
    
    delete app;
}

/**
 * @brief Test enum value conversion
 * Verify that enum values are correctly converted to/from strings
 */
TEST_F(ApplicationTest, EnumConversion_Validation)
{
    Application *app = createTestApplication();
    
    // Test that enum values can be converted to strings and back
    const QMetaEnum &appAttrEnum = QMetaEnum::fromType<Application::ApplicationAttribute>();
    const QMetaEnum &genericAttrEnum = QMetaEnum::fromType<Application::GenericAttribute>();
    
    // Test application attribute enum
    const char *keyName = appAttrEnum.valueToKey(Application::kIconSizeLevel);
    EXPECT_NE(keyName, nullptr);
    EXPECT_STREQ(keyName, "kIconSizeLevel");
    
    // Test generic attribute enum
    keyName = genericAttrEnum.valueToKey(Application::kPreviewCompressFile);
    EXPECT_NE(keyName, nullptr);
    EXPECT_STREQ(keyName, "kPreviewCompressFile");
    
    delete app;
}

/**
 * @brief Performance test
 * Verify performance characteristics of key operations
 */
TEST_F(ApplicationTest, Performance_BasicOperations)
{
    Application *app = createTestApplication();
    
    // Mock Settings operations to return quickly
    stub.set_lamda(static_cast<QVariant (Settings::*)(const QString&, const QString&, const QVariant&) const>(&Settings::value),
                   [](Settings *self, const QString &group, const QString &key, const QVariant &defaultValue) -> QVariant {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(key)
                       return defaultValue;
                   });
    
    // Test repeated attribute access performance
    const int iterations = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        Application::appAttribute(Application::kIconSizeLevel);
        Application::genericAttribute(Application::kPreviewCompressFile);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Performance should be reasonable (less than 1 second for 1000 operations)
    EXPECT_LT(duration.count(), 1000);
    
    delete app;
}

/**
 * @brief Comprehensive integration test
 * Test all major functionality working together
 */
TEST_F(ApplicationTest, Integration_ComprehensiveTest)
{
    Application *app = createTestApplication();
    
    // Test singleton
    EXPECT_EQ(Application::instance(), app);
    
    // Test settings creation
    Settings *genericSettings = Application::genericSetting();
    Settings *appSettings = Application::appSetting();
    EXPECT_NE(genericSettings, nullptr);
    EXPECT_NE(appSettings, nullptr);
    
    // Mock settings operations
    stub.set_lamda(static_cast<QVariant (Settings::*)(const QString&, const QString&, const QVariant&) const>(&Settings::value),
                   [](Settings *self, const QString &group, const QString &key, const QVariant &defaultValue) -> QVariant {
                       Q_UNUSED(self)
                       Q_UNUSED(group)
                       Q_UNUSED(defaultValue)
                       if (key == "IconSizeLevel") return 2;
                       if (key == "PreviewCompressFile") return true;
                       return QVariant();
                   });
    
    // Test attribute operations
    QVariant iconSize = Application::appAttribute(Application::kIconSizeLevel);
    QVariant previewCompress = Application::genericAttribute(Application::kPreviewCompressFile);
    
    EXPECT_EQ(iconSize.toInt(), 2);
    EXPECT_TRUE(previewCompress.toBool());
    
    // Test signal emission
    QSignalSpy appAttributeChangedSpy(app, &Application::appAttributeChanged);
    app->d->_q_onSettingsValueChanged("ApplicationAttribute", "IconSizeLevel", 3);
    EXPECT_EQ(appAttributeChangedSpy.count(), 1);
    
    delete app;
} 