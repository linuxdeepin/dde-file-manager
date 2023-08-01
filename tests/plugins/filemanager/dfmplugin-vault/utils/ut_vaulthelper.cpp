// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/vaulthelper.h"
#include "utils/fileencrypthandle.h"
#include "utils/vaultautolock.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"
#include "events/vaulteventcaller.h"
#include "views/vaultpropertyview/vaultpropertydialog.h"

#include <gtest/gtest.h>

#include <QUrl>
#include <QApplication>

#include <DMenu>

#include <dfm-framework/event/event.h>

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/application/settings.h>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultHelper, rootUrl)
{
    QUrl result = VaultHelper::instance()->rootUrl();

    EXPECT_TRUE(result == QUrl("dfmvault:///"));
}

TEST(UT_VaultHelper, sourceRootUrl)
{
    QUrl result = VaultHelper::instance()->sourceRootUrl();

    EXPECT_TRUE(result.path().endsWith(kVaultDecryptDirName));
}

TEST(UT_VaultHelper, sourceRootUrlWithSlash)
{
    QUrl result = VaultHelper::instance()->sourceRootUrlWithSlash();

    EXPECT_TRUE(result.path().endsWith(QString(kVaultDecryptDirName) + QDir::separator()));
}

TEST(UT_VaultHelper, pathToVaultVirtualUrl_one)
{
    QUrl result = VaultHelper::instance()->pathToVaultVirtualUrl("");

    EXPECT_FALSE(result.isValid());
}

TEST(UT_VaultHelper, pathToVaultVirtualUrl_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::sourceRootUrl, []{
        return QUrl("file:///UT_TEST");
    });

    QUrl result = VaultHelper::instance()->pathToVaultVirtualUrl("/UT_TEST");

    EXPECT_TRUE(result == QUrl("dfmvault:///"));
}

TEST(UT_VaultHelper, pathToVaultVirtualUrl_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::sourceRootUrl, []{
        return QUrl("file:///UT_TEST");
    });

    QUrl result = VaultHelper::instance()->pathToVaultVirtualUrl("/UT_TEST/UT_TEST1");

    EXPECT_TRUE(result == QUrl("dfmvault:///UT_TEST1"));
}

TEST(UT_VaultHelper, contenxtMenuHandle)
{
    bool isOk { false };

    QAction action;
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::createMenu, []{
        return new DMenu;
    });
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, QWidget *, char const (&)[23]);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), []{
        return QVariant();
    });
    typedef QAction*(QMenu::*FuncType2)(const QPoint &, QAction *);
    stub.set_lamda(static_cast<FuncType2>(&QMenu::exec), [&action, &isOk]{
        isOk = true;
        return &action;
    });
    typedef bool(EventDispatcherManager::*FuncType3)(const QString &, const QString &, QString, QList<QUrl> &);
    stub.set_lamda(static_cast<FuncType3>(&EventDispatcherManager::publish), []{
        return true;
    });

    VaultHelper::instance()->contenxtMenuHandle(0, QUrl("file:///UT_TEST"), QPoint(0, 0));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, siderItemClicked_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QApplication::restoreOverrideCursor, []{});
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotExisted;
    });
    stub.set_lamda(&VaultHelper::createVaultDialog, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->siderItemClicked(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, siderItemClicked_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QApplication::restoreOverrideCursor, []{});
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kEncrypted;
    });
    stub.set_lamda(&VaultHelper::unlockVaultDialog, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->siderItemClicked(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, siderItemClicked_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QApplication::restoreOverrideCursor, []{});
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnlocked;
    });
    stub.set_lamda(&VaultHelper::defaultCdAction, [ &isOk ]{
        isOk = true;
    });
    stub.set_lamda(&VaultHelper::recordTime, []{});

    VaultHelper::instance()->siderItemClicked(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, siderItemClicked_four)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QApplication::restoreOverrideCursor, []{});
    stub.set_lamda(&VaultHelper::state, [ &isOk ]{
        isOk = true;
        return VaultState::kUnderProcess;
    });

    VaultHelper::instance()->siderItemClicked(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, siderItemClicked_five)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QApplication::restoreOverrideCursor, []{});
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotAvailable;
    });
    stub.set_lamda(&DialogManager::showErrorDialog, [ &isOk ]{
        isOk = true;
    });


    VaultHelper::instance()->siderItemClicked(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, siderItemClicked_six)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QApplication::restoreOverrideCursor, []{});
    stub.set_lamda(&VaultHelper::state, [ &isOk ]{
        isOk = true;
        return VaultState::kUnknow;
    });

    VaultHelper::instance()->siderItemClicked(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, state)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::state, [ &isOk ]{
        isOk = true;
        return kUnknow;
    });

    VaultHelper::instance()->state("/UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, updateState)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::updateState, [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultHelper::instance()->updateState(kUnknow);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, defaultCdAction)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultEventCaller::sendItemActived, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->defaultCdAction(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, openNewWindow)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultEventCaller::sendOpenWindow, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->openNewWindow(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, getVaultVersion_one)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant("UT_TEST");
    });

    bool isOk = VaultHelper::instance()->getVaultVersion();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, getVaultVersion_two)
{
    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant(kConfigVaultVersion);
    });

    bool isOk = VaultHelper::instance()->getVaultVersion();

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultHelper, currentWindowId)
{
    VaultHelper::instance()->currentWinID = 0;
    EXPECT_TRUE(VaultHelper::instance()->currentWindowId() == 0);
}

TEST(UT_VaultHelper, removeWinID)
{
    VaultHelper::instance()->appendWinID(1);
    VaultHelper::instance()->removeWinID(1);

    EXPECT_FALSE(VaultHelper::instance()->winIDs.contains(1));
}

TEST(UT_VaultHelper, createMenu_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotExisted;
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createMenu_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kEncrypted;
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createMenu_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnlocked;
    });
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigValueMethodKey);
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createMenu_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnderProcess;
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createMenu_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kBroken;
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createMenu_six)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotAvailable;
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createMenu_seven)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnknow;
    });

    DMenu *menu = VaultHelper::instance()->createMenu();

    EXPECT_TRUE(menu);

    delete menu;
}

TEST(UT_VaultHelper, createVaultPropertyDialog_one)
{
    QWidget *dlg = VaultHelper::instance()->createVaultPropertyDialog(QUrl("dfmvault:///"));

    EXPECT_TRUE(dlg);
}

TEST(UT_VaultHelper, createVaultPropertyDialog_two)
{
    QWidget *dlg = VaultHelper::instance()->createVaultPropertyDialog(QUrl("dfmvault:///"));

    EXPECT_TRUE(dlg);
}

TEST(UT_VaultHelper, createVaultPropertyDialog_three)
{
    QWidget *dlg = VaultHelper::instance()->createVaultPropertyDialog(QUrl("dfmvault:///UT_TEST"));

    EXPECT_FALSE(dlg);
}

TEST(UT_VaultHelper, vaultToLocalUrl_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::sourceRootUrl, []{
        return QUrl("file:///UT_TEST");
    });

    VaultHelper::instance()->vaultToLocalUrl(QUrl("file:///UT_TEST"));

    QUrl result = VaultHelper::instance()->vaultToLocalUrl(QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(result == QUrl("file:///UT_TEST"));
}

TEST(UT_VaultHelper, vaultToLocalUrl_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::sourceRootUrl, []{
        return QUrl("file:///UT_TEST/UT");
    });

    VaultHelper::instance()->vaultToLocalUrl(QUrl("file:///UT_TEST"));

    QUrl result = VaultHelper::instance()->vaultToLocalUrl(QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(result == QUrl("file:///UT_TEST/UT/UT_TEST"));
}

TEST(UT_VaultHelper, createVault)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::encryptAlgoTypeOfGroupPolicy, []{
        return EncryptType::AES_256_GCM;
    });
    stub.set_lamda(&FileEncryptHandle::createVault, [ &isOk ]{
        isOk = true;;
    });

    QString password;
    VaultHelper::instance()->createVault(password);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, unlockVault)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::unlockVault, [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultHelper::instance()->unlockVault("UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, createVaultDialog)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(DDialog, exec), [ &isOk ]{
        isOk = true;
        return 0;
    });
    stub.set_lamda(&VaultHelper::state, []{
        return kNotExisted;
    });
    typedef QVariant(EventChannelManager::*FuncType)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), []{
        return QVariant();
    });

    VaultHelper::instance()->createVaultDialog();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, unlockVaultDialog_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigValueMethodTransparent);
    });
    stub.set_lamda(&OperatorCenter::passwordFromKeyring, []{
        return "UT_TEST";
    });
    stub.set_lamda(&VaultHelper::unlockVault, [ &isOk ]{
        isOk = true;
        return true;
    });
    stub.set_lamda(&VaultHelper::defaultCdAction, []{});
    stub.set_lamda(&VaultHelper::recordTime, []{});

    VaultHelper::instance()->unlockVaultDialog();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, unlockVaultDialog_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigValueMethodTransparent);
    });
    stub.set_lamda(&OperatorCenter::passwordFromKeyring, [ &isOk ]{
        isOk = true;
        return "";
    });

    VaultHelper::instance()->unlockVaultDialog();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, unlockVaultDialog_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef QVariant(VaultConfig::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });
    stub.set_lamda(VADDR(DDialog, exec), []{
        return 0;
    });
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotExisted;
    });
    typedef QVariant(EventChannelManager::*FuncType2)(const QString &, const QString &, quint64);
    stub.set_lamda(static_cast<FuncType2>(&EventChannelManager::push), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultHelper::instance()->unlockVaultDialog();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, removeVaultDialog)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(DDialog, exec), [ &isOk ]{
        isOk = true;
        return 0;
    });

    VaultHelper::instance()->removeVaultDialog();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, openWindow)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::defaultCdAction, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->openWindow();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, openWidWindow)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultEventCaller::sendItemActived, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->openWidWindow(0, QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, newOpenWindow)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::openNewWindow, [ &isOk ]{
        isOk = true;
    });
    stub.set_lamda(&VaultHelper::recordTime, []{
        int a = 0;
    });

    VaultHelper::instance()->newOpenWindow();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, slotlockVault)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultAutoLock::slotLockVault, [ &isOk ]{
        isOk = true;
    });
    stub.set_lamda(&VaultHelper::defaultCdAction, []{});
    stub.set_lamda(&VaultHelper::recordTime, []{});

    VaultHelper::instance()->winIDs.push_back(0);
    VaultHelper::instance()->slotlockVault(0);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, recordTime)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(Settings::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&Settings::setValue), [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->recordTime("UT_GROUP", "UT_KEY");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, isVaultFile)
{
    bool re1 = VaultHelper::instance()->isVaultFile(QUrl("dfmvault:///UT_TEST"));
    bool re2 = VaultHelper::instance()->isVaultFile(QUrl("file:///UT_TEST"));

    EXPECT_TRUE((re1 == true) && (re2 == false));
}

TEST(UT_VaultHelper, urlsToLocal)
{
    QList<QUrl> list;
    VaultHelper::instance()->urlsToLocal(QList<QUrl>(), nullptr);
    VaultHelper::instance()->urlsToLocal(QList<QUrl>() << QUrl("file:///UT_TEST"), &list);
    bool isOk = VaultHelper::instance()->urlsToLocal(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), &list);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultHelper, showInProgressDailog)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&DialogManager::showErrorDialog, [ &isOk ]{
        isOk = true;
    });

    VaultHelper::instance()->showInProgressDailog("Device or resource busy");

    EXPECT_TRUE(isOk);
}
