// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>

#include "stubext.h"

#include "views/createvaultview/vaultbaseview.h"
#include "views/createvaultview/vaultactivestartview.h"

DPVAULT_USE_NAMESPACE

class VaultBaseViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new VaultActiveStartView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultActiveStartView *view = nullptr;
};

TEST_F(VaultBaseViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultBaseViewTest, SetEncryptInfo_NoCrash)
{
    EncryptInfo info;
    info.mode = EncryptMode::kKeyMode;
    EXPECT_NO_FATAL_FAILURE(view->setEncryptInfo(info));
}
