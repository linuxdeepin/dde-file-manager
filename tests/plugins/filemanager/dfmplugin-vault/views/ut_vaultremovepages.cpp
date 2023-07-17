// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/vaultremovepages.h"
#include "views/removevaultview/vaultremovebypasswordview.h"
#include "views/removevaultview/vaultremoveprogressview.h"
#include "utils/vaulthelper.h"

#include <dfm-framework/event/eventdispatcher.h>

#include <QStackedWidget>
#include <QCloseEvent>

#include <DDialog>

#include <gtest/gtest.h>
#include <stubext.h>

#include <polkit-qt5-1/polkitqt1-authority.h>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace PolkitQt1;
DPF_USE_NAMESPACE

TEST(UT_VaultRemovePages, showRemoveWidget)
{
    VaultRemovePages page;
    page.showRemoveWidget();
    EXPECT_TRUE(page.stackedWidget->currentIndex() == 2);
}

TEST(UT_VaultRemovePages, closeEvent)
{
    VaultRemovePages page;
    QCloseEvent event;
    page.closeEvent(&event);
    EXPECT_FALSE(page.removeVault);
}

TEST(UT_VaultRemovePages, showEvent)
{
    bool isShow { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(DDialog, showEvent), [ &isShow ]{ isShow = true; });

    VaultRemovePages page;
    QShowEvent event;
    page.showEvent(&event);
    EXPECT_TRUE(isShow);
}

TEST(UT_VaultRemovePages, onButtonClicked_cancel)
{
    bool checkAuthorization { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&Authority::checkAuthorization, [ &checkAuthorization ]{ checkAuthorization = true; });

    VaultRemovePages page;
    page.onButtonClicked(0);

    EXPECT_FALSE(checkAuthorization);
}

TEST(UT_VaultRemovePages, onButtonClicked_ok)
{
    bool getPassword { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultRemoveByPasswordView::getPassword, [ &getPassword ]{ getPassword = true; return ""; });


    VaultRemovePages page;
    page.onButtonClicked(1);

    EXPECT_TRUE(getPassword);
}

TEST(UT_VaultRemovePages, slotCheckAuthorizationFinished)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QWidget::isVisible, []{ return true; });
    stub.set_lamda(&VaultHelper::lockVault, []{});

    VaultRemovePages page;
    page.slotCheckAuthorizationFinished(Authority::Yes);

    EXPECT_TRUE(page.removeVault);
}

TEST(UT_VaultRemovePages, onLockVault_state_zero)
{
    bool isRemove { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultRemoveProgressView::removeVault, [ &isRemove ]{ isRemove = true; });

    VaultRemovePages page;
    page.removeVault = true;
    page.onLockVault(0);

    EXPECT_TRUE(isRemove);
}

TEST(UT_VaultRemovePages, onLockVault_state_other)
{
    bool isShowError { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(DDialog, exec), [ &isShowError ]{
        isShowError = true;
        return 0;
    });

    VaultRemovePages page;
    page.removeVault = true;
    page.onLockVault(1);

    EXPECT_TRUE(isShowError);
}

TEST(UT_VaultRemovePages, onVaultRemoveFinish_true)
{
    bool isFinish { false };

    stub_ext::StubExt stub;
    typedef bool (EventDispatcherManager::*FuncType)(const QString &, const QString &, QString, QVariantMap &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), [ &isFinish ] {
        isFinish = true;
        return true;
    });

    VaultRemovePages page;
    page.onVaultRemoveFinish(true);

    EXPECT_TRUE(isFinish);
}

TEST(UT_VaultRemovePages, onVaultRemoveFinish_false)
{
    bool isFinish { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultRemovePages::setInfo, [ &isFinish ]{ isFinish = true; });

    VaultRemovePages page;
    page.onVaultRemoveFinish(true);

    EXPECT_TRUE(isFinish);
}
