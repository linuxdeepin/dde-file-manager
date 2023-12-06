// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/removevaultview/vaultremoveprogressview.h"

#include <gtest/gtest.h>

#include <DWaterProgress>

#include <QDir>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(UT_VaultRemoveProgressView, removeVault)
{
    bool isStart { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&DWaterProgress::start, [ &isStart ] { __DBG_STUB_INVOKE__ isStart = true; });

    VaultRemoveProgressView view;
    view.removeVault("");
    EXPECT_FALSE(isStart);
}

