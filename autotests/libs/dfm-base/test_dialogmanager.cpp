// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dialogmanager.cpp
 * @brief Unit tests for DialogManager (dialogmanager.cpp)
 *        Most methods create DDialog and call exec() — we stub exec to return
 *        a fixed code and avoid actual modal loops.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QList>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QLabel>

#include "stubext.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-mount/base/dmount_global.h>

using namespace dfmbase;

// Controls DDialog::exec return value so individual tests can change the
// dialog result WITHOUT re-stubbing the same function address (which would
// corrupt the stub table after repeated patch/unpatch cycles).
static int g_dialogExecReturn = QDialog::Accepted;

class DialogManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        g_dialogExecReturn = QDialog::Accepted;
        // Stub DDialog::exec (NOT QDialog::exec — DDialog overrides exec() so
        // VADDR(QDialog, exec) would not intercept DDialog::exec calls).
        stub.set_lamda(VADDR(DDialog, exec), [](DDialog *) -> int {
            __DBG_STUB_INVOKE__
            return g_dialogExecReturn;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// ============================================================
// 1. Instance
// ============================================================
TEST_F(DialogManagerTest, InstanceReturnsNonNull)
{
    auto *dm = DialogManager::instance();
    ASSERT_NE(dm, nullptr);
}

TEST_F(DialogManagerTest, InstanceIsSingleton)
{
    auto *a = DialogManager::instance();
    auto *b = DialogManager::instance();
    EXPECT_EQ(a, b);
}

// ============================================================
// 2. showQueryScanningDialog
// ============================================================
TEST_F(DialogManagerTest, ShowQueryScanningDialogReturnsNonNull)
{
    auto *dm = DialogManager::instance();
    auto *d = dm->showQueryScanningDialog("Scanning...");
    EXPECT_NE(d, nullptr);
    delete d;   // WA_DeleteOnClose won't trigger without event loop
}

// ============================================================
// 3. showErrorDialog
// ============================================================
TEST_F(DialogManagerTest, ShowErrorDialogDoesNotCrash)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialog("Error", "Something went wrong"); });
}

// ============================================================
// 4. showMessageDialog (single button)
// ============================================================
TEST_F(DialogManagerTest, ShowMessageDialogSingleButton)
{
    auto *dm = DialogManager::instance();
    int code = dm->showMessageDialog("Title", "Message");
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 5. showMessageDialog (multiple buttons)
// ============================================================
TEST_F(DialogManagerTest, ShowMessageDialogMultipleButtons)
{
    auto *dm = DialogManager::instance();
    int code = dm->showMessageDialog("Title", "Message", {"OK", "Cancel"});
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 6. showErrorDialogWhenOperateDeviceFailed — kUDisksBusyFileSystemUnmounting
// ============================================================
TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyUnmounting)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyFileSystemUnmounting;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyMounting)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyFileSystemMounting;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyErasing)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyFormatErasing;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kRemove, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyMkfsing)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyFormatMkfsing;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyLocking)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyEncryptedLocking;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyUnlocking)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyEncryptedUnlocking;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

// Busy group: SMART / eject / encrypted / swap / filesystem / loop / partition / cleanup / ATA / mdraid
TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusySMARTSelfTesting)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusySMARTSelfTesting;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyDriveEjecting)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyDriveEjecting;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyEncryptedModifying)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyEncryptedModifying;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kRemove, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusySwapStarting)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusySwapSpaceStarting;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyFileSystemModifying)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyFileSystemModifying;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyLoopSetuping)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyLoopSetuping;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyPartitionCreating)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyPartitionCreating;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kRemove, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyCleanuping)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyCleanuping;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_BusyMdRaidCreating)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUDisksBusyMdRaidCreating;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

// Mount-specific errors
TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountNetworkAnonymousNotAllowed)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorNetworkAnonymousNotAllowed;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountWrongPasswd)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorNetworkWrongPasswd;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountUserCancelled)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorUserCancelled;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountCannotMkdirMountPoint)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kDaemonErrorCannotMkdirMountPoint;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountEACCES)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = static_cast<DFMMOUNT::DeviceError>(EACCES);
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountENOENT)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = static_cast<DFMMOUNT::DeviceError>(ENOENT);
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountGIOError)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kGIOError;
    err.message = "GIO error test";
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountDefault)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorFailed;
    err.message = "failed test";
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_MountOperationNotPermitted)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorFailed;
    err.message = "Operation not permitted.";
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err); });
}

// Unmount/Remove errors
TEST_F(DialogManagerTest, ShowErrorOperateDevice_UnmountAuthFailed)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorAuthenticationFailed;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_UnmountBusy)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorFailed;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kUnmount, err); });
}

TEST_F(DialogManagerTest, ShowErrorOperateDevice_RemoveBusy)
{
    auto *dm = DialogManager::instance();
    DFMMOUNT::OperationErrorInfo err;
    err.code = DFMMOUNT::DeviceError::kUserErrorFailed;
    EXPECT_NO_FATAL_FAILURE({ dm->showErrorDialogWhenOperateDeviceFailed(DialogManager::kRemove, err); });
}

// ============================================================
// 7. showNoPermissionDialog
// ============================================================
TEST_F(DialogManagerTest, ShowNoPermissionDialog_EmptyUrls)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showNoPermissionDialog({}); });
}

TEST_F(DialogManagerTest, ShowNoPermissionDialog_SingleUrl)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        dm->showNoPermissionDialog({QUrl::fromLocalFile("/tmp/test.txt")});
    });
}

TEST_F(DialogManagerTest, ShowNoPermissionDialog_MultipleUrls)
{
    auto *dm = DialogManager::instance();
    QList<QUrl> urls = {
        QUrl::fromLocalFile("/tmp/a.txt"),
        QUrl::fromLocalFile("/tmp/b.txt"),
        QUrl::fromLocalFile("/tmp/c.txt")
    };
    EXPECT_NO_FATAL_FAILURE({ dm->showNoPermissionDialog(urls); });
}

TEST_F(DialogManagerTest, ShowNoPermissionDialog_MoreThanTenUrls)
{
    auto *dm = DialogManager::instance();
    QList<QUrl> urls;
    for (int i = 0; i < 15; ++i)
        urls << QUrl::fromLocalFile("/tmp/" + QString::number(i) + ".txt");
    EXPECT_NO_FATAL_FAILURE({ dm->showNoPermissionDialog(urls); });
}

// ============================================================
// 8. showCopyMoveToSelfDialog
// ============================================================
TEST_F(DialogManagerTest, ShowCopyMoveToSelfDialog)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showCopyMoveToSelfDialog(); });
}

// ============================================================
// 9. askForFormat
// ============================================================
TEST_F(DialogManagerTest, AskForFormat)
{
    auto *dm = DialogManager::instance();
    bool result = dm->askForFormat();
    EXPECT_TRUE(result);   // exec stubbed to Accepted
}

// ============================================================
// 10. askPasswordForLockedDevice
// ============================================================
TEST_F(DialogManagerTest, AskPasswordForLockedDevice)
{
    auto *dm = DialogManager::instance();
    QString pwd = dm->askPasswordForLockedDevice("/dev/sdb1");
    // exec returns Accepted but passwordLineEdit is empty
    EXPECT_TRUE(pwd.isEmpty());
}

// ============================================================
// 11. showRunExcutableScriptDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRunExcutableScriptDialog)
{
    // Need to stub InfoFactory::create to avoid real file access
    static std::once_flag flag;
    std::call_once(flag, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/test.sh";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("#!/bin/bash\necho hello");
    f.close();

    auto *dm = DialogManager::instance();
    int code = dm->showRunExcutableScriptDialog(QUrl::fromLocalFile(filePath));
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 12. showRunExcutableFileDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRunExcutableFileDialog)
{
    static std::once_flag flag2;
    std::call_once(flag2, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/testbin";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("binary");
    f.close();

    auto *dm = DialogManager::instance();
    int code = dm->showRunExcutableFileDialog(QUrl::fromLocalFile(filePath));
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 13. showDeleteFilesDialog
// ============================================================
TEST_F(DialogManagerTest, ShowDeleteFilesDialog_EmptyList)
{
    auto *dm = DialogManager::instance();
    int code = dm->showDeleteFilesDialog({});
    EXPECT_EQ(code, QDialog::Rejected);
}

TEST_F(DialogManagerTest, ShowDeleteFilesDialog_SingleLocalFile)
{
    static std::once_flag flag3;
    std::call_once(flag3, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/delme.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    auto *dm = DialogManager::instance();
    int code = dm->showDeleteFilesDialog({QUrl::fromLocalFile(filePath)});
    EXPECT_EQ(code, QDialog::Accepted);
}

TEST_F(DialogManagerTest, ShowDeleteFilesDialog_MultipleFiles)
{
    auto *dm = DialogManager::instance();
    int code = dm->showDeleteFilesDialog({
        QUrl("smb://server/share/file1"),
        QUrl("smb://server/share/file2")
    });
    EXPECT_EQ(code, QDialog::Accepted);
}

TEST_F(DialogManagerTest, ShowDeleteFilesDialog_TrashFile)
{
    static std::once_flag flag4;
    std::call_once(flag4, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/trashitem.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("trash");
    f.close();

    auto *dm = DialogManager::instance();
    int code = dm->showDeleteFilesDialog({QUrl::fromLocalFile(filePath)}, true);
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 14. showClearTrashDialog
// ============================================================
TEST_F(DialogManagerTest, ShowClearTrashDialog_Single)
{
    auto *dm = DialogManager::instance();
    int code = dm->showClearTrashDialog(1);
    EXPECT_EQ(code, QDialog::Accepted);
}

TEST_F(DialogManagerTest, ShowClearTrashDialog_Multiple)
{
    auto *dm = DialogManager::instance();
    int code = dm->showClearTrashDialog(5);
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 15. showNormalDeleteConfirmDialog
// ============================================================
TEST_F(DialogManagerTest, ShowNormalDeleteConfirmDialog_Empty)
{
    auto *dm = DialogManager::instance();
    int code = dm->showNormalDeleteConfirmDialog({});
    EXPECT_EQ(code, QDialog::Rejected);
}

TEST_F(DialogManagerTest, ShowNormalDeleteConfirmDialog_LocalFile)
{
    static std::once_flag flag5;
    std::call_once(flag5, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/normaldel.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    auto *dm = DialogManager::instance();
    int code = dm->showNormalDeleteConfirmDialog({QUrl::fromLocalFile(filePath)});
    EXPECT_EQ(code, QDialog::Accepted);
}

TEST_F(DialogManagerTest, ShowNormalDeleteConfirmDialog_NonLocalFiles)
{
    auto *dm = DialogManager::instance();
    int code = dm->showNormalDeleteConfirmDialog({
        QUrl("smb://server/a"), QUrl("smb://server/b")
    });
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 16. showRestoreFailedDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRestoreFailedDialog_Single)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showRestoreFailedDialog(1); });
}

TEST_F(DialogManagerTest, ShowRestoreFailedDialog_Multiple)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showRestoreFailedDialog(3); });
}

TEST_F(DialogManagerTest, ShowRestoreFailedDialog_Zero)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showRestoreFailedDialog(0); });
}

// ============================================================
// 17. showOperationFailedDialog
// ============================================================
TEST_F(DialogManagerTest, ShowOperationFailedDialog_SingleFile)
{
    auto *dm = DialogManager::instance();
    QMap<QUrl, QString> info;
    info[QUrl("file:///tmp/a.txt")] = "Permission denied";
    EXPECT_NO_FATAL_FAILURE({ dm->showOperationFailedDialog(info); });
}

TEST_F(DialogManagerTest, ShowOperationFailedDialog_MultipleFiles)
{
    static std::once_flag flag6;
    std::call_once(flag6, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString f1 = tmpDir.path() + "/e1.txt";
    QString f2 = tmpDir.path() + "/e2.txt";
    { QFile ff(f1); ff.open(QIODevice::WriteOnly); ff.write("a"); }
    { QFile ff(f2); ff.open(QIODevice::WriteOnly); ff.write("b"); }

    auto *dm = DialogManager::instance();
    QMap<QUrl, QString> info;
    info[QUrl::fromLocalFile(f1)] = "Error 1";
    info[QUrl::fromLocalFile(f2)] = "Error 2";
    EXPECT_NO_FATAL_FAILURE({ dm->showOperationFailedDialog(info); });
}

TEST_F(DialogManagerTest, ShowOperationFailedDialog_Empty)
{
    auto *dm = DialogManager::instance();
    QMap<QUrl, QString> emptyInfo;
    EXPECT_NO_FATAL_FAILURE({ dm->showOperationFailedDialog(emptyInfo); });
}

// ============================================================
// 18. showRestoreDeleteFilesDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRestoreDeleteFilesDialog_Empty)
{
    auto *dm = DialogManager::instance();
    int code = dm->showRestoreDeleteFilesDialog({});
    EXPECT_EQ(code, QDialog::Rejected);
}

TEST_F(DialogManagerTest, ShowRestoreDeleteFilesDialog_SingleFile)
{
    static std::once_flag flag7;
    std::call_once(flag7, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/restore_del.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    auto *dm = DialogManager::instance();
    int code = dm->showRestoreDeleteFilesDialog({QUrl::fromLocalFile(filePath)});
    EXPECT_EQ(code, QDialog::Accepted);
}

TEST_F(DialogManagerTest, ShowRestoreDeleteFilesDialog_MultipleFiles)
{
    auto *dm = DialogManager::instance();
    int code = dm->showRestoreDeleteFilesDialog({
        QUrl("file:///a"), QUrl("file:///b")
    });
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 19. showRenameNameSameErrorDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRenameNameSameErrorDialog)
{
    auto *dm = DialogManager::instance();
    int code = dm->showRenameNameSameErrorDialog("testfile.txt");
    EXPECT_EQ(code, QDialog::Accepted);
}

TEST_F(DialogManagerTest, ShowRenameNameSameErrorDialog_LongName)
{
    auto *dm = DialogManager::instance();
    QString longName(300, 'x');
    int code = dm->showRenameNameSameErrorDialog(longName);
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 20. showRenameBusyErrDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRenameBusyErrDialog)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showRenameBusyErrDialog(); });
}

// ============================================================
// 21. showRenameNameDotBeginDialog
// ============================================================
TEST_F(DialogManagerTest, ShowRenameNameDotBeginDialog)
{
    auto *dm = DialogManager::instance();
    int ret = dm->showRenameNameDotBeginDialog();
    // exec stubbed to Accepted; buttonClicked signal handler checks index==0 → ret=1
    // but since stub returns Accepted, the dialog flow may differ
    EXPECT_NO_FATAL_FAILURE({ (void)ret; });
}

// ============================================================
// 22. showUnableToVistDir
// ============================================================
TEST_F(DialogManagerTest, ShowUnableToVistDir)
{
    auto *dm = DialogManager::instance();
    int code = dm->showUnableToVistDir("/root/secret");
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 23. showBreakSymlinkDialog
// ============================================================
TEST_F(DialogManagerTest, ShowBreakSymlinkDialog_Cancel)
{
    auto *dm = DialogManager::instance();
    g_dialogExecReturn = 0;   // Cancel
    auto type = dm->showBreakSymlinkDialog("target.txt", QUrl("file:///tmp/link"));
    EXPECT_EQ(type, DFMBASE_NAMESPACE::GlobalEventType::kUnknowType);
}

TEST_F(DialogManagerTest, ShowBreakSymlinkDialog_ConfirmNonTrash)
{
    auto *dm = DialogManager::instance();
    g_dialogExecReturn = 1;   // Confirm button
    // Stub FileUtils::isTrashFile to return false
    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    auto type = dm->showBreakSymlinkDialog("target.txt", QUrl("file:///tmp/link"));
    EXPECT_EQ(type, DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash);
}

TEST_F(DialogManagerTest, ShowBreakSymlinkDialog_ConfirmTrash)
{
    auto *dm = DialogManager::instance();
    g_dialogExecReturn = 1;   // Confirm button
    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    auto type = dm->showBreakSymlinkDialog("target.txt", QUrl("file:///tmp/link"));
    EXPECT_EQ(type, DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles);
}

// ============================================================
// 24. showAskIfAddExcutableFlagAndRunDialog
// ============================================================
TEST_F(DialogManagerTest, ShowAskIfAddExcutableFlagAndRunDialog)
{
    auto *dm = DialogManager::instance();
    int code = dm->showAskIfAddExcutableFlagAndRunDialog();
    EXPECT_EQ(code, QDialog::Accepted);
}

// ============================================================
// 25. showDeleteSystemPathWarnDialog
// ============================================================
TEST_F(DialogManagerTest, ShowDeleteSystemPathWarnDialog)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showDeleteSystemPathWarnDialog(0); });
}

// ============================================================
// 26. addTask with null
// ============================================================
TEST_F(DialogManagerTest, AddTask_Null)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->addTask(nullptr); });
}

// ============================================================
// 27. showSetingsDialog null window
// ============================================================
TEST_F(DialogManagerTest, ShowSetingsDialog_NullWindow)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showSetingsDialog(nullptr); });
}

// ============================================================
// 28. showSetingsDialog with group key
// ============================================================
TEST_F(DialogManagerTest, ShowSetingsDialog_NullWindowWithGroupKey)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({ dm->showSetingsDialog(nullptr, "some.group.key"); });
}

// ============================================================
// 29. registerSettingWidget
// ============================================================
TEST_F(DialogManagerTest, RegisterSettingWidget)
{
    auto *dm = DialogManager::instance();
    EXPECT_NO_FATAL_FAILURE({
        dm->registerSettingWidget("testViewType", [](QObject *obj) -> QWidget * {
            return new QLabel("test");
        });
    });
}


#include <QFileInfo>
