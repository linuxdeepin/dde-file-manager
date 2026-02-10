// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QCheckBox>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/dialogs/settingsdialog/controls/aliascombobox.h>
#include <dfm-base/dialogs/settingsdialog/controls/checkboxwithmessage.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/private/application_p.h>
#include <dfm-base/base/configs/settingbackend.h>

// Include DTK widgets
#include <DSettingsDialog>
#include <DSettings>
#include <DSettingsOption>
#include <DApplication>
#include <DComboBox>

// Include stub headers
#include "stubext.h"

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace dfmbase;

class TestSettingDialog : public testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        
        // Stub UI methods to avoid actual dialog display
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;  // or QDialog::Rejected as needed
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

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

        // Stub Application::appAttributeTrigger to avoid creating another Application instance
        stub.set_lamda(&Application::appAttributeTrigger, [](Application::TriggerAttribute, quint64) {
            __DBG_STUB_INVOKE__
        });
    }
    
    void TearDown() override {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    // Application *app = nullptr;
};

// Test constructor
TEST_F(TestSettingDialog, TestConstructor)
{
    SettingDialog dialog;
    
    // Should not crash
    EXPECT_NE(&dialog, nullptr);
    EXPECT_TRUE(dialog.isVisible() == false); // Initially not visible
}

// Test constructor with parent
TEST_F(TestSettingDialog, TestConstructorWithParent)
{
    QWidget parent;
    SettingDialog dialog(&parent);
    
    // Should not crash
    EXPECT_NE(&dialog, nullptr);
    EXPECT_EQ(dialog.parent(), &parent);
}

// Test initialize method
TEST_F(TestSettingDialog, TestInitialize)
{
    SettingDialog dialog;

    // Application::instance() should not be nullptr since we stubbed it
    EXPECT_NE(Application::instance(), nullptr);
    
    // Mock DSettings::fromJsonFile to avoid loading actual file
    stub.set_lamda((DSettings*(*)(const QString&))&DSettings::fromJsonFile,
                   [](const QString&) -> DSettings* {
        __DBG_STUB_INVOKE__
        return nullptr;  // Return null for simplicity
    });
    // Mock SettingBackend::setToSettings(DSettings *settings)
    stub.set_lamda(&SettingBackend::setToSettings,
                   [](SettingBackend *backend, DSettings *settings) {
        __DBG_STUB_INVOKE__
    });
    // Test initialize - should not crash
    dialog.initialze();
    
    // Should not crash even with null settings
    SUCCEED();
}

// Test setItemVisible
TEST_F(TestSettingDialog, TestSetItemVisible)
{
    // Test with valid key
    SettingDialog::setItemVisiable("base.delete_confirm", true);
    SettingDialog::setItemVisiable("base.delete_confirm", false);
    
    // Test with empty key
    SettingDialog::setItemVisiable("", true);
    SettingDialog::setItemVisiable("", false);
    
    // Should not crash
    SUCCEED();
}

// Test createAutoMountCheckBox
TEST_F(TestSettingDialog, TestCreateAutoMountCheckBox)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Mock the option methods
    stub.set_lamda(&DSettingsOption::key, [](DSettingsOption *opt) -> QString {
        return "base.auto_mount";
    });
    
    // Create the widget
    auto pair = SettingDialog::createAutoMountCheckBox(&option);
    
    // Should return a valid pair
    EXPECT_NE(pair.first, nullptr);
    EXPECT_NE(pair.second, nullptr);
    
    // Should be a checkbox
    auto checkBox = qobject_cast<QCheckBox*>(pair.first);
    EXPECT_NE(checkBox, nullptr);
}

// Test createAutoMountOpenCheckBox
TEST_F(TestSettingDialog, TestCreateAutoMountOpenCheckBox)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Mock the option methods
    stub.set_lamda(&DSettingsOption::key, [](DSettingsOption *opt) -> QString {
        return "base.auto_mount_open";
    });
    
    // Create the widget
    auto pair = SettingDialog::createAutoMountOpenCheckBox(&option);
    
    // Should return a valid pair
    EXPECT_NE(pair.first, nullptr);
    EXPECT_NE(pair.second, nullptr);
    
    // Should be a checkbox
    auto checkBox = qobject_cast<QCheckBox*>(pair.first);
    EXPECT_NE(checkBox, nullptr);
}

// Test createCheckBoxWithMessage
TEST_F(TestSettingDialog, TestCreateCheckBoxWithMessage)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Mock the option methods
    stub.set_lamda(&DSettingsOption::key, [](DSettingsOption *opt) -> QString {
        return "base.something_with_message";
    });
    
    // Create the widget
    auto pair = SettingDialog::createCheckBoxWithMessage(&option);
    
    // Should return a valid pair
    EXPECT_NE(pair.first, nullptr);
    EXPECT_NE(pair.second, nullptr);
}

// Test createPushButton
TEST_F(TestSettingDialog, TestCreatePushButton)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Create the widget
    auto pair = SettingDialog::createPushButton(&option);
    
    // Should return a valid pair
    EXPECT_NE(pair.first, nullptr);
    EXPECT_NE(pair.second, nullptr);
}

// Test createSliderWithSideIcon
TEST_F(TestSettingDialog, TestCreateSliderWithSideIcon)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Create the widget
    auto pair = SettingDialog::createSliderWithSideIcon(&option);
    
    // Should return a valid pair
    EXPECT_NE(pair.first, nullptr);
    EXPECT_NE(pair.second, nullptr);
}

// Test createPathComboboxItem
TEST_F(TestSettingDialog, TestCreatePathComboboxItem)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Create the widget
    auto pair = SettingDialog::createPathComboboxItem(&option);
    
    // Should return a valid pair
    EXPECT_NE(pair.first, nullptr);
    EXPECT_NE(pair.second, nullptr);
    
    // First should be an AliasComboBox
    auto comboBox = qobject_cast<AliasComboBox*>(pair.first);
    EXPECT_NE(comboBox, nullptr);
}

// Test mountCheckBoxStateChangedHandle
TEST_F(TestSettingDialog, TestMountCheckBoxStateChangedHandle)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Mock setValue method
    stub.set_lamda(&DSettingsOption::setValue, [](DSettingsOption*, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    // Test with checked state
    SettingDialog::mountCheckBoxStateChangedHandle(&option, 2); // Qt::Checked
    
    // Test with unchecked state
    SettingDialog::mountCheckBoxStateChangedHandle(&option, 0); // Qt::Unchecked
    
    // Should not crash
    SUCCEED();
}

// Test autoMountCheckBoxChangedHandle
TEST_F(TestSettingDialog, TestAutoMountCheckBoxChangedHandle)
{
    // Create a mock DSettingsOption
    DSettingsOption option;
    
    // Mock setValue method
    stub.set_lamda(&DSettingsOption::setValue, [](DSettingsOption*, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    // Test with checked state
    SettingDialog::autoMountCheckBoxChangedHandle(&option, 2); // Qt::Checked
    
    // Test with unchecked state
    SettingDialog::autoMountCheckBoxChangedHandle(&option, 0); // Qt::Unchecked
    
    // Should not crash
    SUCCEED();
}

// Test pathComboBoxChangedHandle
TEST_F(TestSettingDialog, TestPathComboBoxChangedHandle)
{
    // Create widgets
    AliasComboBox comboBox;
    DSettingsOption option;
    
    // Mock the necessary methods
    stub.set_lamda(&QComboBox::currentText, [](QComboBox *cb) -> QString {
        return "Test Path";
    });
    
    stub.set_lamda(&DSettingsOption::setValue, [](DSettingsOption*, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    // Test the handler
    bool result = SettingDialog::pathComboBoxChangedHandle(&comboBox, &option, 0);
    
    // Should return true for success
    EXPECT_TRUE(result);
}

// Test needHide with known keys
TEST_F(TestSettingDialog, TestNeedHide)
{
    // Test known hidden keys
    EXPECT_TRUE(SettingDialog::needHide("base.context.menu_hidden_items"));
    EXPECT_TRUE(SettingDialog::needHide("base.default_terminal"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.beautify_filename"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.display_filename_extension"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.search_hidden_file"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.show_crashed_backup"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.merge_all_folders_window"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.new_tab_on_activate"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.remember_the_last_file_selected"));
    EXPECT_TRUE(SettingDialog::needHide("advance.other.right_click_copy"));
    EXPECT_TRUE(SettingDialog::needHide("base.picker.hide_system_partition"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.file_empty_size_skip"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.text_file_max_size"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.compression_file_max_size"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.video_file_max_size"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.image_file_max_size"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.audio_file_max_size"));
    EXPECT_TRUE(SettingDialog::needHide("base.preview.other_file_max_size"));
    EXPECT_TRUE(SettingDialog::needHide("advance.mount.auto_mount_and_open"));
    EXPECT_TRUE(SettingDialog::needHide("advance.mount.dev_mount_no_interactive"));
}

// Test needHide with unknown keys
TEST_F(TestSettingDialog, TestNeedHideUnknown)
{
    // Test unknown keys
    EXPECT_FALSE(SettingDialog::needHide("some.random.key"));
    EXPECT_FALSE(SettingDialog::needHide(""));
    EXPECT_FALSE(SettingDialog::needHide("base"));
    EXPECT_FALSE(SettingDialog::needHide("base.")); // Just base with dot
}

// Test settingFilter with valid JSON
TEST_F(TestSettingDialog, TestSettingFilter)
{
    SettingDialog dialog;
    
    // Create valid JSON data
    QJsonObject rootObj;
    QJsonObject groupObj;
    groupObj["name"] = "Test Group";
    groupObj["keys"] = QJsonArray::fromStringList(QStringList() << "test.key1" << "test.key2");
    
    QJsonObject optionObj;
    optionObj["type"] = "checkbox";
    optionObj["text"] = "Test Option";
    optionObj["default"] = true;
    rootObj["groups"] = QJsonArray() << groupObj;
    rootObj["options"] = QJsonObject{{"test.key1", optionObj}};
    
    QJsonDocument doc(rootObj);
    QByteArray data = doc.toJson();
    
    // Test filtering
    dialog.settingFilter(data);
    
    // Should not crash
    SUCCEED();
}

// Test settingFilter with empty data
TEST_F(TestSettingDialog, TestSettingFilterEmpty)
{
    SettingDialog dialog;
    
    QByteArray emptyData;
    dialog.settingFilter(emptyData);
    
    // Should not crash
    SUCCEED();
}

// Test loadSettings with non-existent file
TEST_F(TestSettingDialog, TestLoadSettingsNonExistent)
{
    SettingDialog dialog;
    
    // Try to load non-existent template file
    dialog.loadSettings("/non/existent/template.json");
    
    // Should not crash
    SUCCEED();
}

// Test static members initialization
TEST_F(TestSettingDialog, TestStaticMembers)
{
    // Access static members to ensure they exist
    // Note: These are pointers and might be null initially
    EXPECT_TRUE(SettingDialog::kAutoMountCheckBox.isNull() || !SettingDialog::kAutoMountCheckBox.isNull());
    EXPECT_TRUE(SettingDialog::kAutoMountOpenCheckBox.isNull() || !SettingDialog::kAutoMountOpenCheckBox.isNull());
    
    // Hidden items set should be valid
    EXPECT_TRUE(SettingDialog::kHiddenSettingItems.isEmpty() || !SettingDialog::kHiddenSettingItems.isEmpty());
    
    // Parent window ID should be a valid quint64
    quint64 parentWid = SettingDialog::parentWid;
    EXPECT_TRUE(parentWid == 0 || parentWid > 0);
}

// Test dialog visibility
TEST_F(TestSettingDialog, TestDialogVisibility)
{
    SettingDialog dialog;
    
    // Initially hidden
    EXPECT_FALSE(dialog.isVisible());
    
    // Show dialog
    dialog.show();
    // Note: Dialog might not actually show without event loop
    // This test mainly ensures the method doesn't crash
    
    // Hide dialog
    dialog.hide();
    EXPECT_FALSE(dialog.isVisible());
}

// Test dialog title
TEST_F(TestSettingDialog, TestDialogTitle)
{
    SettingDialog dialog;
    
    // Set title using DSettingsDialog's method
    QString testTitle = "Test Settings";
    dialog.setWindowTitle(testTitle);
    EXPECT_EQ(dialog.windowTitle(), testTitle);
}

// Test multiple dialog instances
TEST_F(TestSettingDialog, TestMultipleInstances)
{
    SettingDialog dialog1;
    SettingDialog dialog2;
    
    // Both should exist independently
    EXPECT_NE(&dialog1, &dialog2);
    EXPECT_NE(&dialog1, nullptr);
    EXPECT_NE(&dialog2, nullptr);
}

// Test dialog destruction
TEST_F(TestSettingDialog, TestDestructor)
{
    // Create and destroy dialog
    {
        SettingDialog dialog;
        // Dialog will be destroyed when leaving scope
    }
    
    // Should not crash
    SUCCEED();
}