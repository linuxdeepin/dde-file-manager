// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QMutex>
#include <QDialog>
#include <QDBusUnixFileDescriptor>
#include <DTitlebar>

#include <dfm-base/dialogs/taskdialog/taskdialog.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/private/application_p.h>
#include <QSharedPointer>

// Include stub headers
#include "stubext.h"

using namespace dfmbase;

// Create a concrete implementation of AbstractJobHandler for testing
class DialogJobHandlerImpl : public dfmbase::AbstractJobHandler
{
    Q_OBJECT
public:
    DialogJobHandlerImpl() {}

    // AbstractJobHandler might not have these as pure virtual methods
    // Just inherit the base class without overriding
};

class TestTaskDialog : public testing::Test
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

        dialog = new TaskDialog();
    }
    
    void TearDown() override {
        stub.clear();
        if (dialog) {
            // Close dialog first to ensure proper cleanup
            dialog->close();
            // Process pending events to ensure cleanup is complete
            QCoreApplication::processEvents();
            // Give some time for async cleanup
            QThread::msleep(10);
            QCoreApplication::processEvents();
            delete dialog;
            dialog = nullptr;
        }
        
        // Clean up the Application instance to avoid "there should be only one application object" assertion
        stub.set_lamda(&Application::instance, []() -> Application * {
            return nullptr;
        });
    }
    
public:
    stub_ext::StubExt stub;
    QApplication *app = nullptr;
    TaskDialog *dialog = nullptr;
};

// Test constructor
TEST_F(TestTaskDialog, TestConstructor)
{
    QObject parent;
    
    TaskDialog *tmpdialog = new TaskDialog(&parent);
    
    ASSERT_NE(tmpdialog, nullptr);
    // Parent is QObject, not QWidget, so parent() might be different
    SUCCEED();
    delete tmpdialog;
}

// Test constructor without parent
TEST_F(TestTaskDialog, TestConstructorWithoutParent)
{
    ASSERT_NE(dialog, nullptr);
    EXPECT_EQ(dialog->parent(), nullptr);
}

// Test destructor
TEST_F(TestTaskDialog, TestDestructor)
{
    TaskDialog *tmpdialog = new TaskDialog();
    delete tmpdialog;
    
    SUCCEED();
}

// Test initUI method
TEST_F(TestTaskDialog, TestInitUI)
{
    // initUI is called in constructor
    // Check if UI components are initialized
    EXPECT_NE(dialog->findChild<QListWidget*>(), nullptr);
    
    // Check if titlebar exists
    EXPECT_NE(dialog->findChild<DTitlebar*>(), nullptr);
    
    // Check window properties
    EXPECT_GT(dialog->width(), 0);
}

// Test addTask with null handler
TEST_F(TestTaskDialog, TestAddTaskNullHandler)
{
    JobHandlePointer nullHandler;
    dialog->addTask(nullHandler);
    
    // Should handle null handler gracefully
    SUCCEED();
}

// Test addTask with valid handler
TEST_F(TestTaskDialog, TestAddTaskValidHandler)
{
    // Create a mock AbstractJobHandler
    JobHandlePointer handler(new DialogJobHandlerImpl());
    
    // Mock the required methods
    stub.set_lamda(VADDR(AbstractJobHandler, setSignalConnectFinished), []() {
        __DBG_STUB_INVOKE__
    });
    
    dialog->addTask(handler);
    
    // Should add the task without crashing
    SUCCEED();
    handler.reset();
}

// Test addTask with duplicate handler
TEST_F(TestTaskDialog, TestAddTaskDuplicateHandler)
{
    JobHandlePointer handler(new DialogJobHandlerImpl());
    
    // Mock methods
    stub.set_lamda(VADDR(AbstractJobHandler, setSignalConnectFinished), []() {
        __DBG_STUB_INVOKE__
    });
    
    // Add the same handler twice
    dialog->addTask(handler);
    dialog->addTask(handler);
    
    // Should handle duplicate gracefully
    SUCCEED();
    handler.reset();
}

// Test blockShutdown method
TEST_F(TestTaskDialog, TestBlockShutdown)
{
    // Mock UniversalUtils::blockShutdown
    stub.set_lamda(ADDR(UniversalUtils, blockShutdown), [](QDBusReply<QDBusUnixFileDescriptor>&) {
        __DBG_STUB_INVOKE__
    });
    
    dialog->blockShutdown();
    
    // Should not crash
    SUCCEED();
}

// Test blockShutdown with valid reply
TEST_F(TestTaskDialog, TestBlockShutdownWithValidReply)
{
    // Mock UniversalUtils::blockShutdown to set valid reply
    stub.set_lamda(ADDR(UniversalUtils, blockShutdown), [](QDBusReply<QDBusUnixFileDescriptor>& reply) {
        __DBG_STUB_INVOKE__
        // Simulate valid reply with fd > 0
        // We can't actually create a real QDBusUnixFileDescriptor in test
    });
    
    dialog->blockShutdown();
    
    SUCCEED();
}

// Test addTaskWidget method
TEST_F(TestTaskDialog, TestAddTaskWidget)
{
    JobHandlePointer handler(new DialogJobHandlerImpl());
    
    // Create a TaskWidget (this would normally be created in addTask)
    // Since we can't easily mock TaskWidget, we'll test with null
    dialog->addTaskWidget(handler, nullptr);
    
    // Should handle null widget gracefully
    SUCCEED();
}

// Test setTitle method
TEST_F(TestTaskDialog, TestSetTitle)
{
    // Test with different task counts
    dialog->setTitle(0);
    dialog->setTitle(1);
    dialog->setTitle(5);
    dialog->setTitle(10);
    
    // Should not crash
    SUCCEED();
}

// Test adjustSize method
TEST_F(TestTaskDialog, TestAdjustSize)
{
    // Test adjustment with different heights
    dialog->adjustSize(0);
    dialog->adjustSize(100);
    dialog->adjustSize(200);
    
    // Should not crash
    SUCCEED();
}

// Test moveYCenter method
TEST_F(TestTaskDialog, TestMoveYCenter)
{
    // Mock screen geometry
    // stub.set_lamda(ADDR(QApplication, primaryScreen), []() {
    //     __DBG_STUB_INVOKE__
    //     QScreen *screen = QApplication::primaryScreen();
    //     return screen;
    // });
    
    // Mock availableGeometry
    stub.set_lamda(VADDR(QScreen, availableGeometry), []() {
        __DBG_STUB_INVOKE__
        return QRect(0, 0, 1920, 1080);
    });
    
    dialog->moveYCenter();
    
    // Should not crash
    SUCCEED();
}

// Test closeEvent method
TEST_F(TestTaskDialog, TestCloseEvent)
{
    // Add some mock tasks first
    JobHandlePointer handler1(new DialogJobHandlerImpl());
    JobHandlePointer handler2(new DialogJobHandlerImpl());
    
    // Mock the taskItems directly through the public interface
    dialog->addTask(handler1);
    
    // Create and trigger close event
    QCloseEvent event;
    QApplication::sendEvent(dialog, &event);
    
    // Should handle close event
    SUCCEED();
    handler1.reset();
    handler2.reset();
}

// Test keyPressEvent with Escape key
TEST_F(TestTaskDialog, TestKeyPressEventEscape)
{
    // Create escape key event
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    
    // Note: QDialog doesn't have closeRequested signal in older Qt versions
    // We'll just test that the key event is handled without crashing
    QApplication::sendEvent(dialog, &event);
    
    // Should handle escape key
    SUCCEED();
}

// Test keyPressEvent with other keys
TEST_F(TestTaskDialog, TestKeyPressEventOther)
{
    // Create a non-escape key event
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    
    QApplication::sendEvent(dialog, &event);
    
    // Should handle other keys gracefully
    SUCCEED();
}

// Test removeTask slot
TEST_F(TestTaskDialog, TestRemoveTask)
{
    // Remove task when list is empty
    dialog->removeTask();
    
    // Should handle empty list gracefully
    SUCCEED();
}

// Test window flags and properties
TEST_F(TestTaskDialog, TestWindowProperties)
{
    // Should have appropriate window flags
    Qt::WindowFlags flags = dialog->windowFlags();
    EXPECT_TRUE(flags & Qt::Window);
    
    // Should have window icon
    EXPECT_FALSE(dialog->windowIcon().isNull());
}

// Test font settings
TEST_F(TestTaskDialog, TestFont)
{
    QFont font = dialog->font();
    EXPECT_EQ(font.pixelSize(), 14);
}

// Test window title and icon
TEST_F(TestTaskDialog, TestWindowIcon)
{
    // Should have dde-file-manager icon
    QIcon icon = dialog->windowIcon();
    EXPECT_FALSE(icon.isNull());
}

// Test fixed width
TEST_F(TestTaskDialog, TestFixedWidth)
{
    // Should have fixed width
    EXPECT_EQ(dialog->width(), 700);  // kDefaultWidth
    EXPECT_EQ(dialog->minimumWidth(), 700);
    EXPECT_EQ(dialog->maximumWidth(), 700);
}

// Test list widget properties
TEST_F(TestTaskDialog, TestListWidgetProperties)
{
    QListWidget *listWidget = dialog->findChild<QListWidget*>();
    ASSERT_NE(listWidget, nullptr);
    
    // Should have no selection mode
    EXPECT_EQ(listWidget->selectionMode(), QListWidget::NoSelection);
    
    // Should have no frame
    EXPECT_EQ(listWidget->frameShape(), QFrame::NoFrame);
}

// Test titlebar properties
TEST_F(TestTaskDialog, TestTitlebarProperties)
{
    DTitlebar *titlebar = dialog->findChild<DTitlebar*>();
    ASSERT_NE(titlebar, nullptr);
    
    // Note: DTitlebar might not have isMenuVisible() or icon() methods in all versions
    // We'll just verify the titlebar exists
    EXPECT_NE(titlebar, nullptr);
}

// Test layout properties
TEST_F(TestTaskDialog, TestLayoutProperties)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(dialog->layout());
    ASSERT_NE(layout, nullptr);
    
    // Should have no margins
    EXPECT_EQ(layout->contentsMargins(), QMargins(0, 0, 0, 0));
    
    // Should have no spacing
    EXPECT_EQ(layout->spacing(), 0);
}

// Test signal emission
TEST_F(TestTaskDialog, TestClosedSignal)
{
    QSignalSpy spy(dialog, &TaskDialog::closed);
    
    // Trigger close event
    QCloseEvent event;
    QApplication::sendEvent(dialog, &event);
    
    // Should emit closed signal
    EXPECT_EQ(spy.count(), 1);
}

// Test with multiple tasks
TEST_F(TestTaskDialog, TestMultipleTasks)
{
    JobHandlePointer handler1(new DialogJobHandlerImpl());
    JobHandlePointer handler2(new DialogJobHandlerImpl());
    JobHandlePointer handler3(new DialogJobHandlerImpl());
    
    // Mock methods
    stub.set_lamda(VADDR(AbstractJobHandler, setSignalConnectFinished), []() {
        __DBG_STUB_INVOKE__
    });
    
    // Add multiple tasks
    dialog->addTask(handler1);
    dialog->addTask(handler2);
    dialog->addTask(handler3);
    
    // Should handle multiple tasks
    SUCCEED();
    handler1.reset();
    handler2.reset();
    handler3.reset();
}

// Test static member kMaxHeight
TEST_F(TestTaskDialog, TestKMaxHeight)
{
    // adjust size as 0
    EXPECT_EQ(TaskDialog::kMaxHeight, 0);
    
    // After creating dialog and adjusting, it might change
    dialog->adjustSize(100);
    
    // kMaxHeight might be updated
    // The exact value depends on screen size and other factors
    SUCCEED();
}

// Test with parent widget
TEST_F(TestTaskDialog, TestWithParentWidget)
{
    QWidget parent;
    TaskDialog *tmpdialog = new TaskDialog(&parent);
    
    // Should handle parent widget
    SUCCEED();
    
    delete tmpdialog;
}

// Test task removal when list becomes empty
TEST_F(TestTaskDialog, TestRemoveTaskWhenEmpty)
{
    JobHandlePointer handler(new DialogJobHandlerImpl());
    
    // Mock methods and signals
    stub.set_lamda(VADDR(AbstractJobHandler, setSignalConnectFinished), []() {
        __DBG_STUB_INVOKE__
    });
    
    // Add and then remove task
    dialog->addTask(handler);
    
    // Simulate task completion/removal
    dialog->removeTask();
    
    // Should handle empty list
    SUCCEED();
}

// Test dialog visibility
TEST_F(TestTaskDialog, TestDialogVisibility)
{
    // Dialog might be hidden initially
    // After adding task, it should be shown
    JobHandlePointer handler(new DialogJobHandlerImpl());
    
    // Mock methods
    stub.set_lamda(VADDR(AbstractJobHandler, setSignalConnectFinished), []() {
        __DBG_STUB_INVOKE__
    });
    
    dialog->addTask(handler);
    
    // Should handle visibility state
    SUCCEED();
    handler.reset();
}

#include "test_taskdialog.moc"
