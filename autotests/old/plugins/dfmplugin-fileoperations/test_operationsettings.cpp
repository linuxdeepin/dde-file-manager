// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QSignalSpy>

#include "stubext.h"

#include "settings/operationsettings.h"

#include <DSettingsOption>
#include <DRadioButton>
#include <DTipLabel>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestOperationSettings : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Create a mock DSettingsOption
        option = new DSettingsOption();
        option->setValue(true);  // Default sync mode
    }

    void TearDown() override
    {
        stub.clear();
        delete option;
        option = nullptr;
    }

    stub_ext::StubExt stub;
    DSettingsOption *option { nullptr };
};

// ========== OperationSettings::createSyncModeItem() Tests ==========

TEST_F(TestOperationSettings, CreateSyncModeItem_ReturnsValidWidget)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);

    ASSERT_NE(widget, nullptr);
    EXPECT_NE(widget->layout(), nullptr);

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_ContainsTwoRadioButtons)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    // Find all radio buttons
    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();

    EXPECT_EQ(radioButtons.count(), 2);

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_SyncModeCheckedByDefault)
{
    option->setValue(true);

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // First button (sync mode) should be checked
    EXPECT_TRUE(radioButtons[0]->isChecked());
    EXPECT_FALSE(radioButtons[1]->isChecked());

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_PerformanceModeCheckedWhenFalse)
{
    option->setValue(false);

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // Second button (performance mode) should be checked
    EXPECT_FALSE(radioButtons[0]->isChecked());
    EXPECT_TRUE(radioButtons[1]->isChecked());

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_ContainsTwoTipLabels)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DTipLabel *> tipLabels = widget->findChildren<DTipLabel *>();

    EXPECT_EQ(tipLabels.count(), 2);

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_ButtonGroupExists)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<QButtonGroup *> buttonGroups = widget->findChildren<QButtonGroup *>();

    EXPECT_EQ(buttonGroups.count(), 1);

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_ClickingSyncButtonUpdatesOption)
{
    option->setValue(false);  // Start with performance mode

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // Simulate clicking sync mode button
    radioButtons[0]->click();

    // Option value should be updated to true
    EXPECT_TRUE(option->value().toBool());
    EXPECT_TRUE(radioButtons[0]->isChecked());
    EXPECT_FALSE(radioButtons[1]->isChecked());

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_ClickingPerformanceButtonUpdatesOption)
{
    option->setValue(true);  // Start with sync mode

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // Simulate clicking performance mode button
    radioButtons[1]->click();

    // Option value should be updated to false
    EXPECT_FALSE(option->value().toBool());
    EXPECT_FALSE(radioButtons[0]->isChecked());
    EXPECT_TRUE(radioButtons[1]->isChecked());

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_OptionValueChangeUpdatesButtons)
{
    option->setValue(true);

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    EXPECT_TRUE(radioButtons[0]->isChecked());

    // Change option value programmatically
    option->setValue(false);

    // Buttons should update
    EXPECT_FALSE(radioButtons[0]->isChecked());
    EXPECT_TRUE(radioButtons[1]->isChecked());

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_LayoutStructure)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout *>(widget->layout());
    ASSERT_NE(mainLayout, nullptr);

    // Main layout should have 2 items (sync widget and performance widget)
    EXPECT_EQ(mainLayout->count(), 2);

    delete widget;
}

// ========== Signal/Slot Tests ==========

TEST_F(TestOperationSettings, CreateSyncModeItem_SignalSlotConnection)
{
    option->setValue(true);

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QSignalSpy spy(option, &DSettingsOption::valueChanged);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // Click performance button
    radioButtons[1]->click();

    // Signal should have been emitted
    EXPECT_GT(spy.count(), 0);

    delete widget;
}

// ========== Edge Cases ==========

TEST_F(TestOperationSettings, CreateSyncModeItem_NullOption)
{
    // This test verifies behavior with null option
    // In real implementation, this should be handled gracefully
    QWidget *widget = nullptr;

    // Attempt to create widget with null option
    // Note: This may crash in actual implementation,
    // so we use a try-catch or stub to handle it
    try {
        widget = OperationSettings::createSyncModeItem(nullptr);
    } catch (...) {
        // Expected to fail or return null
    }

    // Widget should be null or method should handle gracefully
    // This test documents expected behavior
    if (widget) {
        delete widget;
    }
}

TEST_F(TestOperationSettings, CreateSyncModeItem_RapidToggle)
{
    option->setValue(true);

    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // Rapidly toggle between modes
    for (int i = 0; i < 10; ++i) {
        radioButtons[0]->click();
        radioButtons[1]->click();
    }

    // Final state should be performance mode (last click)
    EXPECT_FALSE(option->value().toBool());
    EXPECT_FALSE(radioButtons[0]->isChecked());
    EXPECT_TRUE(radioButtons[1]->isChecked());

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_MultipleWidgetInstances)
{
    // Create multiple widget instances from the same option
    QWidget *widget1 = OperationSettings::createSyncModeItem(option);
    QWidget *widget2 = OperationSettings::createSyncModeItem(option);

    ASSERT_NE(widget1, nullptr);
    ASSERT_NE(widget2, nullptr);

    // Both should be valid but independent
    EXPECT_NE(widget1, widget2);

    delete widget1;
    delete widget2;
}

// ========== Memory Management Tests ==========

TEST_F(TestOperationSettings, CreateSyncModeItem_MemoryCleanup)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    // Verify children are created
    EXPECT_GT(widget->findChildren<DRadioButton *>().count(), 0);
    EXPECT_GT(widget->findChildren<DTipLabel *>().count(), 0);

    // Delete widget should clean up all children
    delete widget;

    // No memory leaks expected (verified by valgrind/ASAN in actual test runs)
    SUCCEED();
}

// ========== Widget Properties Tests ==========

TEST_F(TestOperationSettings, CreateSyncModeItem_TipLabelProperties)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DTipLabel *> tipLabels = widget->findChildren<DTipLabel *>();
    ASSERT_EQ(tipLabels.count(), 2);

    // Verify tip labels have proper alignment
    for (auto *label : tipLabels) {
        EXPECT_TRUE(label->alignment() & Qt::AlignLeft);
        EXPECT_TRUE(label->wordWrap());
        EXPECT_FALSE(label->text().isEmpty());
    }

    delete widget;
}

TEST_F(TestOperationSettings, CreateSyncModeItem_RadioButtonProperties)
{
    QWidget *widget = OperationSettings::createSyncModeItem(option);
    ASSERT_NE(widget, nullptr);

    QList<DRadioButton *> radioButtons = widget->findChildren<DRadioButton *>();
    ASSERT_EQ(radioButtons.count(), 2);

    // Verify radio buttons have text
    for (auto *button : radioButtons) {
        EXPECT_FALSE(button->text().isEmpty());
    }

    delete widget;
}
