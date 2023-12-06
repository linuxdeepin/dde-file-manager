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
    page.pageSelect(kPasswordWidget);
    EXPECT_TRUE(page.passwordView != Q_NULLPTR);
}

