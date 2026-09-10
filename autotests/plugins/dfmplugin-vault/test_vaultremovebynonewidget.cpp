// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>

#include "stubext.h"

#include "views/removevaultview/vaultremovebynonewidget.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"

DPVAULT_USE_NAMESPACE

class VaultRemoveByNoneWidgetTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&VaultUtils::showAuthorityDialog, [](VaultUtils *, const QString &) {});
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        view = new VaultRemoveByNoneWidget();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultRemoveByNoneWidget *view = nullptr;
};

TEST_F(VaultRemoveByNoneWidgetTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultRemoveByNoneWidgetTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(VaultRemoveByNoneWidgetTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(VaultRemoveByNoneWidgetTest, ButtonClicked_IndexZero_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "Cancel"));
}

TEST_F(VaultRemoveByNoneWidgetTest, ButtonClicked_IndexOne_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "Delete"));
}

TEST_F(VaultRemoveByNoneWidgetTest, SlotCheckAuthorizationFinished_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(true));
}

TEST_F(VaultRemoveByNoneWidgetTest, SlotCheckAuthorizationFinished_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(false));
}
