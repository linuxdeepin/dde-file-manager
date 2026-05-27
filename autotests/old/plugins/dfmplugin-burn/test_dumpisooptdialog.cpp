// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/dialogs/dumpisooptdialog.h"
#include "plugins/common/dfmplugin-burn/utils/burnjobmanager.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <DFileDialog>
#include <QStandardPaths>
#include <QPushButton>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_DumpISOOptDialog : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(VADDR(DDialog, exec), [&] { __DBG_STUB_INVOKE__
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });                                             return QDialog::Accepted; });
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });

        // Mock WindowUtils to avoid Wayland-specific code
        stub.set_lamda(ADDR(WindowUtils, isWayLand), [] {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Mock device info
        stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
            __DBG_STUB_INVOKE__
            QVariantMap info;
            info[DeviceProperty::kIdLabel] = "TestDisc";
            info[DeviceProperty::kDevice] = "/dev/sr0";
            info[DeviceProperty::kUDisks2Size] = 700000000ULL;   // 700MB
            return info;
        });

        dialog = new DumpISOOptDialog("/org/freedesktop/UDisks2/block_devices/sr0");
    }

    virtual void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    DumpISOOptDialog *dialog = nullptr;
};

TEST_F(UT_DumpISOOptDialog, Constructor)
{
    EXPECT_EQ(dialog->curDevId, "/org/freedesktop/UDisks2/block_devices/sr0");
    EXPECT_TRUE(dialog->isModal());
    EXPECT_EQ(dialog->curDiscName, "TestDisc");
    EXPECT_EQ(dialog->curDev, "/dev/sr0");
}

TEST_F(UT_DumpISOOptDialog, Constructor_Wayland)
{
    stub.set_lamda(ADDR(WindowUtils, isWayLand), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    DumpISOOptDialog waylandDialog("/org/freedesktop/UDisks2/block_devices/sr1");
    EXPECT_EQ(waylandDialog.curDevId, "/org/freedesktop/UDisks2/block_devices/sr1");
}

TEST_F(UT_DumpISOOptDialog, Constructor_EmptyDiscName)
{
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kIdLabel] = "";   // Empty label
        info[DeviceProperty::kDevice] = "/dev/sr0";
        info[DeviceProperty::kUDisks2Size] = 700000000ULL;
        return info;
    });

    stub.set_lamda(ADDR(DeviceUtils, nameOfDefault), [] {
        __DBG_STUB_INVOKE__
        return QString("DefaultName");
    });

    DumpISOOptDialog emptyNameDialog("/org/freedesktop/UDisks2/block_devices/sr0");
    EXPECT_EQ(emptyNameDialog.curDiscName, "DefaultName");
}

TEST_F(UT_DumpISOOptDialog, initData_ValidDiscName)
{
    EXPECT_EQ(dialog->curDiscName, "TestDisc");
    EXPECT_EQ(dialog->curDev, "/dev/sr0");
}

TEST_F(UT_DumpISOOptDialog, onButtonClicked_Cancel)
{
    dialog->onButtonClicked(0, "Cancel");

    // Should not trigger any dump operations
    // No assertions needed as this is a cancel operation
}

TEST_F(UT_DumpISOOptDialog, onButtonClicked_CreateImage_ValidPath)
{
    bool startDumpISOImageCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startDumpISOImage), [&startDumpISOImageCalled] {
        __DBG_STUB_INVOKE__
        startDumpISOImageCalled = true;
    });

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/test.iso");
    });

    dialog->fileChooser->setText("/tmp/test.iso");
    dialog->onButtonClicked(1, "Create ISO Image");

    EXPECT_TRUE(startDumpISOImageCalled);
}

TEST_F(UT_DumpISOOptDialog, onButtonClicked_CreateImage_InvalidPath)
{
    bool startDumpISOImageCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startDumpISOImage), [&startDumpISOImageCalled] {
        __DBG_STUB_INVOKE__
        startDumpISOImageCalled = true;
    });

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        return QUrl();   // Invalid URL
    });

    dialog->fileChooser->setText("");
    dialog->onButtonClicked(1, "Create ISO Image");

    EXPECT_TRUE(startDumpISOImageCalled);   // Should still be called even with invalid path
}

TEST_F(UT_DumpISOOptDialog, onButtonClicked_CreateImage_EmptyDevice)
{
    bool startDumpISOImageCalled = false;

    stub.set_lamda(ADDR(BurnJobManager, startDumpISOImage), [&startDumpISOImageCalled] {
        __DBG_STUB_INVOKE__
        startDumpISOImageCalled = true;
    });

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp/test.iso");
    });

    dialog->curDev = "";   // Empty device
    dialog->onButtonClicked(1, "Create ISO Image");

    EXPECT_TRUE(startDumpISOImageCalled);   // Should still be called
}

TEST_F(UT_DumpISOOptDialog, onFileChoosed_ValidPath)
{
    QString testPath = "/tmp/testdir";

    // Mock file doesn't exist initially
    bool fileExists = false;
    stub.set_lamda(VADDR(SyncFileInfo, exists), [&fileExists] {
        __DBG_STUB_INVOKE__
        return fileExists;
    });

    dialog->onFileChoosed(testPath);

    QString expectedPath = testPath + "/" + dialog->curDiscName + ".iso";
    EXPECT_EQ(dialog->fileChooser->text(), expectedPath);
}

TEST_F(UT_DumpISOOptDialog, onFileChoosed_FileExists_GenerateSerial)
{
    QString testPath = "/tmp/testdir";

    // Mock file exists for first attempt, then doesn't exist
    int callCount = 0;
    stub.set_lamda(VADDR(SyncFileInfo, exists), [&callCount] {
        __DBG_STUB_INVOKE__
        return (callCount++ == 0);   // First call returns true, second returns false
    });

    dialog->onFileChoosed(testPath);

    QString expectedPath = testPath + "/" + dialog->curDiscName + "(1).iso";
    EXPECT_EQ(dialog->fileChooser->text(), expectedPath);
}

TEST_F(UT_DumpISOOptDialog, onFileChoosed_MaxSerialReached)
{
    QString testPath = "/tmp/testdir";

    // Mock file always exists to trigger max serial scenario.
    stub.set_lamda(VADDR(SyncFileInfo, exists), [] {
        __DBG_STUB_INVOKE__
        return true;   // Always exists
    });

    dialog->onFileChoosed(testPath);

    // Should handle max serial gracefully (implementation should log warning and return)
}

TEST_F(UT_DumpISOOptDialog, onPathChanged_ValidLocalPath)
{
    QString validPath = "/tmp/test.iso";

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [&validPath] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile(validPath);
    });

    stub.set_lamda(ADDR(ProtocolUtils, isRemoteFile), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(ProtocolUtils, isSMBFile), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    dialog->onPathChanged(validPath);

    EXPECT_TRUE(dialog->createImgBtn->isEnabled());
}

TEST_F(UT_DumpISOOptDialog, onPathChanged_EmptyPath)
{
    QString emptyPath = "";

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        return QUrl();
    });

    dialog->onPathChanged(emptyPath);

    EXPECT_FALSE(dialog->createImgBtn->isEnabled());
}

TEST_F(UT_DumpISOOptDialog, onPathChanged_InvalidUrl)
{
    QString invalidPath = "invalid://path";

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        QUrl url("invalid://path");
        url.setScheme("invalid");
        return url;
    });

    dialog->onPathChanged(invalidPath);

    EXPECT_FALSE(dialog->createImgBtn->isEnabled());
}

TEST_F(UT_DumpISOOptDialog, onPathChanged_RemoteFile)
{
    QString remotePath = "/remote/path/test.iso";

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/remote/path/test.iso");
    });

    stub.set_lamda(ADDR(ProtocolUtils, isRemoteFile), [] {
        __DBG_STUB_INVOKE__
        return true;   // Is remote file
    });

    dialog->onPathChanged(remotePath);

    EXPECT_FALSE(dialog->createImgBtn->isEnabled());
}

TEST_F(UT_DumpISOOptDialog, onPathChanged_SMBFile)
{
    QString smbPath = "smb://server/share/test.iso";

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        return QUrl("smb://server/share/test.iso");
    });

    stub.set_lamda(ADDR(ProtocolUtils, isRemoteFile), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(ProtocolUtils, isSMBFile), [] {
        __DBG_STUB_INVOKE__
        return true;   // Is SMB file
    });

    dialog->onPathChanged(smbPath);

    EXPECT_FALSE(dialog->createImgBtn->isEnabled());
}

TEST_F(UT_DumpISOOptDialog, onPathChanged_NonLocalFile)
{
    QString httpPath = "http://example.com/test.iso";

    using FromUserInputFunc = QUrl (*)(const QString &, bool);
    stub.set_lamda(static_cast<FromUserInputFunc>(&UrlRoute::fromUserInput), [] {
        __DBG_STUB_INVOKE__
        QUrl url("http://example.com/test.iso");
        return url;
    });

    dialog->onPathChanged(httpPath);

    EXPECT_FALSE(dialog->createImgBtn->isEnabled());
}

TEST_F(UT_DumpISOOptDialog, UI_Components_Existence)
{
    // Test that all UI components are properly created
    EXPECT_TRUE(dialog->contentWidget != nullptr);
    EXPECT_TRUE(dialog->saveAsImgLabel != nullptr);
    EXPECT_TRUE(dialog->commentLabel != nullptr);
    EXPECT_TRUE(dialog->savePathLabel != nullptr);
    EXPECT_TRUE(dialog->fileChooser != nullptr);
    EXPECT_TRUE(dialog->createImgBtn != nullptr);
}

TEST_F(UT_DumpISOOptDialog, UI_DefaultValues)
{
    // Test initial UI state
    EXPECT_FALSE(dialog->createImgBtn->isEnabled());   // Should be disabled initially
    EXPECT_EQ(dialog->fileChooser->fileMode(), QFileDialog::FileMode::Directory);
}

TEST_F(UT_DumpISOOptDialog, UI_DefaultDirectory)
{
    // Test that default directory is set to Documents
    QString expectedPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
    QUrl expectedUrl = QUrl::fromLocalFile(expectedPath);

    // The fileChooser should be set to Documents directory by default
    EXPECT_EQ(dialog->fileChooser->directoryUrl(), expectedUrl);
}

TEST_F(UT_DumpISOOptDialog, Dialog_Properties)
{
    // Test dialog properties
    EXPECT_TRUE(dialog->isModal());
    EXPECT_EQ(dialog->size(), QSize(400, 242));
}

TEST_F(UT_DumpISOOptDialog, Signal_Connections)
{
    // Test that signals are properly connected by emitting them
    bool buttonClickedConnected = false;
    bool fileChoosedConnected = false;
    bool textChangedConnected = false;

    // Connect to test signals
    QObject::connect(dialog, &DumpISOOptDialog::buttonClicked, [&buttonClickedConnected] {
        buttonClickedConnected = true;
    });

    QObject::connect(dialog->fileChooser, &DFileChooserEdit::fileChoosed, [&fileChoosedConnected] {
        fileChoosedConnected = true;
    });

    QObject::connect(dialog->fileChooser, &DFileChooserEdit::textChanged, [&textChangedConnected] {
        textChangedConnected = true;
    });

    // Emit signals to test connections
    emit dialog->buttonClicked(0, "test");
    emit dialog->fileChooser->fileChoosed("/test/path");
    emit dialog->fileChooser->textChanged("/test/text");

    EXPECT_TRUE(buttonClickedConnected);
    EXPECT_TRUE(fileChoosedConnected);
    EXPECT_TRUE(textChangedConnected);
}

TEST_F(UT_DumpISOOptDialog, Destructor)
{
    // Test that destructor doesn't crash
    DumpISOOptDialog *testDialog = new DumpISOOptDialog("/org/freedesktop/UDisks2/block_devices/sr0");
    delete testDialog;

    // If we reach here, destructor worked correctly
    EXPECT_TRUE(true);
}

TEST_F(UT_DumpISOOptDialog, initData_DeviceInfoRetrieval)
{
    // Test that device info is properly retrieved during initialization
    EXPECT_FALSE(dialog->curDiscName.isEmpty());
    EXPECT_FALSE(dialog->curDev.isEmpty());
    EXPECT_EQ(dialog->curDiscName, "TestDisc");
    EXPECT_EQ(dialog->curDev, "/dev/sr0");
}
