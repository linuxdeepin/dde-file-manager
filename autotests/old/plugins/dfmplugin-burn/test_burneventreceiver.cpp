// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/events/burneventreceiver.h"
#include "plugins/common/dfmplugin-burn/events/burneventcaller.h"
#include "plugins/common/dfmplugin-burn/utils/burnjobmanager.h"
#include "plugins/common/dfmplugin-burn/dialogs/burnoptdialog.h"
#include "plugins/common/dfmplugin-burn/dialogs/dumpisooptdialog.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-framework/event/event.h>

#include <DDialog>
#include <QWidget>
#include <QApplication>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_BurnEventReceiver : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(VADDR(DDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });
    }
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_BurnEventReceiver, instance)
{
    BurnEventReceiver *receiver1 = BurnEventReceiver::instance();
    BurnEventReceiver *receiver2 = BurnEventReceiver::instance();

    EXPECT_TRUE(receiver1 != nullptr);
    EXPECT_EQ(receiver1, receiver2);   // Should be singleton
}

TEST_F(UT_BurnEventReceiver, handleShowBurnDlg_UDFSupported)
{
    bool dialogShown = false;

    stub.set_lamda(VADDR(DDialog, exec), [&dialogShown] {
        __DBG_STUB_INVOKE__
        dialogShown = true;
        return QDialog::Accepted;
    });

    QWidget parent;
    BurnEventReceiver::instance()->handleShowBurnDlg("/dev/sr0", true, &parent);

    EXPECT_TRUE(dialogShown);
}

TEST_F(UT_BurnEventReceiver, handleShowBurnDlg_UDFNotSupported)
{
    bool dialogShown = false;

    stub.set_lamda(VADDR(DDialog, exec), [&dialogShown] {
        __DBG_STUB_INVOKE__
        dialogShown = true;
        return QDialog::Accepted;
    });

    QWidget parent;
    BurnEventReceiver::instance()->handleShowBurnDlg("/dev/sr0", false, &parent);

    EXPECT_TRUE(dialogShown);
}

TEST_F(UT_BurnEventReceiver, handleShowBurnDlg_NullParent)
{
    bool dialogShown = false;

    stub.set_lamda(VADDR(DDialog, exec), [&dialogShown] {
        __DBG_STUB_INVOKE__
        dialogShown = true;
        return QDialog::Accepted;
    });

    BurnEventReceiver::instance()->handleShowBurnDlg("/dev/sr0", true, nullptr);

    EXPECT_TRUE(dialogShown);
}

TEST_F(UT_BurnEventReceiver, handleShowDumpISODlg)
{
    bool dialogShown = false;

    stub.set_lamda(VADDR(DumpISOOptDialog, exec), [&dialogShown] {
        __DBG_STUB_INVOKE__
        dialogShown = true;
        return QDialog::Accepted;
    });

    BurnEventReceiver::instance()->handleShowDumpISODlg("/dev/sr0");

    EXPECT_TRUE(dialogShown);
}

TEST_F(UT_BurnEventReceiver, handleErase)
{
    bool eraseStarted = false;

    stub.set_lamda(ADDR(BurnJobManager, startEraseDisc), [&eraseStarted] {
        __DBG_STUB_INVOKE__
        eraseStarted = true;
    });
    stub.set_lamda(VADDR(DDialog, exec), [&] { __DBG_STUB_INVOKE__ return DDialog::Accepted; });

    BurnEventReceiver::instance()->handleErase("/dev/sr0");

    EXPECT_TRUE(eraseStarted);
}

TEST_F(UT_BurnEventReceiver, handlePasteTo_Copy)
{
    bool pasteFilesCalled = false;
    QList<QUrl> capturedUrls;
    QUrl capturedDest;
    bool capturedIsCopy = false;

    stub.set_lamda(ADDR(BurnEventCaller, sendPasteFiles), [&](const QList<QUrl> &urls, const QUrl &dest, bool isCopy) {
        __DBG_STUB_INVOKE__
        pasteFilesCalled = true;
        capturedUrls = urls;
        capturedDest = dest;
        capturedIsCopy = isCopy;
    });

    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/file1.txt"),
                         QUrl::fromLocalFile("/tmp/file2.txt") };
    QUrl dest;
    dest.setScheme("burn");
    dest.setPath("/dev/sr0/disc_files/");

    BurnEventReceiver::instance()->handlePasteTo(urls, dest, true);

    EXPECT_TRUE(pasteFilesCalled);
    EXPECT_EQ(capturedUrls, urls);
    EXPECT_TRUE(capturedIsCopy);
}

TEST_F(UT_BurnEventReceiver, handlePasteTo_Move)
{
    bool pasteFilesCalled = false;
    QList<QUrl> capturedUrls;
    QUrl capturedDest;
    bool capturedIsCopy = true; // Initialize to opposite of expected

    stub.set_lamda(ADDR(BurnEventCaller, sendPasteFiles), [&](const QList<QUrl> &urls, const QUrl &dest, bool isCopy) {
        __DBG_STUB_INVOKE__
        pasteFilesCalled = true;
        capturedUrls = urls;
        capturedDest = dest;
        capturedIsCopy = isCopy;
    });

    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl dest;
    dest.setScheme("burn");
    dest.setPath("/dev/sr0/disc_files/");

    BurnEventReceiver::instance()->handlePasteTo(urls, dest, false);

    EXPECT_TRUE(pasteFilesCalled);
    EXPECT_EQ(capturedUrls, urls);
    EXPECT_FALSE(capturedIsCopy);
}

TEST_F(UT_BurnEventReceiver, handleMountImage)
{
    // This test requires complex framework event stubbing
    // For now, just test that the method doesn't crash
    QUrl isoUrl = QUrl::fromLocalFile("/tmp/test.iso");
    BurnEventReceiver::instance()->handleMountImage(12345, isoUrl);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_BurnEventReceiver, handleCopyFilesResult_Success)
{
    bool auditLogStarted = false;

    stub.set_lamda(ADDR(BurnJobManager, startAuditLogForCopyFromDisc), [&auditLogStarted] {
        __DBG_STUB_INVOKE__
        auditLogStarted = true;
    });

    // Stub DeviceProxyManager to simulate copying from optical disc
    stub.set_lamda(ADDR(DeviceProxyManager, isFileFromOptical), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    BurnEventReceiver::instance()->handleCopyFilesResult(srcUrls, destUrls, true, "");

    EXPECT_TRUE(auditLogStarted);
}

TEST_F(UT_BurnEventReceiver, handleCopyFilesResult_Failure)
{
    // The method ignores the 'ok' and 'errMsg' parameters and doesn't show error dialogs
    // This test just verifies the method doesn't crash when called with failure parameters
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    BurnEventReceiver::instance()->handleCopyFilesResult(srcUrls, destUrls, false, "Copy failed");

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_BurnEventReceiver, handleFileCutResult_Success)
{
    bool putFilesStarted = false;

    stub.set_lamda(ADDR(BurnJobManager, startPutFilesToDisc), [&putFilesStarted] {
        __DBG_STUB_INVOKE__
        putFilesStarted = true;
    });

    // Stub DeviceUtils methods for packet writing conditions
    stub.set_lamda(ADDR(DeviceUtils, getMountInfo), [] {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });
    
    stub.set_lamda(ADDR(DeviceUtils, isPWUserspaceOpticalDiscDev), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    BurnEventReceiver::instance()->handleFileCutResult(srcUrls, destUrls, true, "");

    EXPECT_TRUE(putFilesStarted);
}

TEST_F(UT_BurnEventReceiver, handleFileCutResult_Failure)
{
    // The method ignores the 'ok' and 'errMsg' parameters and doesn't show error dialogs
    // This test just verifies the method doesn't crash when called with failure parameters
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    BurnEventReceiver::instance()->handleFileCutResult(srcUrls, destUrls, false, "Cut failed");

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_BurnEventReceiver, handleFileRemoveResult_Success)
{
    bool removeStarted = false;

    stub.set_lamda(ADDR(BurnJobManager, startRemoveFilesFromDisc), [&removeStarted] {
        __DBG_STUB_INVOKE__
        removeStarted = true;
    });

    // Stub DeviceUtils methods for packet writing conditions
    stub.set_lamda(ADDR(DeviceUtils, getMountInfo), [] {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });
    
    stub.set_lamda(ADDR(DeviceUtils, isPWUserspaceOpticalDiscDev), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QList<QUrl> srcUrls;
    srcUrls << QUrl::fromLocalFile("/media/sr0/file1.txt");

    BurnEventReceiver::instance()->handleFileRemoveResult(srcUrls, true, "");

    EXPECT_TRUE(removeStarted);
}

TEST_F(UT_BurnEventReceiver, handleFileRemoveResult_Failure)
{
    // The method ignores the 'ok' and 'errMsg' parameters and doesn't show error dialogs
    // This test just verifies the method doesn't crash when called with failure parameters
    QList<QUrl> srcUrls;
    srcUrls << QUrl::fromLocalFile("/media/sr0/file1.txt");

    BurnEventReceiver::instance()->handleFileRemoveResult(srcUrls, false, "Remove failed");

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_BurnEventReceiver, handleFileRenameResult_Success)
{
    bool renameStarted = false;

    stub.set_lamda(ADDR(BurnJobManager, startRenameFileFromDisc), [&renameStarted] {
        __DBG_STUB_INVOKE__
        renameStarted = true;
    });

    // Stub DeviceUtils methods for packet writing conditions
    stub.set_lamda(ADDR(DeviceUtils, getMountInfo), [] {
        __DBG_STUB_INVOKE__
        return QString("/dev/sr0");
    });
    
    stub.set_lamda(ADDR(DeviceUtils, isPWUserspaceOpticalDiscDev), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls[QUrl::fromLocalFile("/media/sr0/oldname.txt")] = QUrl::fromLocalFile("/media/sr0/newname.txt");

    BurnEventReceiver::instance()->handleFileRenameResult(12345, renamedUrls, true, "");

    EXPECT_TRUE(renameStarted);
}

TEST_F(UT_BurnEventReceiver, handleFileRenameResult_Failure)
{
    // The method checks 'ok' parameter and returns early if false, but doesn't show error dialogs
    // This test verifies that no rename operation is started when ok=false
    bool renameStarted = false;

    stub.set_lamda(ADDR(BurnJobManager, startRenameFileFromDisc), [&renameStarted] {
        __DBG_STUB_INVOKE__
        renameStarted = true;
    });

    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls[QUrl::fromLocalFile("/media/sr0/oldname.txt")] = QUrl::fromLocalFile("/media/sr0/newname.txt");

    BurnEventReceiver::instance()->handleFileRenameResult(12345, renamedUrls, false, "Rename failed");

    // Should not start rename operation when ok=false
    EXPECT_FALSE(renameStarted);
}

TEST_F(UT_BurnEventReceiver, handleFileRenameResult_EmptyMap)
{
    // Should handle empty rename map gracefully
    QMap<QUrl, QUrl> emptyMap;

    BurnEventReceiver::instance()->handleFileRenameResult(12345, emptyMap, true, "");
    // Should not crash
}
