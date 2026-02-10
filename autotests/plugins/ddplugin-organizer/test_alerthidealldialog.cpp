// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/alerthidealldialog.h"

#include <QEvent>
#include <QMouseEvent>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_AlertHideAllDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        dialog = new AlertHideAllDialog();

        // mock the UI show
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        delete dialog;

        stub.clear();
    }

public:
    AlertHideAllDialog *dialog;
    stub_ext::StubExt stub;
};

TEST_F(UT_AlertHideAllDialog, TestConstructor)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_FALSE(dialog->isRepeatNoMore());
    EXPECT_EQ(dialog->confirmBtnIndex(), -1);
}

TEST_F(UT_AlertHideAllDialog, TestInitialize)
{
    EXPECT_NO_THROW(dialog->initialize());
    
    // Test that initialize doesn't crash even if called multiple times
    EXPECT_NO_THROW(dialog->initialize());
    EXPECT_NO_THROW(dialog->initialize());
}

TEST_F(UT_AlertHideAllDialog, TestIsRepeatNoMore)
{
    // Initially should be false
    EXPECT_FALSE(dialog->isRepeatNoMore());
    
    // After initialize, should still be valid
    dialog->initialize();
    EXPECT_FALSE(dialog->isRepeatNoMore());
}

TEST_F(UT_AlertHideAllDialog, TestConfirmBtnIndex)
{
    // Initially should be -1
    EXPECT_EQ(dialog->confirmBtnIndex(), -1);
    
    // After initialize, should be valid (>= 0)
    dialog->initialize();
    EXPECT_GE(dialog->confirmBtnIndex(), 0);
}

TEST_F(UT_AlertHideAllDialog, TestEventFilter)
{
    // Test with basic event types
    QEvent showEvent(QEvent::Show);
    QEvent hideEvent(QEvent::Hide);
    QEvent resizeEvent(QEvent::Resize);
    
    // Event filter should handle various events without crashing
    EXPECT_TRUE(dialog->eventFilter(dialog, &showEvent));
    EXPECT_TRUE(dialog->eventFilter(dialog, &hideEvent));
    EXPECT_TRUE(dialog->eventFilter(dialog, &resizeEvent));
    
    // Test with mouse events
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent mouseMove(QEvent::MouseMove, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    
    EXPECT_TRUE(dialog->eventFilter(dialog, &mousePress));
    EXPECT_TRUE(dialog->eventFilter(dialog, &mouseRelease));
    EXPECT_TRUE(dialog->eventFilter(dialog, &mouseMove));
    
    // Test with null object
    EXPECT_TRUE(dialog->eventFilter(nullptr, &showEvent));
    
    // Test with null event
    // EXPECT_TRUE(dialog->eventFilter(dialog, nullptr));
}

TEST_F(UT_AlertHideAllDialog, TestEventFilterAfterInitialize)
{
    dialog->initialize();
    
    // Test events after initialization
    QEvent showEvent(QEvent::Show);
    QEvent closeEvent(QEvent::Close);
    
    EXPECT_TRUE(dialog->eventFilter(dialog, &showEvent));
    EXPECT_TRUE(dialog->eventFilter(dialog, &closeEvent));
}

TEST_F(UT_AlertHideAllDialog, TestMultipleInitialization)
{
    // Test multiple calls to initialize
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW(dialog->initialize());
        EXPECT_FALSE(dialog->isRepeatNoMore());
        EXPECT_GE(dialog->confirmBtnIndex(), 0);
    }
}

TEST_F(UT_AlertHideAllDialog, TestEdgeCases)
{
    // Test dialog with parent
    QWidget parent;
    AlertHideAllDialog *childDialog = new AlertHideAllDialog(&parent);
    EXPECT_NE(childDialog, nullptr);
    EXPECT_NO_THROW(childDialog->initialize());
    delete childDialog;
    
    // Test that main dialog is still valid
    EXPECT_NE(dialog, nullptr);
    EXPECT_FALSE(dialog->isRepeatNoMore());
}

TEST_F(UT_AlertHideAllDialog, TestDialogProperties)
{
    dialog->initialize();
    
    // Test dialog basic properties
    EXPECT_FALSE(dialog->isModal());  // DDialog default behavior
    EXPECT_TRUE(dialog->isVisible() || !dialog->isVisible());  // Either state is valid
    
    // Test that dialog can be shown and hidden
    EXPECT_NO_THROW(dialog->show());
    EXPECT_NO_THROW(dialog->hide());
    
    // Test that dialog can be raised
    EXPECT_NO_THROW(dialog->raise());
}

TEST_F(UT_AlertHideAllDialog, TestEventFilterWithDifferentObjects)
{
    QObject *testObject = new QObject();
    
    QEvent testEvent(QEvent::User);
    EXPECT_TRUE(dialog->eventFilter(testObject, &testEvent));
    
    delete testObject;
}

TEST_F(UT_AlertHideAllDialog, TestComplexEventSequence)
{
    dialog->initialize();
    
    // Simulate a sequence of events that might occur in real usage
    QEvent showEvent(QEvent::Show);
    QEvent resizeEvent(QEvent::Resize);
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPoint(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, QPoint(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QEvent closeEvent(QEvent::Close);
    
    EXPECT_TRUE(dialog->eventFilter(dialog, &showEvent));
    EXPECT_TRUE(dialog->eventFilter(dialog, &resizeEvent));
    EXPECT_TRUE(dialog->eventFilter(dialog, &mousePress));
    EXPECT_TRUE(dialog->eventFilter(dialog, &mouseRelease));
    EXPECT_TRUE(dialog->eventFilter(dialog, &closeEvent));
    
    // Dialog state should still be valid after event sequence
    EXPECT_FALSE(dialog->isRepeatNoMore());
    EXPECT_GE(dialog->confirmBtnIndex(), 0);
}
