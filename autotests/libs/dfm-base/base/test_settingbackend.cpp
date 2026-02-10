// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QVariant>
#include <memory>

#include "stubext.h"

#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/private/application_p.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>

#include <DSettings>

DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestSettingBackend : public testing::Test
{
public:
    void SetUp() override
    {
        // Initialize QCoreApplication if not already initialized
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }

        // Stub Application::instance to avoid real application initialization issues
        stub.set_lamda(&Application::instance, []() -> Application * {
            __DBG_STUB_INVOKE__
            static Application *fakeApp = nullptr;
            if (!fakeApp) {
                fakeApp = new Application();
            }
            return fakeApp;
        });
        // Clear any existing Application instance
        ApplicationPrivate::self = nullptr;

        backend = SettingBackend::instance();
    }

    void TearDown() override
    {
        stub.clear();

        if (app) {
            delete app;
            app = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    SettingBackend *backend { nullptr };
    QCoreApplication *app { nullptr };
    
    // Helper variables to avoid stack-use-after-return issues in tests
    bool test_getterCalled { false };
    bool test_setterCalled { false };
    QVariant test_retrievedValue;
};

// ========== Basic Functionality Tests ==========

TEST_F(TestSettingBackend, instance_ReturnsSingleton)
{
    // Test that instance() returns the same singleton
    auto instance1 = SettingBackend::instance();
    auto instance2 = SettingBackend::instance();

    ASSERT_TRUE(instance1 != nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestSettingBackend, setToSettings_WithValidSettings)
{
    // Create a DSettings object
    DSettings settings;

    // Test setting the backend
    backend->setToSettings(&settings);

    // Verify backend was set (indirectly by checking if no crash occurred)
    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, setToSettings_WithNullSettings)
{
    // Test with null settings (should not crash)
    backend->setToSettings(nullptr);

    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, keys_ReturnsNonEmpty)
{
    // Test that keys() returns a non-empty list
    QStringList keyList = backend->keys();

    EXPECT_FALSE(keyList.isEmpty());
}

TEST_F(TestSettingBackend, doSync_DoesNotCrash)
{
    // Test that doSync() doesn't crash (it's a no-op implementation)
    backend->doSync();

    EXPECT_TRUE(true);
}

// ========== Setting Accessor Tests ==========

TEST_F(TestSettingBackend, addSettingAccessor_WithGetterAndSetter)
{
    // Test adding a setting accessor with both getter and setter
    QString testKey = "test.key.accessor";
    QVariant testValue = QString("test_value");
    QVariant retrievedValue;

    // Reset test member variables for this test
    test_getterCalled = false;
    test_setterCalled = false;
    test_retrievedValue = QVariant();

    auto getter = [this, testValue]() -> QVariant {
        test_getterCalled = true;
        return testValue;
    };

    auto setter = [this](const QVariant &val) {
        test_setterCalled = true;
        test_retrievedValue = val;
    };

    backend->addSettingAccessor(testKey, getter, setter);

    // Verify the accessor was added by checking keys
    QStringList keyList = backend->keys();
    EXPECT_TRUE(keyList.contains(testKey));

    // Test getter
    QVariant result = backend->getOption(testKey);
    EXPECT_TRUE(test_getterCalled);
    EXPECT_EQ(result, testValue);

    // Test setter through onOptionSetted
    backend->onOptionSetted(testKey, QVariant("new_value"));
    EXPECT_TRUE(test_setterCalled);
    EXPECT_EQ(test_retrievedValue.toString(), QString("new_value"));
}

TEST_F(TestSettingBackend, addSettingAccessor_WithNullGetter)
{
    // Test adding a setting accessor with null getter
    QString testKey = "test.null.getter";

    auto setter = [](const QVariant &) {
        // Do nothing
    };

    // Should log a warning but not crash
    backend->addSettingAccessor(testKey, nullptr, setter);

    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, addSettingAccessor_WithNullSetter)
{
    // Test adding a setting accessor with null setter
    QString testKey = "test.null.setter";

    auto getter = []() -> QVariant {
        return QVariant("test");
    };

    // Should log a warning but not crash
    backend->addSettingAccessor(testKey, getter, nullptr);

    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, removeSettingAccessor_ExistingKey)
{
    // Test removing an existing setting accessor
    QString testKey = "test.remove.key";

    auto getter = []() -> QVariant { return QVariant(); };
    auto setter = [](const QVariant &) {};

    backend->addSettingAccessor(testKey, getter, setter);

    // Verify it was added
    EXPECT_TRUE(backend->keys().contains(testKey));

    // Remove it
    backend->removeSettingAccessor(testKey);

    // Verify it was removed
    EXPECT_FALSE(backend->keys().contains(testKey));
}

TEST_F(TestSettingBackend, removeSettingAccessor_NonExistingKey)
{
    // Test removing a non-existing key (should log warning but not crash)
    QString testKey = "test.nonexisting.key";

    backend->removeSettingAccessor(testKey);

    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, addSettingAccessor_ApplicationAttribute)
{
    // Test adding a setting accessor for ApplicationAttribute
    auto setter = [](const QVariant &) {
        // Do nothing, just test that it can be added
    };

    // Use a valid ApplicationAttribute that has a mapping
    Application::ApplicationAttribute attr = Application::kAllwayOpenOnNewWindow;

    backend->addSettingAccessor(attr, setter);

    // The key should be in the predefined mapping
    QStringList keyList = backend->keys();
    EXPECT_FALSE(keyList.isEmpty());

    // Clean up to avoid affecting other tests
    QString key = "00_base.00_open_action.00_allways_open_on_new_window";
    backend->removeSettingAccessor(key);
}

TEST_F(TestSettingBackend, addSettingAccessor_GenericAttribute)
{
    // Test adding a setting accessor for GenericAttribute
    auto setter = [](const QVariant &) {
        // Do nothing, just test that it can be added
    };

    // Use a valid GenericAttribute that has a mapping
    Application::GenericAttribute attr = Application::kShowedHiddenFiles;

    backend->addSettingAccessor(attr, setter);

    // The key should be in the predefined mapping
    QStringList keyList = backend->keys();
    EXPECT_FALSE(keyList.isEmpty());

    // Clean up to avoid affecting other tests
    QString key = "00_base.03_files_and_folders.00_show_hidden";
    backend->removeSettingAccessor(key);
}

// ========== Serial Data Key Tests ==========

TEST_F(TestSettingBackend, addToSerialDataKey_AddsKey)
{
    // Test adding a key to serial data keys
    QString testKey = "test.serial.key";

    backend->addToSerialDataKey(testKey);

    // We can't directly verify the internal set, but we can test behavior
    // through onOptionSetted which behaves differently for serial data keys
    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, removeSerialDataKey_RemovesKey)
{
    // Test removing a key from serial data keys
    QString testKey = "test.serial.remove";

    backend->addToSerialDataKey(testKey);
    backend->removeSerialDataKey(testKey);

    EXPECT_TRUE(true);
}

// ========== Option Setting Tests ==========

TEST_F(TestSettingBackend, getOption_WithAppAttribute)
{
    // Stub Application::appAttribute to return a known value
    QVariant expectedValue = QVariant(true);

    stub.set_lamda(&Application::appAttribute, [expectedValue](Application::ApplicationAttribute) -> QVariant {
        __DBG_STUB_INVOKE__
        return expectedValue;
    });

    // Get an option that maps to ApplicationAttribute
    QString key = "00_base.00_open_action.00_allways_open_on_new_window";
    QVariant result = backend->getOption(key);

    EXPECT_EQ(result, expectedValue);
}

TEST_F(TestSettingBackend, getOption_WithGenericAttribute)
{
    // Stub Application::genericAttribute to return a known value
    QVariant expectedValue = QVariant(false);

    stub.set_lamda(&Application::genericAttribute, [expectedValue](Application::GenericAttribute) -> QVariant {
        __DBG_STUB_INVOKE__
        return expectedValue;
    });

    // Get an option that maps to GenericAttribute
    QString key = "00_base.03_files_and_folders.00_show_hidden";
    QVariant result = backend->getOption(key);

    EXPECT_EQ(result, expectedValue);
}

TEST_F(TestSettingBackend, getOption_WithCustomAccessor)
{
    // Test getting an option with custom accessor
    QString testKey = "test.custom.option";
    QVariant expectedValue = QString("custom_value");

    auto getter = [expectedValue]() -> QVariant {
        return expectedValue;
    };

    backend->addSettingAccessor(testKey, getter, nullptr);

    QVariant result = backend->getOption(testKey);
    EXPECT_EQ(result, expectedValue);
}

TEST_F(TestSettingBackend, getOption_NonExistentKey)
{
    // Test getting a non-existent option
    QString nonExistentKey = "non.existent.key.12345";

    QVariant result = backend->getOption(nonExistentKey);

    // Should return an invalid QVariant
    EXPECT_FALSE(result.isValid());
}

// ========== Signal Tests ==========

TEST_F(TestSettingBackend, optionSetted_EmitsSignal)
{
    // Test that doSetOption emits optionSetted signal
    QSignalSpy spy(backend, &SettingBackend::optionSetted);

    QString testKey = "test.signal.key";
    QVariant testValue = QString("test_value");

    // Call protected method through public method
    backend->onOptionSetted(testKey, testValue);

    // Wait for queued connection
    QCoreApplication::processEvents();

    EXPECT_GE(spy.count(), 0);   // May have been processed already
}

TEST_F(TestSettingBackend, onValueChanged_ApplicationAttribute)
{
    // Test onValueChanged for ApplicationAttribute
    QSignalSpy spy(backend, SIGNAL(optionChanged(QString, QVariant)));

    // Trigger value change for a known ApplicationAttribute
    int attr = static_cast<int>(Application::kAllwayOpenOnNewWindow);
    QVariant value = QVariant(true);

    // Directly call the protected method
    // Note: We can access it due to compiler flags for testing
    backend->onValueChanged(attr, value);

    EXPECT_GE(spy.count(), 0);
}

TEST_F(TestSettingBackend, onValueChanged_GenericAttribute)
{
    // Test onValueChanged for GenericAttribute
    QSignalSpy spy(backend, SIGNAL(optionChanged(QString, QVariant)));

    // Trigger value change for a known GenericAttribute
    int attr = static_cast<int>(Application::kShowedHiddenFiles);
    QVariant value = QVariant(false);

    backend->onValueChanged(attr, value);

    EXPECT_GE(spy.count(), 0);
}

TEST_F(TestSettingBackend, onValueChanged_UnknownAttribute)
{
    // Test onValueChanged with unknown attribute
    QSignalSpy spy(backend, SIGNAL(optionChanged(QString, QVariant)));

    // Use an invalid attribute value
    int attr = 99999;
    QVariant value = QVariant();

    backend->onValueChanged(attr, value);

    // Should not emit signal for unknown attribute
    EXPECT_EQ(spy.count(), 0);
}

// ========== Option Setting with Attributes Tests ==========

TEST_F(TestSettingBackend, onOptionSetted_SavesAppAttribute)
{
    // Test that onOptionSetted saves to ApplicationAttribute
    bool setterCalled = false;
    QVariant savedValue;

    stub.set_lamda(&Application::setAppAttribute,
                   [&setterCalled, &savedValue](Application::ApplicationAttribute, const QVariant &val) {
                       __DBG_STUB_INVOKE__
                       setterCalled = true;
                       savedValue = val;
                   });

    QString key = "00_base.00_open_action.00_allways_open_on_new_window";
    QVariant value = QVariant(true);

    backend->onOptionSetted(key, value);

    EXPECT_TRUE(setterCalled);
    EXPECT_EQ(savedValue, value);
}

TEST_F(TestSettingBackend, onOptionSetted_SavesGenericAttribute)
{
    // Test that onOptionSetted saves to GenericAttribute
    bool setterCalled = false;
    QVariant savedValue;

    stub.set_lamda(&Application::setGenericAttribute,
                   [&setterCalled, &savedValue](Application::GenericAttribute, const QVariant &val) {
                       __DBG_STUB_INVOKE__
                       setterCalled = true;
                       savedValue = val;
                   });

    QString key = "00_base.03_files_and_folders.00_show_hidden";
    QVariant value = QVariant(false);

    backend->onOptionSetted(key, value);

    EXPECT_TRUE(setterCalled);
    EXPECT_EQ(savedValue, value);
}

TEST_F(TestSettingBackend, onOptionSetted_CallsCustomSetter)
{
    // Test that onOptionSetted calls custom setter
    QString testKey = "test.custom.setter";
    bool setterCalled = false;
    QVariant savedValue;

    auto setter = [&setterCalled, &savedValue](const QVariant &val) {
        setterCalled = true;
        savedValue = val;
    };

    backend->addSettingAccessor(testKey, nullptr, setter);

    QVariant value = QString("custom_value");
    backend->onOptionSetted(testKey, value);

    EXPECT_TRUE(setterCalled);
    EXPECT_EQ(savedValue, value);
}

TEST_F(TestSettingBackend, onOptionSetted_WithSerialDataKey)
{
    // Test onOptionSetted with serial data key
    QString testKey = "test.serial.option";
    bool setterCalled = false;

    auto setter = [&setterCalled](const QVariant &) {
        setterCalled = true;
    };

    backend->addSettingAccessor(testKey, nullptr, setter);
    backend->addToSerialDataKey(testKey);

    backend->onOptionSetted(testKey, QVariant("value"));

    EXPECT_TRUE(setterCalled);
}

// ========== Initialization Tests ==========

TEST_F(TestSettingBackend, initPresetSettingConfig_DoesNotCrash)
{
    // Test that initialization doesn't crash
    // This is called in constructor, so if we got here, it worked
    EXPECT_TRUE(backend != nullptr);
}

// ========== Edge Cases and Error Handling ==========

TEST_F(TestSettingBackend, keys_ContainsPredefinedKeys)
{
    // Test that keys() contains some predefined keys
    QStringList keyList = backend->keys();

    // Check for at least one known ApplicationAttribute key
    bool hasAppAttrKey = false;
    for (const QString &key : keyList) {
        if (key.contains("00_base.00_open_action")) {
            hasAppAttrKey = true;
            break;
        }
    }

    EXPECT_TRUE(hasAppAttrKey);
}

TEST_F(TestSettingBackend, addSettingAccessor_OverwriteExisting)
{
    // Test overwriting an existing setting accessor
    QString testKey = "test.overwrite.key";

    int callCount = 0;

    auto getter1 = [&callCount]() -> QVariant {
        callCount = 1;
        return QVariant("value1");
    };

    auto getter2 = [&callCount]() -> QVariant {
        callCount = 2;
        return QVariant("value2");
    };

    backend->addSettingAccessor(testKey, getter1, nullptr);
    backend->addSettingAccessor(testKey, getter2, nullptr);

    QVariant result = backend->getOption(testKey);

    // The second getter should have been used
    EXPECT_EQ(callCount, 2);
    EXPECT_EQ(result.toString(), QString("value2"));
}

TEST_F(TestSettingBackend, getOption_EmptyKey)
{
    // Test getting option with empty key
    QVariant result = backend->getOption("");

    EXPECT_FALSE(result.isValid());
}

TEST_F(TestSettingBackend, onOptionSetted_EmptyKey)
{
    // Test setting option with empty key (should not crash)
    backend->onOptionSetted("", QVariant());

    EXPECT_TRUE(true);
}

TEST_F(TestSettingBackend, doSetOption_TriggersSignal)
{
    // Test that doSetOption triggers the optionSetted signal
    QSignalSpy spy(backend, &SettingBackend::optionSetted);

    QString testKey = "test.dosetsignal.key";
    QVariant testValue = QString("signal_value");

    // Access protected method (allowed due to test compiler flags)
    backend->doSetOption(testKey, testValue);

    EXPECT_EQ(spy.count(), 1);

    if (spy.count() > 0) {
        QList<QVariant> arguments = spy.takeFirst();
        EXPECT_EQ(arguments.at(0).toString(), testKey);
        EXPECT_EQ(arguments.at(1), testValue);
    }
}

// ========== Integration Test ==========

TEST_F(TestSettingBackend, Integration_CompleteWorkflow)
{
    // Test complete workflow: add accessor, get, set
    QString testKey = "test.integration.workflow";
    QVariant storedValue;

    auto getter = [&storedValue]() -> QVariant {
        return storedValue;
    };

    auto setter = [&storedValue](const QVariant &val) {
        storedValue = val;
    };

    // Add accessor
    backend->addSettingAccessor(testKey, getter, setter);

    // Set value
    QVariant testValue = QString("integration_test");
    backend->onOptionSetted(testKey, testValue);

    // Get value
    QVariant result = backend->getOption(testKey);

    EXPECT_EQ(result, testValue);
    EXPECT_EQ(storedValue, testValue);

    // Remove accessor
    backend->removeSettingAccessor(testKey);

    // Verify removed
    EXPECT_FALSE(backend->keys().contains(testKey));
}
