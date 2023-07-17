// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/vaultcreatepage.h"

#include <QStackedWidget>

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_VaultActiveView, slotNextWidget)
{
    VaultActiveView view;
    view.slotNextWidget();
    EXPECT_TRUE(view.stackedWidget->currentIndex() == 1);
}

TEST(UT_VaultActiveView, setBegingState)
{
    VaultActiveView view;
    view.setBeginingState();
    EXPECT_TRUE(view.stackedWidget->currentIndex() == 0);
}
