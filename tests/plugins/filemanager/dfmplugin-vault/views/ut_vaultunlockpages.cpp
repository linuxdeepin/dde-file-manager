// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/vaultunlockpages.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_VaultUnlockPages, pageSelect_kUnlockPage)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UnlockView::buttonClicked, []{});

    VaultUnlockPages page;
    page.pageSelect(PageType::kUnlockPage);
    page.onButtonClicked(0, "");

    EXPECT_TRUE(page.unlockView != NULL);
}

TEST(UT_VaultUnlockPages, pageSelect_kRetrievePage)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&RecoveryKeyView::buttonClicked, []{});

    VaultUnlockPages page;
    page.pageSelect(PageType::kRetrievePage);
    page.onButtonClicked(0, "");

    EXPECT_TRUE(page.retrievePasswordView != NULL);
}

TEST(UT_VaultUnlockPages, pageSelect_kPasswordRecoverPage)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PasswordRecoveryView::buttonClicked, []{});

    VaultUnlockPages page;
    page.pageSelect(PageType::kPasswordRecoverPage);
    page.onButtonClicked(0, "");

    EXPECT_TRUE(page.passwordRecoveryView != NULL);
}

TEST(UT_VaultUnlockPage, onSetBtnEnabled)
{
    VaultUnlockPages page;
    page.addButton("");
    page.onSetBtnEnabled(0, true);

    EXPECT_TRUE(page.getButton(0)->isEnabled());
}
