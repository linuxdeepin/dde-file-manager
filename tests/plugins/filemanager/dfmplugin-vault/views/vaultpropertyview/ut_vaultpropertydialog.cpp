// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/vaultpropertyview/vaultpropertydialog.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_VaultPropertyDialog, selectFileUrl)
{
    VaultPropertyDialog dialog;
    dialog.selectFileUrl(QUrl("file:///Ut_test"));

    EXPECT_TRUE(dialog.fileIconLabel != nullptr && dialog.basicWidget != nullptr);
}
