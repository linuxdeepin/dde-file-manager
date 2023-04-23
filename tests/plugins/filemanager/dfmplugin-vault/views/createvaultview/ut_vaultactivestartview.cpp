// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/createvaultview/vaultactivestartview.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_VaultActiveStartView, slotStartBtnClicked)
{
    VaultActiveStartView view;
    view.slotStartBtnClicked();

    EXPECT_TRUE(view.startBtn != nullptr);
}
