// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>

#include "stubext.h"

#include "views/unlockview/unlockview.h"

DPVAULT_USE_NAMESPACE

class UnlockViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new UnlockView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    UnlockView *view = nullptr;
};

// --- construction (covers initUI) ---

TEST_F(UnlockViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

// --- btnText ---

TEST_F(UnlockViewTest, BtnText_Default_ReturnsUnlockButton)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
    // Default: Cancel + Unlock
    EXPECT_FALSE(btns[1].isEmpty());
}

TEST_F(UnlockViewTest, BtnText_MigrationMode_ReturnsVerifyPasswordButton)
{
    view->setOldPasswordSchemeMigrationMode(true);
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

// --- titleText ---

TEST_F(UnlockViewTest, TitleText_Default_ReturnsUnlockTitle)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(UnlockViewTest, TitleText_MigrationMode_ReturnsMigrationTitle)
{
    view->setOldPasswordSchemeMigrationMode(true);
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

// --- setOldPasswordSchemeMigrationMode / isOldPasswordSchemeMigrationMode ---

TEST_F(UnlockViewTest, IsOldPasswordSchemeMigrationMode_DefaultFalse)
{
    EXPECT_FALSE(view->isOldPasswordSchemeMigrationMode());
}

TEST_F(UnlockViewTest, SetOldPasswordSchemeMigrationMode_True)
{
    view->setOldPasswordSchemeMigrationMode(true);
    EXPECT_TRUE(view->isOldPasswordSchemeMigrationMode());
}

TEST_F(UnlockViewTest, SetOldPasswordSchemeMigrationMode_False)
{
    view->setOldPasswordSchemeMigrationMode(true);
    view->setOldPasswordSchemeMigrationMode(false);
    EXPECT_FALSE(view->isOldPasswordSchemeMigrationMode());
}

// --- onPasswordChanged (slot, changes password state) ---

TEST_F(UnlockViewTest, OnPasswordChanged_Empty_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onPasswordChanged(""));
}

TEST_F(UnlockViewTest, OnPasswordChanged_NonEmpty_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onPasswordChanged("testpassword"));
}

// --- onVaultUlocked (slot) ---

TEST_F(UnlockViewTest, OnVaultUlocked_StateZero_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onVaultUlocked(0));
}

// --- buttonClicked (emits signals) ---

TEST_F(UnlockViewTest, ButtonClicked_CancelIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "Cancel"));
}
