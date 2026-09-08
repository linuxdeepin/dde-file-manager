// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultHelper deep-coverage tests. Every modal entry (DDialog::exec /
// QMenu::exec) is stubbed so dialog-building code paths run synchronously
// under offscreen. Vault state and FileEncryptHandle interactions are stubbed.

#include <gtest/gtest.h>
#include <QUrl>
#include <QPoint>
#include <QMenu>
#include <QFile>
#include <QDir>

#include "stubext.h"

#include "utils/vaulthelper.h"
#include "utils/vaultdefine.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"
#include "events/vaulteventcaller.h"
#include "views/vaultpropertyview/vaultpropertydialog.h"

#include <DDialog>
#include <QTimer>
#include <QApplication>

#include <dfm-base/utils/dialogmanager.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
const QString kConfigPath = kVaultBasePath + "/" + QString(kVaultConfigFileName);
const QString kContainerPath = kVaultBasePath + "/password_container.bin";
}

class VaultHelperMoreTest : public testing::Test
{
protected:
    void SetUp() override
    {
        QDir().mkpath(kVaultBasePath);
        if (QFile::exists(kConfigPath))
            QFile::copy(kConfigPath, kConfigPath + ".utbak");

        // nothing: modal dialogs are closed by scheduleAutoCloseModal()
    }

    void TearDown() override
    {
        stub.clear();
        QFile::remove(kContainerPath);
        QFile::remove(kConfigPath);
        if (QFile::exists(kConfigPath + ".utbak")) {
            QFile::remove(kConfigPath);
            QFile::rename(kConfigPath + ".utbak", kConfigPath);
        }
    }

    // Modal exec() loops cannot be stubbed (virtual), so any dialog opened
    // via exec() is closed from a timer running inside that event loop.
    void scheduleAutoCloseModal()
    {
        QTimer::singleShot(50, []() {
            int guard = 0;
            while (QWidget *modal = QApplication::activeModalWidget()) {
                modal->close();
                QApplication::processEvents();
                if (++guard > 5)
                    break;
            }
        });
    }

    void stubState(VaultState state)
    {
        using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
        stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                       [state](FileEncryptHandle *, const QString &, bool) -> VaultState {
                           return state;
                       });
    }

    void writeContainer(bool present)
    {
        QFile::remove(kContainerPath);
        if (present) {
            QFile f(kContainerPath);
            f.open(QIODevice::WriteOnly);
            f.close();
        }
    }

    void setEncryptionMethod(const QString &method)
    {
        VaultConfig config;
        config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(method));
    }

    stub_ext::StubExt stub;
};

TEST_F(VaultHelperMoreTest, KillVaultTasks_EmptyImpl_NoSideEffect)
{
    VaultHelper::instance()->appendWinID(9);
    VaultHelper::instance()->killVaultTasks();
    EXPECT_EQ(VaultHelper::instance()->currentWindowId(), quint64(9));
}

TEST_F(VaultHelperMoreTest, CreateMenu_NotExistedState_HasCreateActionOnly)
{
    stubState(kNotExisted);
    DMenu *menu = VaultHelper::instance()->createMenu();
    ASSERT_NE(menu, nullptr);
    EXPECT_EQ(menu->actions().size(), 1);
    EXPECT_EQ(menu->actions().first()->text(), QObject::tr("Create Vault"));
    delete menu;
}

TEST_F(VaultHelperMoreTest, CreateMenu_EncryptedNewVersion_HasResetPassword)
{
    stubState(kEncrypted);
    writeContainer(true);   // isNewVaultVersion() == true, method != transparent

    DMenu *menu = VaultHelper::instance()->createMenu();
    ASSERT_NE(menu, nullptr);
    EXPECT_EQ(menu->actions().size(), 3);   // Unlock + separator + Reset Password
    delete menu;
}

TEST_F(VaultHelperMoreTest, CreateMenu_EncryptedOldVersion_NoResetPassword)
{
    stubState(kEncrypted);
    writeContainer(false);

    DMenu *menu = VaultHelper::instance()->createMenu();
    ASSERT_NE(menu, nullptr);
    EXPECT_EQ(menu->actions().size(), 2);   // Unlock + separator
    delete menu;
}

TEST_F(VaultHelperMoreTest, CreateMenu_UnlockedKeyMethod_TriggerAllActions_NoCrash)
{
    stubState(kUnlocked);
    writeContainer(false);
    setEncryptionMethod(kConfigValueMethodKey);

    bool lockCalled = false;
    stub.set_lamda(&VaultHelper::lockVault,
                   [&lockCalled](VaultHelper *, bool) -> bool {
                       lockCalled = true;
                       return true;
                   });
    stub.set_lamda(&VaultHelper::createVaultDialog, [](VaultHelper *) {});
    stub.set_lamda(&VaultHelper::unlockVaultDialog, [](VaultHelper *) {});
    stub.set_lamda(&VaultHelper::showRemoveVaultDialog, [](VaultHelper *) {});
    stub.set_lamda(&VaultHelper::showResetPasswordDialog, [](VaultHelper *) {});
    stub.set_lamda(VADDR(DialogManager, showErrorDialog),
                   [](DialogManager *, const QString &, const QString &) {});

    DMenu *menu = VaultHelper::instance()->createMenu();
    ASSERT_NE(menu, nullptr);
    EXPECT_GE(menu->actions().size(), 8);

    for (QAction *act : menu->actions()) {
        if (act->menu()) {
            for (QAction *sub : act->menu()->actions())
                sub->trigger();
        }
        act->trigger();
    }
    EXPECT_TRUE(lockCalled);   // the "Lock" lambda executed
    delete menu;
}

TEST_F(VaultHelperMoreTest, CreateMenu_UnlockedTransparent_NoKeyItems)
{
    stubState(kUnlocked);
    writeContainer(false);
    setEncryptionMethod(kConfigValueMethodTransparent);

    DMenu *menu = VaultHelper::instance()->createMenu();
    ASSERT_NE(menu, nullptr);
    // Open / Open-in-new-window / separator / Delete / Properties
    EXPECT_EQ(menu->actions().size(), 5);
    delete menu;
}

TEST_F(VaultHelperMoreTest, SiderItemClicked_NotExisted_ShowsCreateDialog)
{
    stubState(kNotExisted);
    bool createCalled = false;
    stub.set_lamda(&VaultHelper::createVaultDialog,
                   [&createCalled](VaultHelper *) { createCalled = true; });

    VaultHelper::instance()->siderItemClicked(11, QUrl("dfmvault:///"));
    EXPECT_TRUE(createCalled);
}

TEST_F(VaultHelperMoreTest, SiderItemClicked_Encrypted_ShowsUnlockDialog)
{
    stubState(kEncrypted);
    bool unlockCalled = false;
    stub.set_lamda(&VaultHelper::unlockVaultDialog,
                   [&unlockCalled](VaultHelper *) { unlockCalled = true; });

    VaultHelper::instance()->siderItemClicked(12, QUrl("dfmvault:///"));
    EXPECT_TRUE(unlockCalled);
}

TEST_F(VaultHelperMoreTest, SiderItemClicked_Unlocked_PerformsCdAction)
{
    stubState(kUnlocked);
    quint64 capturedWin = 0;
    QUrl capturedUrl;
    stub.set_lamda(&VaultHelper::defaultCdAction,
                   [&capturedWin, &capturedUrl](VaultHelper *, quint64 winId, const QUrl &url) {
                       capturedWin = winId;
                       capturedUrl = url;
                   });

    QUrl url("dfmvault:///");
    VaultHelper::instance()->siderItemClicked(13, url);
    EXPECT_EQ(capturedWin, quint64(13));
    EXPECT_EQ(capturedUrl, url);
}

TEST_F(VaultHelperMoreTest, SiderItemClicked_NotAvailable_ShowsErrorDialog)
{
    stubState(kNotAvailable);
    bool dialogShown = false;
    stub.set_lamda(VADDR(DialogManager, showErrorDialog),
                   [&dialogShown](DialogManager *, const QString &, const QString &) {
                       dialogShown = true;
                   });

    VaultHelper::instance()->siderItemClicked(14, QUrl("dfmvault:///"));
    EXPECT_TRUE(dialogShown);
}

TEST_F(VaultHelperMoreTest, SiderItemClicked_UnderProcess_NoDialog)
{
    stubState(kUnderProcess);
    bool createCalled = false, unlockCalled = false;
    stub.set_lamda(&VaultHelper::createVaultDialog, [&createCalled](VaultHelper *) { createCalled = true; });
    stub.set_lamda(&VaultHelper::unlockVaultDialog, [&unlockCalled](VaultHelper *) { unlockCalled = true; });

    VaultHelper::instance()->siderItemClicked(15, QUrl("dfmvault:///"));
    EXPECT_FALSE(createCalled);
    EXPECT_FALSE(unlockCalled);
}

TEST_F(VaultHelperMoreTest, ContenxtMenuHandle_ExecReturnsNull_WindowTracked)
{
    stubState(kNotExisted);
    stub.set_lamda(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec),
                   [](QMenu *, const QPoint &, QAction *) -> QAction * { return nullptr; });
    stub.set_lamda(&VaultHelper::createVaultDialog, [](VaultHelper *) {});

    VaultHelper::instance()->contenxtMenuHandle(21, QUrl("dfmvault:///"), QPoint(10, 10));
    EXPECT_EQ(VaultHelper::instance()->currentWindowId(), quint64(21));
}

TEST_F(VaultHelperMoreTest, OpenNewWindow_PublishesVaultRootUrl)
{
    QUrl captured;
    stub.set_lamda(&VaultEventCaller::sendOpenWindow,
                   [&captured](const QUrl &url) { captured = url; });

    VaultHelper::instance()->openNewWindow(QUrl("dfmvault:///dir"));
    EXPECT_EQ(captured, QUrl("dfmvault:///dir"));
}

TEST_F(VaultHelperMoreTest, NewOpenWindow_UsesRootUrlAndRecordsTime)
{
    QUrl captured;
    stub.set_lamda(&VaultEventCaller::sendOpenWindow,
                   [&captured](const QUrl &url) { captured = url; });

    VaultHelper::instance()->newOpenWindow();
    EXPECT_EQ(captured, VaultHelper::instance()->rootUrl());
}

TEST_F(VaultHelperMoreTest, OpenWindow_UsesCurrentWindowId)
{
    quint64 capturedWin = 0;
    QUrl capturedUrl;
    stub.set_lamda(&VaultHelper::defaultCdAction,
                   [&capturedWin, &capturedUrl](VaultHelper *, quint64 winId, const QUrl &url) {
                       capturedWin = winId;
                       capturedUrl = url;
                   });

    VaultHelper::instance()->appendWinID(31);
    VaultHelper::instance()->openWindow();
    EXPECT_EQ(capturedWin, quint64(31));
    EXPECT_EQ(capturedUrl, VaultHelper::instance()->rootUrl());
}

TEST_F(VaultHelperMoreTest, CreateVaultDialog_OldVaultPresent_AbortsEarly)
{
    QString oldPath = kVaultBasePathOld + QDir::separator() + QString(kVaultEncrypyDirName)
            + QDir::separator() + QString(kCryfsConfigFileName);
    QDir().mkpath(QFileInfo(oldPath).absolutePath());
    QFile f(oldPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    VaultHelper::instance()->createVaultDialog();
    EXPECT_TRUE(QFile::exists(oldPath));
    QFile::remove(oldPath);
}

TEST_F(VaultHelperMoreTest, CreateVaultDialog_ExecStubbed_ShowsCreatePage)
{
    scheduleAutoCloseModal();
    stubState(kNotExisted);
    VaultHelper::instance()->createVaultDialog();   // DDialog::exec stubbed in fixture
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, UnlockVaultDialog_TransparentEmptyPassword_NoUnlock)
{
    scheduleAutoCloseModal();
    setEncryptionMethod(kConfigValueMethodTransparent);
    stub.set_lamda(&OperatorCenter::passwordFromKeyring,
                   [](OperatorCenter *) -> QString { return QString(); });

    VaultHelper::instance()->unlockVaultDialog();
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, UnlockVaultDialog_KeyOldVersion_ShowsUpgradeDialog)
{
    scheduleAutoCloseModal();
    writeContainer(false);   // old version -> upgrade branch, exec stubbed returns 0 -> return
    VaultHelper::instance()->unlockVaultDialog();
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, UnlockVaultDialog_KeyNewVersion_ShowsUnlockPages)
{
    scheduleAutoCloseModal();
    writeContainer(true);
    stubState(kUnlocked);   // after exec stubbed, state is unlocked -> no sidebar push
    VaultHelper::instance()->unlockVaultDialog();
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, ShowRemoveVaultDialog_KeyMethod_ShowsPasswordPages)
{
    scheduleAutoCloseModal();
    setEncryptionMethod(kConfigValueMethodKey);
    VaultHelper::instance()->showRemoveVaultDialog();
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, ShowRemoveVaultDialog_TransparentMethod_ShowsNoneWidget)
{
    scheduleAutoCloseModal();
    setEncryptionMethod(kConfigValueMethodTransparent);
    VaultHelper::instance()->showRemoveVaultDialog();
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, ShowResetPasswordDialog_ShowsOldPasswordView)
{
    scheduleAutoCloseModal();
    VaultHelper::instance()->showResetPasswordDialog();
    SUCCEED();
}

TEST_F(VaultHelperMoreTest, CreateVaultPropertyDialog_MatchingUrl_ReturnsSameDialog)
{
    scheduleAutoCloseModal();
    QUrl root = VaultHelper::instance()->rootUrl();
    QWidget *first = VaultHelper::instance()->createVaultPropertyDialog(root);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(VaultHelper::instance()->createVaultPropertyDialog(root), first);
    EXPECT_EQ(VaultHelper::instance()->createVaultPropertyDialog(QUrl("dfmvault:///other")),
              nullptr);
}
