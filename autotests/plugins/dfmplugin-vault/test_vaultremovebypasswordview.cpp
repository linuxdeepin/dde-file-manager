// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>

#include "stubext.h"

#include "views/removevaultview/vaultremovebypasswordview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"

DPVAULT_USE_NAMESPACE

class VaultRemoveByPasswordViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&VaultUtils::showAuthorityDialog, [](VaultUtils *, const QString &) {});
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        stub.set_lamda(&OperatorCenter::checkPassword, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        view = new VaultRemoveByPasswordView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultRemoveByPasswordView *view = nullptr;
};

TEST_F(VaultRemoveByPasswordViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultRemoveByPasswordViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(VaultRemoveByPasswordViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(VaultRemoveByPasswordViewTest, OnPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onPasswordChanged("testpassword"));
}

TEST_F(VaultRemoveByPasswordViewTest, OnPasswordChanged_Empty_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onPasswordChanged(""));
}

TEST_F(VaultRemoveByPasswordViewTest, ButtonClicked_IndexZero_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "Cancel"));
}

TEST_F(VaultRemoveByPasswordViewTest, ButtonClicked_IndexOne_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "Delete"));
}

TEST_F(VaultRemoveByPasswordViewTest, ShowAlertMessage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("test alert", 100));
}

TEST_F(VaultRemoveByPasswordViewTest, ShowToolTip_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showToolTip("test tip", 100, VaultRemoveByPasswordView::kWarning));
}

TEST_F(VaultRemoveByPasswordViewTest, SetTipsButtonVisible_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setTipsButtonVisible(true));
    EXPECT_NO_FATAL_FAILURE(view->setTipsButtonVisible(false));
}

TEST_F(VaultRemoveByPasswordViewTest, SlotCheckAuthorizationFinished_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(true));
}

TEST_F(VaultRemoveByPasswordViewTest, SlotCheckAuthorizationFinished_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(false));
}
