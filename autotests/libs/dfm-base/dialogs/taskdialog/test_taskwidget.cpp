// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QUrl>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QEnterEvent>
#include <QTimer>
#include <QSignalSpy>

#include <dfm-base/dialogs/taskdialog/taskwidget.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/private/application_p.h>

// Include stub headers
#include "stubext.h"

using namespace dfmbase;

// Create a concrete implementation of AbstractJobHandler for testing
class WidgetJobHandlerImpl : public dfmbase::AbstractJobHandler
{
    Q_OBJECT
public:
    WidgetJobHandlerImpl() {}

    // AbstractJobHandler might not have these as pure virtual methods
    // Just inherit the base class without overriding
};

class TestTaskWidget : public testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 1;
            char name[] = "test";
            char *argv[] = {name, nullptr};
            app = new QApplication(argc, argv);
        } else {
            app = nullptr;  // QApplication already exists
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

        widget = new TaskWidget();
    }
    
    void TearDown() override {
        stub.clear();
        if (widget) {
            // Process pending events before deletion
            QCoreApplication::processEvents();
            delete widget;
            widget = nullptr;
        }
    }
    
public:
    stub_ext::StubExt stub;
    QApplication *app = nullptr;
    TaskWidget *widget = nullptr;
};

// Test constructor
TEST_F(TestTaskWidget, TestConstructor)
{
    QWidget parent;
    
    TaskWidget *tmpwidget = new TaskWidget(&parent);
    
    ASSERT_NE(tmpwidget, nullptr);
    EXPECT_EQ(tmpwidget->parent(), &parent);
    delete tmpwidget;
}

// Test constructor without parent
TEST_F(TestTaskWidget, TestConstructorWithoutParent)
{
    TaskWidget *tmpwidget = new TaskWidget();
    
    ASSERT_NE(tmpwidget, nullptr);
    EXPECT_EQ(tmpwidget->parent(), nullptr);
    delete tmpwidget;
}

// Test destructor
TEST_F(TestTaskWidget, TestDestructor)
{
    TaskWidget *tmpwidget = new TaskWidget();
    delete tmpwidget;
    
    SUCCEED();
}

// Test setTaskHandle with null handler
TEST_F(TestTaskWidget, TestSetTaskHandleNull)
{   
    JobHandlePointer nullHandler;
    widget->setTaskHandle(nullHandler);
    
    // Should handle null handler gracefully
    SUCCEED();
}

// Test setTaskHandle with valid handler
TEST_F(TestTaskWidget, TestSetTaskHandleValid)
{
    // Create a mock AbstractJobHandler
    JobHandlePointer handler(new WidgetJobHandlerImpl());
    
    // // Mock getTaskInfoByNotifyType to return null for all types
    // stub.set_lamda(ADDR(AbstractJobHandler, getTaskInfoByNotifyType), []() {
    //     __DBG_STUB_INVOKE__
    //     return JobInfoPointer(nullptr);
    // });
    
    widget->setTaskHandle(handler);
    
    // Should not crash
    SUCCEED();
}

// Test resetPauseStute method
TEST_F(TestTaskWidget, TestResetPauseStute)
{
    widget->resetPauseStute();
    
    // Should not crash
    SUCCEED();
}

// Test onButtonClicked with null sender
TEST_F(TestTaskWidget, TestOnButtonClickedNullSender)
{
    // Call onButtonClicked directly (sender() will be null)
    widget->onButtonClicked();
    
    // Should handle null sender gracefully
    SUCCEED();
}

// Test parentClose method
TEST_F(TestTaskWidget, TestParentClose)
{
    widget->parentClose();
    
    // Should not crash
    SUCCEED();
}

// Test onShowErrors method
TEST_F(TestTaskWidget, TestOnShowErrors)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    widget->onShowErrors(jobInfo);
    
    // Should not crash
    SUCCEED();
}

// Test onShowConflictInfo method
TEST_F(TestTaskWidget, TestOnShowConflictInfo)
{
    QUrl source("file:///source");
    QUrl target("file:///target");
    AbstractJobHandler::SupportActions action = AbstractJobHandler::SupportAction::kCoexistAction;
    
    widget->onShowConflictInfo(source, target, action);
    
    // Should not crash
    SUCCEED();
}

// Test onShowPermanentlyDelete method
TEST_F(TestTaskWidget, TestOnShowPermanentlyDelete)
{
    QUrl source("file:///source");
    AbstractJobHandler::SupportActions action = AbstractJobHandler::SupportAction::kReplaceAction;
    
    widget->onShowPermanentlyDelete(source, action);
    
    // Should not crash
    SUCCEED();
}

// Test onHandlerTaskStateChange method
TEST_F(TestTaskWidget, TestOnHandlerTaskStateChange)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    widget->onHandlerTaskStateChange(jobInfo);
    
    // Should not crash
    SUCCEED();
}

// Test onShowTaskInfo method
TEST_F(TestTaskWidget, TestOnShowTaskInfo)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    widget->onShowTaskInfo(jobInfo);
    
    // Should not crash
    SUCCEED();
}

// Test onShowTaskProccess method
TEST_F(TestTaskWidget, TestOnShowTaskProccess)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    widget->onShowTaskProccess(jobInfo);
    
    // Should not crash
    SUCCEED();
}

// Test onShowSpeedUpdatedInfo method
TEST_F(TestTaskWidget, TestOnShowSpeedUpdatedInfo)
{
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    widget->onShowSpeedUpdatedInfo(jobInfo);
    
    // Should not crash
    SUCCEED();
}

// Test onInfoTimer method
TEST_F(TestTaskWidget, TestOnInfoTimer)
{
    widget->onInfoTimer();
    
    // Should not crash
    SUCCEED();
}

// Test UI components existence
TEST_F(TestTaskWidget, TestUIComponents)
{
    // Check if basic UI components exist
    // Note: These might be nullptr since we can't access private members directly
    // But the widget should be properly initialized
    EXPECT_NE(widget, nullptr);
    EXPECT_GT(widget->width(), 0);
    EXPECT_GT(widget->height(), 0);
}

// Test enterEvent
TEST_F(TestTaskWidget, TestEnterEvent)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QEnterEvent event(QPointF(0, 0), QPointF(0, 0), QPointF(0, 0));
#else
    QEvent event(QEvent::Enter);
#endif
    
    QApplication::sendEvent(widget, &event);
    
    // Should not crash
    SUCCEED();
}

// Test leaveEvent
TEST_F(TestTaskWidget, TestLeaveEvent)
{
    QEvent event(QEvent::Leave);
    QApplication::sendEvent(widget, &event);
    
    // Should not crash
    SUCCEED();
}

// Test paintEvent
TEST_F(TestTaskWidget, TestPaintEvent)
{
    QPaintEvent event(widget->rect());
    QApplication::sendEvent(widget, &event);
    
    // Should not crash
    SUCCEED();
}

// Test resizeEvent
TEST_F(TestTaskWidget, TestResizeEvent)
{
    QResizeEvent event(QSize(400, 100), QSize(300, 80));
    QApplication::sendEvent(widget, &event);
    
    // Should not crash
    SUCCEED();
}

// Test buttonClicked signal
TEST_F(TestTaskWidget, TestButtonClickedSignal)
{
    QSignalSpy spy(widget, &TaskWidget::buttonClicked);
    
    // We can't easily trigger button clicks without access to private buttons
    // But we can verify the signal exists
    EXPECT_EQ(spy.count(), 0);
}

// Test heightChanged signal
TEST_F(TestTaskWidget, TestHeightChangedSignal)
{
    QSignalSpy spy(widget, &TaskWidget::heightChanged);
    
    // Height might change during initialization
    // We can't easily trigger it but can verify the signal exists
    EXPECT_TRUE(widget->metaObject()->indexOfSignal("heightChanged(int)") >= 0);
}

// Test formatTime method (private method)
TEST_F(TestTaskWidget, TestFormatTime)
{
    // We can't test private method directly
    // But we can test that time formatting doesn't crash in related operations
    JobInfoPointer jobInfo(new QMap<quint8, QVariant>());
    (*jobInfo)[static_cast<quint8>(0)] = static_cast<qint64>(60); // 60 seconds
    
    widget->onShowSpeedUpdatedInfo(jobInfo);
    
    SUCCEED();
}

// Test with different job info states
TEST_F(TestTaskWidget, TestDifferentJobStates)
{
    // Test running state
    JobInfoPointer runningInfo(new QMap<quint8, QVariant>());
    (*runningInfo)[static_cast<quint8>(0)] = static_cast<int>(AbstractJobHandler::JobState::kRunningState);
    widget->onHandlerTaskStateChange(runningInfo);
    
    // Test paused state
    JobInfoPointer pausedInfo(new QMap<quint8, QVariant>());
    (*pausedInfo)[static_cast<quint8>(0)] = static_cast<int>(AbstractJobHandler::JobState::kPauseState);
    widget->onHandlerTaskStateChange(pausedInfo);
    
    // Test stopped state
    JobInfoPointer stoppedInfo(new QMap<quint8, QVariant>());
    (*stoppedInfo)[static_cast<quint8>(0)] = static_cast<int>(AbstractJobHandler::JobState::kStopState);
    widget->onHandlerTaskStateChange(stoppedInfo);
    
    SUCCEED();
}

// Test with task progress
TEST_F(TestTaskWidget, TestTaskProgress)
{
    JobInfoPointer progressInfo(new QMap<quint8, QVariant>());
    (*progressInfo)[static_cast<quint8>(0)] = static_cast<qint64>(1000);
    (*progressInfo)[static_cast<quint8>(1)] = static_cast<qint64>(500);
    
    widget->onShowTaskProccess(progressInfo);
    
    // Should handle progress update
    SUCCEED();
}

// Test with speed information
TEST_F(TestTaskWidget, TestSpeedInformation)
{
    JobInfoPointer speedInfo(new QMap<quint8, QVariant>());
    (*speedInfo)[static_cast<quint8>(0)] = static_cast<qint64>(1024 * 1024); // 1 MB/s
    (*speedInfo)[static_cast<quint8>(1)] = static_cast<qint64>(120); // 2 minutes
    
    widget->onShowSpeedUpdatedInfo(speedInfo);
    
    // Should handle speed update
    SUCCEED();
}

// Test with file information
TEST_F(TestTaskWidget, TestFileInformation)
{
    QUrl sourceUrl("file:///test/source.txt");
    QUrl targetUrl("file:///test/target.txt");
    
    JobInfoPointer taskInfo(new QMap<quint8, QVariant>());
    (*taskInfo)[static_cast<quint8>(0)] = sourceUrl;
    (*taskInfo)[static_cast<quint8>(1)] = targetUrl;
    
    widget->onShowTaskInfo(taskInfo);
    
    // Should handle file information
    SUCCEED();
}

// Test multiple handlers
TEST_F(TestTaskWidget, TestMultipleHandlers)
{
    JobHandlePointer handler1(new WidgetJobHandlerImpl());
    JobHandlePointer handler2(new WidgetJobHandlerImpl());
    
    // // Mock getTaskInfoByNotifyType
    // stub.set_lamda(ADDR(AbstractJobHandler, getTaskInfoByNotifyType), []() {
    //     __DBG_STUB_INVOKE__
    //     return JobInfoPointer(nullptr);
    // });
    
    widget->setTaskHandle(handler1);
    widget->setTaskHandle(handler2);
    
    // Should handle multiple handlers
    SUCCEED();
}

// Test widget size hints
TEST_F(TestTaskWidget, TestWidgetSizeHints)
{
    // Should have reasonable size hints
    QSize sizeHint = widget->sizeHint();
    EXPECT_GT(sizeHint.width(), 0);
    EXPECT_GT(sizeHint.height(), 0);
    
    QSize minimumSize = widget->minimumSize();
    EXPECT_GE(minimumSize.width(), 0);
    EXPECT_GE(minimumSize.height(), 0);
}

// Test widget visibility
TEST_F(TestTaskWidget, TestWidgetVisibility)
{
    // Initially should be hidden
    EXPECT_FALSE(widget->isVisible());
    
    // Test hiding
    widget->hide();
    EXPECT_FALSE(widget->isVisible());
    
    // Test showing
    widget->show();
    EXPECT_TRUE(widget->isVisible());
}

// Test widget enabled state
TEST_F(TestTaskWidget, TestWidgetEnabledState)
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

// Test widget font
TEST_F(TestTaskWidget, TestWidgetFont)
{
    QFont font = widget->font();
    EXPECT_FALSE(font.family().isEmpty());
    
    // Test setting font
    font.setBold(true);
    widget->setFont(font);
    EXPECT_TRUE(widget->font().bold());
}

// Test widget stylesheet
TEST_F(TestTaskWidget, TestWidgetStylesheet)
{
    QString stylesheet = "QWidget { background-color: red; }";
    widget->setStyleSheet(stylesheet);
    
    EXPECT_EQ(widget->styleSheet(), stylesheet);
}

// Test with long file paths
TEST_F(TestTaskWidget, TestLongFilePaths)
{
    QString longPath = "/very/long/path/that/exceeds/normal/display/limits/and/should/be/elided/properly/by/the/elidedlabel/component/when/displayed/in/the/task/widget.txt";
    
    JobInfoPointer taskInfo(new QMap<quint8, QVariant>());
    (*taskInfo)[static_cast<quint8>(0)] = QUrl::fromLocalFile(longPath);
    (*taskInfo)[static_cast<quint8>(1)] = QUrl::fromLocalFile(longPath + "_target");
    
    widget->onShowTaskInfo(taskInfo);
    
    // Should handle long paths without crashing
    SUCCEED();
}

// Test error handling with corrupted data
TEST_F(TestTaskWidget, TestErrorHandlingCorruptedData)
{
    JobInfoPointer emptyInfo(new QMap<quint8, QVariant>());
    JobInfoPointer corruptedInfo(new QMap<quint8, QVariant>());
    (*corruptedInfo)[static_cast<quint8>(0)] = QVariant();  // Invalid variant
    (*corruptedInfo)[static_cast<quint8>(1)] = QString();   // Empty string
    
    widget->onShowErrors(emptyInfo);
    widget->onShowTaskInfo(corruptedInfo);
    widget->onShowTaskProccess(corruptedInfo);
    widget->onShowSpeedUpdatedInfo(corruptedInfo);
    
    // Should handle corrupted data gracefully
    SUCCEED();
}

// Test multiple instances
TEST_F(TestTaskWidget, TestMultipleInstances)
{
    TaskWidget *widget1 = new TaskWidget();
    TaskWidget *widget2 = new TaskWidget();
    TaskWidget *widget3 = new TaskWidget();
    
    // All should be valid instances
    EXPECT_NE(widget1, widget2);
    EXPECT_NE(widget2, widget3);
    EXPECT_NE(widget1, widget3);
    
    delete widget1;
    delete widget2;
    delete widget3;
}

#include "test_taskwidget.moc"
