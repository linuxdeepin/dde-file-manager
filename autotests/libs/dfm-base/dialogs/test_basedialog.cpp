// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QWidget>
#include <QResizeEvent>
#include <QFont>
#include <QApplication>
#include <QWindow>

#include <dfm-base/dialogs/basedialog/basedialog.h>
#include <dfm-base/utils/windowutils.h>

#include <DTitlebar>
#include <DDialog>
#include <DAbstractDialog>

// Include stub headers
#include "stubext.h"

using namespace DTK_WIDGET_NAMESPACE;

using namespace dfmbase;

class TestBaseDialog : public testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
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
    }

    void TearDown() override {
        // Cleanup code after each test
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

// Test constructor and destructor
TEST_F(TestBaseDialog, TestConstructorDestructor) {
    QWidget parent;
    BaseDialog *dialog = new BaseDialog(&parent);
    
    EXPECT_NE(dialog, nullptr);
    EXPECT_EQ(dialog->parent(), &parent);
    
    // Test destructor
    delete dialog;
    
    // Should not crash
    SUCCEED();
}

// Test constructor with nullptr parent
TEST_F(TestBaseDialog, TestConstructorWithNullParent) {
    BaseDialog *dialog = new BaseDialog(nullptr);
    
    EXPECT_NE(dialog, nullptr);
    EXPECT_EQ(dialog->parent(), nullptr);
    
    delete dialog;
    SUCCEED();
}

// Test setTitle function
TEST_F(TestBaseDialog, TestSetTitle) {
    BaseDialog dialog;
    QString testTitle = "Test Dialog Title";
    
    // Mock DTitlebar::setTitle
    QString capturedTitle;
    stub.set_lamda(ADDR(DTitlebar, setTitle), [&](QWidget*, const QString &title) {
        capturedTitle = title;
    });
    
    dialog.setTitle(testTitle);
    
    EXPECT_EQ(capturedTitle, testTitle);
}

// Test setTitle with empty string
TEST_F(TestBaseDialog, TestSetTitleEmpty) {
    BaseDialog dialog;
    QString emptyTitle = "";
    
    // Mock DTitlebar::setTitle
    QString capturedTitle;
    stub.set_lamda(ADDR(DTitlebar, setTitle), [&](QWidget*, const QString &title) {
        capturedTitle = title;
    });
    
    dialog.setTitle(emptyTitle);
    
    EXPECT_EQ(capturedTitle, emptyTitle);
}

// Test setTitleFont function
TEST_F(TestBaseDialog, TestSetTitleFont) {
    BaseDialog dialog;
    QFont testFont;
    testFont.setFamily("Arial");
    testFont.setPointSize(12);
    
    // Mock DTitlebar::setFont
    QFont capturedFont;
    stub.set_lamda(ADDR(DTitlebar, setFont), [&](QWidget*, const QFont &font) {
        capturedFont = font;
    });
    
    dialog.setTitleFont(testFont);
    
    EXPECT_EQ(capturedFont.family(), testFont.family());
    EXPECT_EQ(capturedFont.pointSize(), testFont.pointSize());
}

// Test setTitleFont with default font
TEST_F(TestBaseDialog, TestSetTitleFontDefault) {
    BaseDialog dialog;
    QFont defaultFont;
    
    // Mock DTitlebar::setFont
    QFont capturedFont;
    stub.set_lamda(ADDR(DTitlebar, setFont), [&](QWidget*, const QFont &font) {
        capturedFont = font;
    });
    
    dialog.setTitleFont(defaultFont);
    
    EXPECT_EQ(capturedFont, defaultFont);
}

// Test resizeEvent
TEST_F(TestBaseDialog, TestResizeEvent) {
    BaseDialog dialog;
    
    // Mock DTitlebar::setFixedWidth
    int capturedWidth = 0;
    stub.set_lamda(ADDR(DTitlebar, setFixedWidth), [&](QWidget*, int width) {
        capturedWidth = width;
    });
    
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    QApplication::sendEvent(&dialog, &event);
    
    EXPECT_EQ(capturedWidth, 800);
}

// Test resizeEvent with zero size
TEST_F(TestBaseDialog, TestResizeEventZeroSize) {
    BaseDialog dialog;
    
    // Mock DTitlebar::setFixedWidth
    int capturedWidth = 0;
    stub.set_lamda(ADDR(DTitlebar, setFixedWidth), [&](QWidget*, int width) {
        capturedWidth = width;
    });
    
    QResizeEvent event(QSize(0, 0), QSize(0, 0));
    QApplication::sendEvent(&dialog, &event);
    
    EXPECT_EQ(capturedWidth, 0);
}

// Test resizeEvent with negative size (edge case)
TEST_F(TestBaseDialog, TestResizeEventNegativeSize) {
    BaseDialog dialog;
    
    // Mock DTitlebar::setFixedWidth
    int capturedWidth = 0;
    stub.set_lamda(ADDR(DTitlebar, setFixedWidth), [&](QWidget*, int width) {
        capturedWidth = width;
    });
    
    QResizeEvent event(QSize(-100, -100), QSize(0, 0));
    QApplication::sendEvent(&dialog, &event);
    
    EXPECT_EQ(capturedWidth, -100);
}

// Test titlebar initialization in Wayland environment
TEST_F(TestBaseDialog, TestTitlebarInitializationWayland) {
    // Mock WindowUtils::isWayLand to return true
    stub.set_lamda(ADDR(WindowUtils, isWayLand), []() {
        return true;
    });
    
    QWidget parent;
    BaseDialog dialog(&parent);
    
    // Should have created a titlebar
    EXPECT_NE(dialog.titlebar, nullptr);
    
    // In Wayland, titlebar should be transparent
    // This is set in constructor
    SUCCEED();
}

// Test titlebar initialization in non-Wayland environment
TEST_F(TestBaseDialog, TestTitlebarInitializationNonWayland) {
    // Mock WindowUtils::isWayLand to return false
    stub.set_lamda(ADDR(WindowUtils, isWayLand), []() {
        return false;
    });
    
    QWidget parent;
    BaseDialog dialog(&parent);
    
    // Should have created a titlebar
    EXPECT_NE(dialog.titlebar, nullptr);
    
    // Should not crash
    SUCCEED();
}

// Test window flags in Wayland
TEST_F(TestBaseDialog, TestWindowFlagsWayland) {
    // Mock WindowUtils::isWayLand to return true
    stub.set_lamda(ADDR(WindowUtils, isWayLand), []() {
        return true;
    });
    
    QWidget parent;
    BaseDialog dialog(&parent);
    
    // In Wayland, the window should have certain flags and properties
    // The constructor sets window flags and properties
    EXPECT_TRUE(dialog.testAttribute(Qt::WA_NativeWindow));
}

// Test window flags in non-Wayland
TEST_F(TestBaseDialog, TestWindowFlagsNonWayland) {
    // Mock WindowUtils::isWayLand to return false
    stub.set_lamda(ADDR(WindowUtils, isWayLand), []() {
        return false;
    });
    
    QWidget parent;
    BaseDialog dialog(&parent);
    
    // Should create successfully in non-Wayland mode
    EXPECT_NE(dialog.titlebar, nullptr);
}

// Test titlebar background transparency
TEST_F(TestBaseDialog, TestTitlebarBackground) {
    BaseDialog dialog;
    
    // The constructor sets titlebar background to transparent
    // This is a fixed behavior
    EXPECT_NE(dialog.titlebar, nullptr);
    
    // The actual transparency is set in constructor
    // We can't directly test it without accessing private members
    SUCCEED();
}

// Test multiple resize events
TEST_F(TestBaseDialog, TestMultipleResizeEvents) {
    BaseDialog dialog;
    
    // Track width changes
    QList<int> widthChanges;
    stub.set_lamda(ADDR(DTitlebar, setFixedWidth), [&](QWidget*, int width) {
        widthChanges.append(width);
    });
    
    // Send multiple resize events
    QResizeEvent event1(QSize(400, 300), QSize(200, 150));
    QApplication::sendEvent(&dialog, &event1);
    
    QResizeEvent event2(QSize(800, 600), QSize(400, 300));
    QApplication::sendEvent(&dialog, &event2);
    
    QResizeEvent event3(QSize(1024, 768), QSize(800, 600));
    QApplication::sendEvent(&dialog, &event3);
    
    EXPECT_EQ(widthChanges.size(), 3);
    EXPECT_EQ(widthChanges[0], 400);
    EXPECT_EQ(widthChanges[1], 800);
    EXPECT_EQ(widthChanges[2], 1024);
}

// Test setting title after constructor
TEST_F(TestBaseDialog, TestSetTitleAfterConstructor) {
    BaseDialog dialog;
    
    // Set initial title
    QString initialTitle = "Initial Title";
    stub.set_lamda(ADDR(DTitlebar, setTitle), [](QWidget*, const QString &) {});
    dialog.setTitle(initialTitle);
    
    // Change title
    QString newTitle = "New Title";
    dialog.setTitle(newTitle);
    
    // Should not crash
    SUCCEED();
}

// Test setting font multiple times
TEST_F(TestBaseDialog, TestSetFontMultipleTimes) {
    BaseDialog dialog;
    
    QFont font1, font2, font3;
    font1.setFamily("Arial");
    font2.setFamily("Helvetica");
    font3.setFamily("Times");
    
    stub.set_lamda(ADDR(DTitlebar, setFont), [](QWidget*, const QFont &) {});
    
    // Set multiple fonts
    dialog.setTitleFont(font1);
    dialog.setTitleFont(font2);
    dialog.setTitleFont(font3);
    
    // Should not crash
    SUCCEED();
}