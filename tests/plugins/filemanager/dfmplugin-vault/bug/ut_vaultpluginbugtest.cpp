// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/unlockview/retrievepasswordview.h"
#include "vault.h"
#include "utils/vaultvisiblemanager.h"
#include "views/createvaultview/vaultactivesetunlockmethodview.h"

#include <gtest/gtest.h>

#include <DComboBox>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(UT_VaultPluginBugTest, bug_193745_WindowNoStaysOnTop)
{
    RetrievePasswordView view;
    view.onComboBoxIndex(1);

    Qt::WindowFlags flags = view.fileDialog->windowFlags();
    EXPECT_TRUE(flags & Qt::WindowStaysOnTopHint);
}

TEST(UT_VaultPluginBugTest, bug_178305_NotRegisterService)
{
    bool isRegisterService { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultVisibleManager::pluginServiceRegister, [ &isRegisterService ] { __DBG_STUB_INVOKE__ isRegisterService = true; });

    Vault plugin;
    plugin.initialize();
    EXPECT_TRUE(isRegisterService);
}

TEST(UT_VaultPluginBugTest, bug_177631_NotHaveTransparentEncryptionFunction)
{
    VaultActiveSetUnlockMethodView view;
    int count = view.typeCombo->count();
    EXPECT_TRUE(count > 1);
    if (count > 1) {
        EXPECT_TRUE(view.typeCombo->itemText(1) == VaultActiveSetUnlockMethodView::tr("Transparent encryption"));
    }
}
