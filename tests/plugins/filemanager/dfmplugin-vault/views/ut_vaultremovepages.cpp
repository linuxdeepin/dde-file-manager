// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/vaultremovepages.h"
#include "views/removevaultview/vaultremovebypasswordview.h"
#include "views/removevaultview/vaultremoveprogressview.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"

#include <dfm-framework/event/eventdispatcher.h>

#include <QStackedWidget>
#include <QCloseEvent>

#include <DDialog>

#include <gtest/gtest.h>
#include <stubext.h>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPF_USE_NAMESPACE

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
    stub.set_lamda(&VaultUtils::showAuthorityDialog, [ &checkAuthorization ]{ checkAuthorization = true; });

    VaultRemovePages page;
    page.onButtonClicked(0, "");

    EXPECT_FALSE(checkAuthorization);
}
