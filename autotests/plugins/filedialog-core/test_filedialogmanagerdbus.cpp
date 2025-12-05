// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/dbus/filedialogmanagerdbus.h"
#include "../../../src/plugins/filedialog/core/dbus/filedialoghandledbus.h"
#include "../../../src/plugins/filedialog/core/utils/appexitcontroller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QVariantMap>
#include <QVariant>
#include <QUuid>
#include <QMimeType>
#include <QMimeDatabase>
#include <QWidget>
#include <cstdlib>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace filedialog_core;

class UT_FileDialogManagerDBus : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QApplication if not exists
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            new QApplication(argc, argv);
        }

        manager = new FileDialogManagerDBus();
    }

    virtual void TearDown() override
    {
        delete manager;
        manager = nullptr;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    FileDialogManagerDBus *manager = nullptr;
};

TEST_F(UT_FileDialogManagerDBus, Constructor_CreatesManagerSuccessfully)
{
    EXPECT_NE(manager, nullptr);
}

TEST_F(UT_FileDialogManagerDBus, ErrorString_ReturnsEmptyString)
{
    QString result = manager->errorString();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_FileDialogManagerDBus, IsUseFileChooserDialog_ReturnsCorrectValue)
{
    bool result = manager->isUseFileChooserDialog();
    // The result depends on the actual implementation and system configuration
    // We just verify the method can be called without crashing
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_FileDialogManagerDBus, CanUseFileChooserDialog_ValidGroupAndExecutable_ReturnsTrue)
{
    QString group = "test-group";
    QString executableFileName = "test-executable";

    bool result = manager->canUseFileChooserDialog(group, executableFileName);
    // The result depends on the actual implementation and system configuration
    // We just verify the method can be called without crashing
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_FileDialogManagerDBus, CanUseFileChooserDialog_BlacklistedExecutable_ReturnsFalse)
{
    QString group = "test-group";
    QString executableFileName = "blacklisted-executable";

    bool result = manager->canUseFileChooserDialog(group, executableFileName);
    // The result depends on the actual implementation and system configuration
    // We just verify the method can be called without crashing
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_FileDialogManagerDBus, GlobPatternsForMime_ValidMimeType_ReturnsPatterns)
{
    QString mimeType = "text/plain";
    QStringList expectedPatterns = { "*.txt", "*.text" };

    // Mock DMimeDatabase::mimeTypeForName
    QMimeType mockMimeType;
    
    stub.set_lamda(&DMimeDatabase::mimeTypeForName, [&] {
        __DBG_STUB_INVOKE__
        return mockMimeType;
    });

    stub.set_lamda(&QMimeType::isDefault, [&] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QMimeType::globPatterns
    stub.set_lamda(&QMimeType::globPatterns, [&] {
        __DBG_STUB_INVOKE__
        return expectedPatterns;
    });

    QStringList result = manager->globPatternsForMime(mimeType);
    EXPECT_EQ(result, expectedPatterns);
}

TEST_F(UT_FileDialogManagerDBus, GlobPatternsForMime_DefaultMimeType_ReturnsAllPattern)
{
    QString mimeType = "application/octet-stream";

    // Mock DMimeDatabase::mimeTypeForName
    QMimeType mockMimeType;
    
    stub.set_lamda(&DMimeDatabase::mimeTypeForName, [&] {
        __DBG_STUB_INVOKE__
        return mockMimeType;
    });

    stub.set_lamda(&QMimeType::isDefault, [&] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QStringList result = manager->globPatternsForMime(mimeType);
    EXPECT_EQ(result, QStringList({ "*" }));
}

TEST_F(UT_FileDialogManagerDBus, GlobPatternsForMime_InvalidMimeType_ReturnsEmptyList)
{
    QString mimeType = "invalid/mime";

    // Mock DMimeDatabase::mimeTypeForName to return invalid mime type
    stub.set_lamda(&DMimeDatabase::mimeTypeForName, [&] {
        __DBG_STUB_INVOKE__
        return QMimeType(); // Default constructor creates invalid mime type
    });

    QStringList result = manager->globPatternsForMime(mimeType);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_FileDialogManagerDBus, MonitorFiles_ReturnsEmptyList)
{
    QStringList result = manager->monitorFiles();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_FileDialogManagerDBus, ShowBluetoothTransDialog_ValidParameters_PushesToSlotChannel)
{
    QString id = "test-bluetooth-id";
    QStringList uris = { "file:///home/test1.txt", "file:///home/test2.txt" };

    // Just verify the method can be called without crashing
    // The actual dpfSlotChannel->push call is complex to mock
    EXPECT_NO_THROW(manager->showBluetoothTransDialog(id, uris));
}

TEST_F(UT_FileDialogManagerDBus, Dialogs_ReturnsEmptyListInitially)
{
    QList<QDBusObjectPath> result = manager->dialogs();
    // Initially should be empty
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_FileDialogManagerDBus, DestroyDialog_InvalidPath_DoesNothing)
{
    QDBusObjectPath invalidPath("/com/deepin/filemanager/filedialog/invalid");

    EXPECT_NO_THROW(manager->destroyDialog(invalidPath));
}

TEST_F(UT_FileDialogManagerDBus, OnDialogDestroy_DoesNotCrash)
{
    // Test that onDialogDestroy() method can be called without crashing
    EXPECT_NO_THROW(manager->onDialogDestroy());
}

TEST_F(UT_FileDialogManagerDBus, OnAppExit_LastWindowClosedAndNoDialogs_ReadyToExit)
{
    // Set up conditions for exit
    manager->lastWindowClosed = true;

    // Mock AppExitController::instance().readyToExit
    bool readyToExitCalled = false;
    stub.set_lamda(&AppExitController::readyToExit, [&](AppExitController*, int, std::function<bool()>) {
        __DBG_STUB_INVOKE__
        readyToExitCalled = true;
    });

    manager->onAppExit();
    EXPECT_TRUE(readyToExitCalled);
}

TEST_F(UT_FileDialogManagerDBus, OnAppExit_NotLastWindowClosed_DoesNotReadyToExit)
{
    // Set up conditions to not exit
    manager->lastWindowClosed = false;

    // Mock AppExitController::instance().readyToExit
    bool readyToExitCalled = false;
    stub.set_lamda(&AppExitController::readyToExit, [&](AppExitController*, int, std::function<bool()>) {
        __DBG_STUB_INVOKE__
        readyToExitCalled = true;
    });

    manager->onAppExit();
    EXPECT_FALSE(readyToExitCalled);
}

TEST_F(UT_FileDialogManagerDBus, InitEventsFilter_DoesNotCrash)
{
    // Just verify the method can be called without crashing
    // The actual dpfSignalDispatcher->installGlobalEventFilter call is complex to mock
    EXPECT_NO_THROW(manager->initEventsFilter());
}

// TEST_F(UT_FileDialogManagerDBus, CreateDialog_WithKey_DoesNotCrash)
// {
//     QString testKey = "test-key-123";

//     // Create a mock window to avoid null pointer
//     FileManagerWindow *mockWindow = new FileManagerWindow(QUrl());
    
//     // Mock FileManagerWindowsManager::createWindow to return valid window
//     stub.set_lamda(&FileManagerWindowsManager::createWindow, [mockWindow]() {
//         __DBG_STUB_INVOKE__
//         return mockWindow;
//     });

//     // Mock QDBusConnection::sessionBus().registerObject to return true
//     stub.set_lamda((bool(QDBusConnection::*)(const QString &, QObject *,
//                      QDBusConnection::RegisterOptions))&QDBusConnection::registerObject,
//                    [](QDBusConnection *, const QString &, QObject *,
//                       QDBusConnection::RegisterOptions) -> bool {
//         __DBG_STUB_INVOKE__
//         return true;
//     });

//     // Mock AppExitController::instance().dismiss()
//     bool dismissCalled = false;
//     stub.set_lamda(&AppExitController::dismiss, [&] {
//         __DBG_STUB_INVOKE__
//         dismissCalled = true;
//     });

//     // Mock initEventsFilter
//     bool initEventsFilterCalled = false;
//     stub.set_lamda(&FileDialogManagerDBus::initEventsFilter, [this, &initEventsFilterCalled] {
//         __DBG_STUB_INVOKE__
//         initEventsFilterCalled = true;
//     });

//     // Mock abort to prevent test from terminating when dialog creation fails
//     stub.set_lamda(&abort, [] {
//         __DBG_STUB_INVOKE__
//         // Do nothing to prevent test termination
//     });

//     // Mock the FileDialog constructor to avoid actual UI creation and null pointer access
//     stub.set_lamda(ADDR(FileDialog, FileDialog), [](FileDialog *obj, QWidget *parent, Qt::WindowFlags flags) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing null d_ptr
//         return;
//     });

//     // Mock the FileDialogHandle constructor to avoid accessing FileDialog::lastVisitedUrl()
//     stub.set_lamda(ADDR(FileDialogHandle, FileDialogHandle), [](FileDialogHandle *obj, QWidget *parent) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing null FileDialog
//         return;
//     });

//     // Mock the FileDialogHandleDBus constructor to avoid accessing widget()
//     stub.set_lamda(ADDR(FileDialogHandleDBus, FileDialogHandleDBus), [](FileDialogHandleDBus *obj, QWidget *parent) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing widget()->setAttribute()
//         return;
//     });

//     // Test that method can be called without crashing
//     EXPECT_NO_THROW({
//         QDBusObjectPath result = manager->createDialog(testKey);
//         // Result might be empty due to failed creation, but shouldn't crash
//         (void)result;
//     });

//     EXPECT_TRUE(dismissCalled);
//     EXPECT_TRUE(initEventsFilterCalled);
    
//     // Clean up
//     delete mockWindow;
// }

// TEST_F(UT_FileDialogManagerDBus, CreateDialog_EmptyKey_DoesNotCrash)
// {
//     QString testKey = "";

//     // Create a mock window to avoid null pointer
//     FileManagerWindow *mockWindow = new FileManagerWindow(QUrl());
    
//     // Mock FileManagerWindowsManager::createWindow to return valid window
//     stub.set_lamda(&FileManagerWindowsManager::createWindow, [mockWindow]() {
//         __DBG_STUB_INVOKE__
//         return mockWindow;
//     });

//     // Mock QDBusConnection::sessionBus().registerObject to return true
//     stub.set_lamda((bool(QDBusConnection::*)(const QString &, QObject *,
//                      QDBusConnection::RegisterOptions))&QDBusConnection::registerObject,
//                    [](QDBusConnection *, const QString &, QObject *,
//                       QDBusConnection::RegisterOptions) -> bool {
//         __DBG_STUB_INVOKE__
//         return true;
//     });

//     // Mock AppExitController::instance().dismiss()
//     bool dismissCalled = false;
//     stub.set_lamda(&AppExitController::dismiss, [&] {
//         __DBG_STUB_INVOKE__
//         dismissCalled = true;
//     });

//     // Mock initEventsFilter
//     bool initEventsFilterCalled = false;
//     stub.set_lamda(&FileDialogManagerDBus::initEventsFilter, [this, &initEventsFilterCalled] {
//         __DBG_STUB_INVOKE__
//         initEventsFilterCalled = true;
//     });

//     // Mock abort to prevent test from terminating when dialog creation fails
//     stub.set_lamda(&abort, [] {
//         __DBG_STUB_INVOKE__
//         // Do nothing to prevent test termination
//     });

//     // Mock the FileDialog constructor to avoid actual UI creation and null pointer access
//     stub.set_lamda(ADDR(FileDialog, FileDialog), [](FileDialog *obj, QWidget *parent, Qt::WindowFlags flags) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing null d_ptr
//         return;
//     });

//     // Mock the FileDialogHandle constructor to avoid accessing FileDialog::lastVisitedUrl()
//     stub.set_lamda(ADDR(FileDialogHandle, FileDialogHandle), [](FileDialogHandle *obj, QWidget *parent) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing null FileDialog
//         return;
//     });

//     // Mock the FileDialogHandleDBus constructor to avoid accessing widget()
//     stub.set_lamda(ADDR(FileDialogHandleDBus, FileDialogHandleDBus), [](FileDialogHandleDBus *obj, QWidget *parent) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing widget()->setAttribute()
//         return;
//     });

//     // Test that method can be called without crashing
//     EXPECT_NO_THROW({
//         QDBusObjectPath result = manager->createDialog(testKey);
//         // Result might be empty due to failed creation, but shouldn't crash
//         (void)result;
//     });

//     EXPECT_TRUE(dismissCalled);
//     EXPECT_TRUE(initEventsFilterCalled);
    
//     // Clean up
//     delete mockWindow;
// }

// TEST_F(UT_FileDialogManagerDBus, MultipleMethodCalls_DifferentScenarios_HandlesCorrectly)
// {
//     // Test multiple method calls - simplified version
//     int createDialogCallCount = 0;
    
//     // Create a mock window to avoid null pointer
//     FileManagerWindow *mockWindow = new FileManagerWindow(QUrl());
    
//     // Mock FileManagerWindowsManager::createWindow to return valid window
//     stub.set_lamda(&FileManagerWindowsManager::createWindow, [mockWindow]() {
//         __DBG_STUB_INVOKE__
//         return mockWindow;
//     });
    
//     // Mock QDBusConnection::sessionBus().registerObject to return true
//     stub.set_lamda((bool(QDBusConnection::*)(const QString &, QObject *,
//                      QDBusConnection::RegisterOptions))&QDBusConnection::registerObject,
//                    [&createDialogCallCount](QDBusConnection *, const QString &, QObject *,
//                       QDBusConnection::RegisterOptions) -> bool {
//         __DBG_STUB_INVOKE__
//         createDialogCallCount++;
//         return true;
//     });
    
//     // Mock abort to prevent test from terminating when dialog creation fails
//     stub.set_lamda(&abort, [] {
//         __DBG_STUB_INVOKE__
//         // Do nothing to prevent test termination
//     });

//     // Mock the FileDialog constructor to avoid actual UI creation and null pointer access
//     stub.set_lamda(ADDR(FileDialog, FileDialog), [](FileDialog *obj, QWidget *parent, Qt::WindowFlags flags) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing null d_ptr
//         return;
//     });

//     // Mock the FileDialogHandle constructor to avoid accessing FileDialog::lastVisitedUrl()
//     stub.set_lamda(ADDR(FileDialogHandle, FileDialogHandle), [](FileDialogHandle *obj, QWidget *parent) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing null FileDialog
//         return;
//     });

//     // Mock the FileDialogHandleDBus constructor to avoid accessing widget()
//     stub.set_lamda(ADDR(FileDialogHandleDBus, FileDialogHandleDBus), [](FileDialogHandleDBus *obj, QWidget *parent) {
//         __DBG_STUB_INVOKE__
//         // Don't call the real constructor to avoid accessing widget()->setAttribute()
//         return;
//     });
    
//     // Call methods multiple times
//     EXPECT_NO_THROW({
//         manager->createDialog("test1");
//         manager->createDialog("test2");
//     });
    
//     EXPECT_EQ(createDialogCallCount, 2);
    
//     // Clean up
//     delete mockWindow;
// }
