// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QApplication>
#include <QDialog>
#include <QTimer>
#include <QThread>
#include <DDialog>
#include <DSettingsDialog>

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/private/application_p.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/dialogs/mountpasswddialog/mountsecretdiskaskpassworddialog.h>
#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/dialogs/taskdialog/taskdialog.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <DDialog>
#include <DSettingsDialog>

// Include stub headers
#include "stubext.h"

using namespace dfmbase;

class TestDialogManager : public testing::Test {
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
    }

    void TearDown() override {
        // Cleanup code after each test
        // 先清理可能的设置对话框相关资源
        // 然后清除stub
        stub.clear();
        
        // Clean up the Application instance to avoid "there should be only one application object" assertion
        stub.set_lamda(&Application::instance, []() -> Application * {
            return nullptr;
        });
    }

public:
    stub_ext::StubExt stub;
    Application *app = nullptr;
};

// Test instance singleton
TEST_F(TestDialogManager, TestInstance) {
    DialogManager *instance1 = DialogManager::instance();
    DialogManager *instance2 = DialogManager::instance();
    
    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

// Test showQueryScanningDialog
TEST_F(TestDialogManager, TestShowQueryScanningDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec to avoid showing actual dialog
    bool dialogCreated = false;
    stub.set_lamda(ADDR(DDialog, show), [&]() {
        dialogCreated = true;
    });
    
    DDialog *dialog = manager->showQueryScanningDialog("Test Title");
    
    EXPECT_NE(dialog, nullptr);
    EXPECT_TRUE(dialogCreated);
    
    // Clean up
    delete dialog;
}

// Test showErrorDialog
TEST_F(TestDialogManager, TestShowErrorDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec to avoid showing actual dialog
    bool dialogExecuted = false;
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        dialogExecuted = true;
        return 0;
    });
    
    manager->showErrorDialog("Error Title", "Error Message");
    
    EXPECT_TRUE(dialogExecuted);
}

// Test showMessageDialog with single button
TEST_F(TestDialogManager, TestShowMessageDialogSingleButton) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec to return specific code
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1;
    });
    
    int result = manager->showMessageDialog("Title", "Message", "OK");
    
    EXPECT_EQ(result, 1);
}

// Test showMessageDialog with multiple buttons
TEST_F(TestDialogManager, TestShowMessageDialogMultipleButtons) {
    DialogManager *manager = DialogManager::instance();
    
    QStringList buttons = {"Button1", "Button2", "Button3"};
    
    // Mock DDialog's exec to return specific code
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 2;
    });
    
    int result = manager->showMessageDialog("Title", "Message", buttons);
    
    EXPECT_EQ(result, 2);
}

// Test showErrorDialogWhenOperateDeviceFailed
TEST_F(TestDialogManager, TestShowErrorDialogWhenOperateDeviceFailed) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock showErrorDialog to track calls
    QString capturedTitle, capturedMessage;
    stub.set_lamda((void(DialogManager::*)(const QString&, const QString&))&DialogManager::showErrorDialog,
                   [&](DialogManager*, const QString &title, const QString &message) {
        capturedTitle = title;
        capturedMessage = message;
    });
    
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorNetworkWrongPasswd;
    
    manager->showErrorDialogWhenOperateDeviceFailed(DialogManager::OperateType::kMount, err);
    
    EXPECT_EQ(capturedTitle, "Mount failed");
    EXPECT_FALSE(capturedMessage.isEmpty());
}

// Test showNoPermissionDialog with single URL
TEST_F(TestDialogManager, TestShowNoPermissionDialogSingleUrl) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/test/file.txt");
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash
    manager->showNoPermissionDialog(urls);
}

// Test showNoPermissionDialog with multiple URLs
TEST_F(TestDialogManager, TestShowNoPermissionDialogMultipleUrls) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> urls;
    for (int i = 0; i < 15; ++i) {
        urls << QUrl::fromLocalFile(QString("/test/file%1.txt").arg(i));
    }
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash even with many URLs
    manager->showNoPermissionDialog(urls);
}

// Test showNoPermissionDialog with empty list
TEST_F(TestDialogManager, TestShowNoPermissionDialogEmptyList) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> emptyUrls;
    
    // Should return early without showing dialog
    manager->showNoPermissionDialog(emptyUrls);
}

// Test showCopyMoveToSelfDialog
TEST_F(TestDialogManager, TestShowCopyMoveToSelfDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash
    manager->showCopyMoveToSelfDialog();
}

// Test addTask
TEST_F(TestDialogManager, TestAddTask) {
    DialogManager *manager = DialogManager::instance();
    
    // Create a mock task
    JobHandlePointer task = nullptr; // In real test, this would be a valid task
    
    // Should not crash even with null task
    manager->addTask(task);
}

// Test registerSettingWidget and showSetingsDialog
TEST_F(TestDialogManager, TestRegisterAndShowSettingDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Register a mock widget creator
    bool creatorCalled = false;
    manager->registerSettingWidget("testWidget", [&](QObject *parent) -> QWidget* {
        creatorCalled = true;
        return new QWidget(qobject_cast<QWidget*>(parent));
    });
    
    // Mock FileManagerWindow
    QUrl url("file:///tmp");
    FileManagerWindow *window = new FileManagerWindow(url);
    window->setProperty("isSettingDialogShown", false);
    
    // Mock SettingDialog
    stub.set_lamda(VADDR(DSettingsDialog, show), [&]() {
        // Do nothing
    });
    
    stub.set_lamda(VADDR(DSettingsDialog, exec), [&]() {
        return 0;
    });

    stub.set_lamda(&SettingBackend::setToSettings,
                   [](SettingBackend *backend, DSettings *settings) {
        __DBG_STUB_INVOKE__
    });
    
    // Should not crash
    manager->showSetingsDialog(window);
    
    // The property should be set
    EXPECT_TRUE(window->property("isSettingDialogShown").toBool());
    
    // 确保窗口的SettingDialog相关资源被清理
    // 等待可能的异步操作完成，然后重置属性
    if (window->property("isSettingDialogShown").toBool()) {
        window->setProperty("isSettingDialogShown", false);
    }
    delete window;
}

// Test askPasswordForLockedDevice
TEST_F(TestDialogManager, TestAskPasswordForLockedDevice) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock MountSecretDiskAskPasswordDialog
    stub.set_lamda(VADDR(QDialog, exec), [&]() {
        return QDialog::Accepted;
    });
    
    QString password = "testpassword";
    stub.set_lamda(VADDR(MountSecretDiskAskPasswordDialog, getUerInputedPassword), [&]() {
        return password;
    });
    
    QString result = manager->askPasswordForLockedDevice("/dev/sdb1");
    
    EXPECT_EQ(result, password);
}

// Test askForFormat - user accepts
TEST_F(TestDialogManager, TestAskForFormatAccept) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec to return Accepted
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return QDialog::Accepted;
    });
    
    bool result = manager->askForFormat();
    
    EXPECT_TRUE(result);
}

// Test askForFormat - user rejects
TEST_F(TestDialogManager, TestAskForFormatReject) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec to return Rejected
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return QDialog::Rejected;
    });
    
    bool result = manager->askForFormat();
    
    EXPECT_FALSE(result);
}

// Test showRunExcutableScriptDialog
TEST_F(TestDialogManager, TestShowRunExcutableScriptDialog) {
    DialogManager *manager = DialogManager::instance();
    
    QUrl scriptUrl = QUrl::fromLocalFile("/test/script.sh");
    
    // Mock InfoFactory::create
    stub.set_lamda((FileInfoPointer(*)(const QUrl&))&InfoFactory::create<FileInfo>,
                   [](const QUrl &url) -> FileInfoPointer {
        auto info = QSharedPointer<SyncFileInfo>::create(url);
        return info;
    });
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 3; // Display option
    });
    
    int result = manager->showRunExcutableScriptDialog(scriptUrl);
    
    EXPECT_EQ(result, 3);
}

// Test showRunExcutableFileDialog
TEST_F(TestDialogManager, TestShowRunExcutableFileDialog) {
    DialogManager *manager = DialogManager::instance();
    
    QUrl exeUrl = QUrl::fromLocalFile("/test/executable");
    
    // Mock InfoFactory::create
    stub.set_lamda((FileInfoPointer(*)(const QUrl&))&InfoFactory::create<FileInfo>,
                   [](const QUrl &url) -> FileInfoPointer {
        auto info = QSharedPointer<SyncFileInfo>::create(url);
        return info;
    });
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 2; // Run option
    });
    
    int result = manager->showRunExcutableFileDialog(exeUrl);
    
    EXPECT_EQ(result, 2);
}

// Test showDeleteFilesDialog with single file
TEST_F(TestDialogManager, TestShowDeleteFilesDialogSingle) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/test/file.txt");
    
    // Mock SyncFileInfo
    stub.set_lamda(VADDR(SyncFileInfo, displayOf), [&](SyncFileInfo*, DisPlayInfoType type) -> QString {
        if (type == DisPlayInfoType::kFileDisplayName)
            return "file.txt";
        return QString();
    });
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1; // Delete option
    });
    
    int result = manager->showDeleteFilesDialog(urls, false);
    
    EXPECT_EQ(result, 1);
}

// Test showDeleteFilesDialog with multiple files
TEST_F(TestDialogManager, TestShowDeleteFilesDialogMultiple) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/test/file1.txt");
    urls << QUrl::fromLocalFile("/test/file2.txt");
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0; // Cancel option
    });
    
    int result = manager->showDeleteFilesDialog(urls, false);
    
    EXPECT_EQ(result, 0);
}

// Test showDeleteFilesDialog with empty list
TEST_F(TestDialogManager, TestShowDeleteFilesDialogEmpty) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> emptyUrls;
    
    int result = manager->showDeleteFilesDialog(emptyUrls, false);
    
    EXPECT_EQ(result, QDialog::Rejected);
}

// Test showClearTrashDialog
TEST_F(TestDialogManager, TestShowClearTrashDialog) {
    DialogManager *manager = DialogManager::instance();
    
    quint64 count = 10;
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1; // Clear option
    });
    
    int result = manager->showClearTrashDialog(count);
    
    EXPECT_EQ(result, 1);
}

// Test showNormalDeleteConfirmDialog
TEST_F(TestDialogManager, TestShowNormalDeleteConfirmDialog) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/test/file.txt");

    // Mock InfoFactory::create to return a valid FileInfo
    stub.set_lamda((FileInfoPointer(*)(const QUrl&))&InfoFactory::create<FileInfo>,
                   [](const QUrl &url) -> FileInfoPointer {
        auto info = QSharedPointer<SyncFileInfo>::create(url);
        return info;
    });

    // Mock SyncFileInfo's displayOf method to prevent null pointer access
    stub.set_lamda(VADDR(SyncFileInfo, displayOf),
                   [](SyncFileInfo*, DisPlayInfoType type) -> QString {
        if (type == DisPlayInfoType::kFileDisplayName)
            return "file.txt";
        return QString();
    });
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1; // Delete option
    });
    
    int result = manager->showNormalDeleteConfirmDialog(urls);
    
    EXPECT_EQ(result, 1);
}

// // Test showNormalDeleteConfirmDialog with null FileInfo (to prevent crash)
// TEST_F(TestDialogManager, TestShowNormalDeleteConfirmDialogNullInfo) {
//     DialogManager *manager = DialogManager::instance();
    
//     QList<QUrl> urls;
//     urls << QUrl::fromLocalFile("/test/file.txt");
    
//     // Mock InfoFactory::create to return null pointer to test null handling
//     stub.set_lamda((FileInfoPointer(*)(const QUrl&))&InfoFactory::create<FileInfo>,
//                    [](const QUrl &url) -> FileInfoPointer {
//         return FileInfoPointer(); // Return null
//     });
    
//     // Mock DDialog's exec
//     stub.set_lamda(VADDR(DDialog, exec), [&]() {
//         return 1; // Delete option
//     });
    
//     // This should not crash even if info is null
//     int result = manager->showNormalDeleteConfirmDialog(urls);
    
//     // The call should succeed (not crash) even with null info
//     EXPECT_EQ(result, 1);
// }

// Test showRestoreFailedDialog
TEST_F(TestDialogManager, TestShowRestoreFailedDialog) {
    DialogManager *manager = DialogManager::instance();
    
    int count = 5;
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash
    manager->showRestoreFailedDialog(count);
}

// Test showRestoreDeleteFilesDialog
TEST_F(TestDialogManager, TestShowRestoreDeleteFilesDialog) {
    DialogManager *manager = DialogManager::instance();
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/test/file.txt");
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0; // Cancel option
    });
    
    int result = manager->showRestoreDeleteFilesDialog(urls);
    
    EXPECT_EQ(result, 0);
}

// Test showRenameNameSameErrorDialog
TEST_F(TestDialogManager, TestShowRenameNameSameErrorDialog) {
    DialogManager *manager = DialogManager::instance();
    
    QString name = "test.txt";
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    int result = manager->showRenameNameSameErrorDialog(name);
    
    EXPECT_EQ(result, 0);
}

// Test showRenameBusyErrDialog
TEST_F(TestDialogManager, TestShowRenameBusyErrDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash
    manager->showRenameBusyErrDialog();
}

// Test showRenameNameDotBeginDialog
TEST_F(TestDialogManager, TestShowRenameNameDotBeginDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1;
    });
    
    int result = manager->showRenameNameDotBeginDialog();
    
    EXPECT_EQ(result, -1); // ret not set by exec
}

// Test showUnableToVistDir
TEST_F(TestDialogManager, TestShowUnableToVistDir) {
    DialogManager *manager = DialogManager::instance();
    
    QString dir = "/test/dir";
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash
    manager->showUnableToVistDir(dir);
}

// Test showBreakSymlinkDialog
TEST_F(TestDialogManager, TestShowBreakSymlinkDialog) {
    DialogManager *manager = DialogManager::instance();
    
    QString targetName = "target.txt";
    QUrl linkfile = QUrl::fromLocalFile("/test/link");
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1; // Delete option
    });
    
    GlobalEventType result = manager->showBreakSymlinkDialog(targetName, linkfile);
    
    EXPECT_EQ(result, GlobalEventType::kMoveToTrash);
}

// Test showAskIfAddExcutableFlagAndRunDialog
TEST_F(TestDialogManager, TestShowAskIfAddExcutableFlagAndRunDialog) {
    DialogManager *manager = DialogManager::instance();
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 1; // Add flag and run
    });
    
    int result = manager->showAskIfAddExcutableFlagAndRunDialog();
    
    EXPECT_EQ(result, 1);
}

// Test showDeleteSystemPathWarnDialog
TEST_F(TestDialogManager, TestShowDeleteSystemPathWarnDialog) {
    DialogManager *manager = DialogManager::instance();
    
    quint64 winId = 12345;
    
    // Mock DDialog's exec
    stub.set_lamda(VADDR(DDialog, exec), [&]() {
        return 0;
    });
    
    // Should not crash
    manager->showDeleteSystemPathWarnDialog(winId);
}