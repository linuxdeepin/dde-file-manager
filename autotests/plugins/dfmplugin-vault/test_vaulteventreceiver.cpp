// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>

#include "stubext.h"

#include "events/vaulteventreceiver.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class VaultEventReceiverImpl : public testing::Test
{
public:
    void SetUp() override
    {
        stub.set_lamda(&PathManager::makeVaultLocalPath, [this](const QString &, const QString &) -> QString {
            return tempPath;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QString tempPath { "/tmp/vault_event_test" };
};

TEST_F(VaultEventReceiverImpl, Instance)
{
    VaultEventReceiver *r = VaultEventReceiver::instance();
    EXPECT_NE(r, nullptr);
    EXPECT_EQ(r, VaultEventReceiver::instance());
}

TEST_F(VaultEventReceiverImpl, ConnectEvent)
{
    VaultEventReceiver::instance()->connectEvent();
    SUCCEED();
}

TEST_F(VaultEventReceiverImpl, ComputerOpenItem_NonVaultPath)
{
    VaultEventReceiver::instance()->computerOpenItem(0, QUrl::fromLocalFile("/tmp/foo"));
    SUCCEED();
}

TEST_F(VaultEventReceiverImpl, ComputerOpenItem_Unlocked)
{
    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState { return kUnlocked; });

    VaultEventReceiver::instance()->computerOpenItem(1, QUrl::fromLocalFile("/tmp/vault_test"));
    SUCCEED();
}

TEST_F(VaultEventReceiverImpl, ComputerOpenItem_Encrypted)
{
    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState { return kEncrypted; });
    using UnlockDialogFunc = void (VaultHelper::*)();
    bool called = false;
    stub.set_lamda(static_cast<UnlockDialogFunc>(&VaultHelper::unlockVaultDialog),
                   [&called](VaultHelper *) { called = true; });

    VaultEventReceiver::instance()->computerOpenItem(1, QUrl::fromLocalFile("/tmp/vault_test"));
    EXPECT_TRUE(called);
}

TEST_F(VaultEventReceiverImpl, HandleNotAllowedAppendCompress_Allowed)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/foo") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/bar");
    EXPECT_FALSE(VaultEventReceiver::instance()->handleNotAllowedAppendCompress(fromUrls, toUrl));
}

TEST_F(VaultEventReceiverImpl, HandleNotAllowedAppendCompress_FromVault)
{
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *out) -> bool {
        if (out)
            *out << QUrl::fromLocalFile("/tmp/vault_event_test/foo");
        return true;
    });

    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/whatever") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/bar");
    EXPECT_TRUE(VaultEventReceiver::instance()->handleNotAllowedAppendCompress(fromUrls, toUrl));
}

TEST_F(VaultEventReceiverImpl, HandleCurrentUrlChanged_VaultScheme)
{
    using FindFunc = FileManagerWindow *(FileManagerWindowsManager::*)(quint64);
    stub.set_lamda(static_cast<FindFunc>(&FileManagerWindowsManager::findWindowById),
                   [](FileManagerWindowsManager *, quint64) -> FileManagerWindow * { return nullptr; });

    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/");
    VaultHelper::instance()->appendWinID(7);
    VaultEventReceiver::instance()->handleCurrentUrlChanged(7, url);
    VaultHelper::instance()->removeWinID(7);
    SUCCEED();
}

TEST_F(VaultEventReceiverImpl, HandleCurrentUrlChanged_NonVaultScheme)
{
    using FindFunc = FileManagerWindow *(FileManagerWindowsManager::*)(quint64);
    stub.set_lamda(static_cast<FindFunc>(&FileManagerWindowsManager::findWindowById),
                   [](FileManagerWindowsManager *, quint64) -> FileManagerWindow * { return nullptr; });

    VaultHelper::instance()->appendWinID(8);
    VaultEventReceiver::instance()->handleCurrentUrlChanged(8, QUrl::fromLocalFile("/tmp/foo"));
    VaultHelper::instance()->removeWinID(8);
    SUCCEED();
}

TEST_F(VaultEventReceiverImpl, HandleSideBarItemDragMoveData_NonTag)
{
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_FALSE(VaultEventReceiver::instance()->handleSideBarItemDragMoveData({}, QUrl(), &action));
}

TEST_F(VaultEventReceiverImpl, HandleSideBarItemDragMoveData_TagVaultFile)
{
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_TRUE(VaultEventReceiver::instance()->handleSideBarItemDragMoveData({ vaultUrl }, tagUrl, &action));
    EXPECT_EQ(action, Qt::IgnoreAction);
}

TEST_F(VaultEventReceiverImpl, HandleShortCutPasteFiles_Empty)
{
    EXPECT_FALSE(VaultEventReceiver::instance()->handleShortCutPasteFiles(0, {}, QUrl()));
}

TEST_F(VaultEventReceiverImpl, HandleShortCutPasteFiles_Allowed)
{
    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &) -> bool { return true; });

    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    QList<QUrl> fromUrls = { vaultUrl };
    QUrl toUrl = QUrl::fromLocalFile("trash:///test");
    EXPECT_TRUE(VaultEventReceiver::instance()->handleShortCutPasteFiles(0, fromUrls, toUrl));
}

TEST_F(VaultEventReceiverImpl, HandleShortCutPasteFiles_Disallowed)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    QList<QUrl> fromUrls = { vaultUrl };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/foo");
    EXPECT_FALSE(VaultEventReceiver::instance()->handleShortCutPasteFiles(0, fromUrls, toUrl));
}

TEST_F(VaultEventReceiverImpl, ChangeUrlEventFilter_NonVaultScheme)
{
    QUrl url = QUrl::fromLocalFile("/tmp/foo");
    EXPECT_FALSE(VaultEventReceiver::instance()->changeUrlEventFilter(0, url));
}

TEST_F(VaultEventReceiverImpl, ChangeUrlEventFilter_Unlocked)
{
    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState { return kUnlocked; });

    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/");
    EXPECT_FALSE(VaultEventReceiver::instance()->changeUrlEventFilter(0, url));
}

TEST_F(VaultEventReceiverImpl, ChangeUrlEventFilter_NotAvailable)
{
    using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                   [](FileEncryptHandle *, const QString &, bool) -> VaultState { return kNotAvailable; });
    using DialogFunc = void (DialogManager::*)(const QString &, const QString &);
    bool called = false;
    stub.set_lamda(VADDR(DialogManager, showErrorDialog),
                   [&called](DialogManager *, const QString &, const QString &) { called = true; });

    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/");
    EXPECT_TRUE(VaultEventReceiver::instance()->changeUrlEventFilter(0, url));
    EXPECT_TRUE(called);
}

TEST_F(VaultEventReceiverImpl, HandlePathtoVirtual_Empty)
{
    QList<QUrl> out;
    EXPECT_FALSE(VaultEventReceiver::instance()->handlePathtoVirtual({}, &out));
}

TEST_F(VaultEventReceiverImpl, HandlePathtoVirtual_VaultFiles)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    QList<QUrl> out;
    EXPECT_TRUE(VaultEventReceiver::instance()->handlePathtoVirtual({ vaultUrl }, &out));
    EXPECT_FALSE(out.isEmpty());
}

TEST_F(VaultEventReceiverImpl, DetailViewIcon_RootUrl)
{
    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/");
    QString iconName;
    EXPECT_TRUE(VaultEventReceiver::instance()->detailViewIcon(url, &iconName));
    EXPECT_EQ(iconName, QString("drive-harddisk-encrypted"));
}

TEST_F(VaultEventReceiverImpl, DetailViewIcon_NonRoot)
{
    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/foo");
    QString iconName;
    EXPECT_FALSE(VaultEventReceiver::instance()->detailViewIcon(url, &iconName));
}

TEST_F(VaultEventReceiverImpl, FileDropHandleWithAction_ToVault)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    QUrl localUrl = QUrl::fromLocalFile("/tmp/foo");
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_TRUE(VaultEventReceiver::instance()->fileDropHandleWithAction({ localUrl }, vaultUrl, &action));
    EXPECT_EQ(action, Qt::CopyAction);
}

TEST_F(VaultEventReceiverImpl, HandlePermissionViewAsh_VaultFile)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    bool isAsh = false;
    EXPECT_TRUE(VaultEventReceiver::instance()->handlePermissionViewAsh(vaultUrl, &isAsh));
    EXPECT_TRUE(isAsh);
}

TEST_F(VaultEventReceiverImpl, HandlePermissionViewAsh_NonVaultFile)
{
    bool isAsh = false;
    EXPECT_FALSE(VaultEventReceiver::instance()->handlePermissionViewAsh(QUrl::fromLocalFile("/tmp/foo"), &isAsh));
}

TEST_F(VaultEventReceiverImpl, HandleFileCanTaged_VaultUrl)
{
    QUrl vaultUrl;
    vaultUrl.setScheme("dfmvault");
    vaultUrl.setPath("/");
    bool canTag = true;
    EXPECT_TRUE(VaultEventReceiver::instance()->handleFileCanTaged(vaultUrl, &canTag));
    EXPECT_FALSE(canTag);
}

TEST_F(VaultEventReceiverImpl, HandleFileCanTaged_NonVaultUrl)
{
    bool canTag = true;
    EXPECT_FALSE(VaultEventReceiver::instance()->handleFileCanTaged(QUrl::fromLocalFile("/tmp/foo"), &canTag));
}
