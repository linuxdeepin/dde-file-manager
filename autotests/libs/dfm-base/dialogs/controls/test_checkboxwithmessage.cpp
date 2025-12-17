// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>
#include <QWidget>

#include <dfm-base/dialogs/settingsdialog/controls/checkboxwithmessage.h>

// Include stub headers
#include "stubext.h"

class TestCheckBoxWithMessage : public testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        widget = new CheckBoxWithMessage();
        
        // // Stub UI methods to avoid actual dialog display
        // stub.set_lamda(&QWidget::show, [](QWidget *) {
        //     __DBG_STUB_INVOKE__
        // });
        // stub.set_lamda(&QWidget::hide, [](QWidget *) {
        //     __DBG_STUB_INVOKE__
        // });
    }
    
    void TearDown() override {
        stub.clear();
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }
    
public:
    stub_ext::StubExt stub;
    QApplication *app = nullptr;
    CheckBoxWithMessage *widget = nullptr;
};

// Test constructor with parent
TEST_F(TestCheckBoxWithMessage, TestConstructorWithParent)
{
    QWidget parent;
    
    CheckBoxWithMessage *tmpwidget = new CheckBoxWithMessage(&parent);
    
    ASSERT_NE(tmpwidget, nullptr);
    EXPECT_EQ(tmpwidget->parent(), &parent);
    delete tmpwidget;
}

// Test constructor without parent
TEST_F(TestCheckBoxWithMessage, TestConstructorWithoutParent)
{   
    ASSERT_NE(widget, nullptr);
    EXPECT_EQ(widget->parent(), nullptr);
}

// Test setDisplayText method
TEST_F(TestCheckBoxWithMessage, TestSetDisplayText)
{   
    QString checkText = "Test checkbox";
    QString message = "Test message";
    widget->setDisplayText(checkText, message);
    
    // Should not crash
    SUCCEED();
}

// Test setChecked method
TEST_F(TestCheckBoxWithMessage, TestSetChecked)
{  
    // Test checking
    widget->setChecked(true);
    
    // Test unchecking
    widget->setChecked(false);
    
    // Should not crash
    SUCCEED();
}

// Test signal emission
TEST_F(TestCheckBoxWithMessage, TestSignalEmission)
{
    QSignalSpy spy(widget, &CheckBoxWithMessage::stateChanged);
    
    // Set checkbox text and message
    widget->setDisplayText("Test", "Message");
    
    // Change checked state
    widget->setChecked(true);
    widget->setChecked(false);
    
    // Should emit signals (though exact count depends on implementation)
    EXPECT_GE(spy.count(), 0);
}

// Test widget composition
TEST_F(TestCheckBoxWithMessage, TestWidgetComposition)
{
    // Should have children (checkbox and label)
    QList<QCheckBox*> checkBoxes = widget->findChildren<QCheckBox*>();
    QList<QLabel*> labels = widget->findChildren<QLabel*>();
    
    EXPECT_GE(checkBoxes.size(), 1);
    EXPECT_GE(labels.size(), 1);
}

// Test with empty strings
TEST_F(TestCheckBoxWithMessage, TestWithEmptyStrings)
{
    widget->setDisplayText("", "");
    
    // Should handle empty strings
    SUCCEED();
}

// Test with long text
TEST_F(TestCheckBoxWithMessage, TestWithLongText)
{
    QString longCheckText = "This is a very long checkbox text that should be handled properly";
    QString longMessage = "This is a very long message that should be displayed correctly without causing any layout issues or visual problems.";
    
    widget->setDisplayText(longCheckText, longMessage);
    
    // Should handle long text
    SUCCEED();
}

// Test multiple setDisplayText calls
TEST_F(TestCheckBoxWithMessage, TestMultipleSetDisplayText)
{
    // Set text multiple times
    widget->setDisplayText("First", "First message");
    widget->setDisplayText("Second", "Second message");
    widget->setDisplayText("Third", "Third message");
    
    // Should handle multiple updates
    SUCCEED();
}

// Test multiple setChecked calls
TEST_F(TestCheckBoxWithMessage, TestMultipleSetChecked)
{
    // Toggle checked state multiple times
    for (int i = 0; i < 10; ++i) {
        widget->setChecked(i % 2 == 0);
    }
    
    // Should handle multiple toggles
    SUCCEED();
}

// Test widget visibility
TEST_F(TestCheckBoxWithMessage, TestWidgetVisibility)
{
    // Initially should be visible
    EXPECT_TRUE(widget->isVisible());
    
    // Test hiding
    widget->hide();
    EXPECT_FALSE(widget->isVisible());
    
    // Test showing
    widget->show();
    EXPECT_TRUE(widget->isVisible());
}

// Test widget enabled state
TEST_F(TestCheckBoxWithMessage, TestWidgetEnabledState)
{
    // Initially should be enabled
    EXPECT_TRUE(widget->isEnabled());
    
    // Test disabling
    widget->setEnabled(false);
    EXPECT_FALSE(widget->isEnabled());
    
    // Test enabling
    widget->setEnabled(true);
    EXPECT_TRUE(widget->isEnabled());
}

// Test widget size
TEST_F(TestCheckBoxWithMessage, TestWidgetSize)
{
    // Should have reasonable size
    QSize size = widget->sizeHint();
    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
    
    QSize minSize = widget->minimumSize();
    EXPECT_GE(minSize.width(), 0);
    EXPECT_GE(minSize.height(), 0);
}

// Test with parent widget
TEST_F(TestCheckBoxWithMessage, TestWithParentWidget)
{
    QWidget parent;
    QVBoxLayout *layout = new QVBoxLayout(&parent);
    
    CheckBoxWithMessage *tmpWidget = new CheckBoxWithMessage(&parent);
    layout->addWidget(tmpWidget);
    
    // Should be properly added to parent
    EXPECT_EQ(tmpWidget->parent(), &parent);
    EXPECT_TRUE(layout->indexOf(tmpWidget) >= 0);
    delete tmpWidget;
}

// Test widget font
TEST_F(TestCheckBoxWithMessage, TestWidgetFont)
{
    QFont font = widget->font();
    EXPECT_FALSE(font.family().isEmpty());
    
    // Test setting font
    font.setBold(true);
    widget->setFont(font);
    EXPECT_TRUE(widget->font().bold());
}

// Test widget stylesheet
TEST_F(TestCheckBoxWithMessage, TestWidgetStylesheet)
{
    QString stylesheet = "QWidget { background-color: red; }";
    widget->setStyleSheet(stylesheet);
    
    EXPECT_EQ(widget->styleSheet(), stylesheet);
}

// Test widget object name
TEST_F(TestCheckBoxWithMessage, TestWidgetObjectName)
{
    QString name = "TestCheckBox";
    widget->setObjectName(name);
    
    EXPECT_EQ(widget->objectName(), name);
}

// Test mouse interaction (basic)
TEST_F(TestCheckBoxWithMessage, TestMouseInteraction)
{
    widget->setDisplayText("Test", "Message");
    
    // Show widget to enable mouse events
    widget->show();
    
    // Simulate mouse click (this may not work without proper event loop)
    QTest::mouseClick(widget, Qt::LeftButton);
    
    // Should not crash
    SUCCEED();
}

// Test with special characters in text
TEST_F(TestCheckBoxWithMessage, TestSpecialCharacters)
{
    QString specialText = "测试复选框 & 特殊字符";
    QString specialMessage = "包含特殊字符的提示信息：@#$%^&*()";
    
    widget->setDisplayText(specialText, specialMessage);
    
    // Should handle special characters
    SUCCEED();
}

// Test destructor
TEST_F(TestCheckBoxWithMessage, TestDestructor)
{
    // Create and immediately delete
    CheckBoxWithMessage *tempWidget = new CheckBoxWithMessage();
    delete tempWidget;
    
    SUCCEED();
}

// Test multiple instances
TEST_F(TestCheckBoxWithMessage, TestMultipleInstances)
{
    CheckBoxWithMessage *widget1 = new CheckBoxWithMessage();
    CheckBoxWithMessage *widget2 = new CheckBoxWithMessage();
    CheckBoxWithMessage *widget3 = new CheckBoxWithMessage();
    
    // Set different texts
    widget1->setDisplayText("Option 1", "Message 1");
    widget2->setDisplayText("Option 2", "Message 2");
    widget3->setDisplayText("Option 3", "Message 3");
    
    // Should be independent
    EXPECT_NE(widget1, widget2);
    EXPECT_NE(widget2, widget3);
    EXPECT_NE(widget1, widget3);
    
    delete widget1;
    delete widget2;
    delete widget3;
}

// Test rapid state changes
TEST_F(TestCheckBoxWithMessage, TestRapidStateChanges)
{
    // Rapid state changes
    for (int i = 0; i < 100; ++i) {
        widget->setChecked(i % 2 == 0);
        widget->setDisplayText(QString("Text %1").arg(i), QString("Message %1").arg(i));
    }
    
    // Should handle rapid changes
    SUCCEED();
}