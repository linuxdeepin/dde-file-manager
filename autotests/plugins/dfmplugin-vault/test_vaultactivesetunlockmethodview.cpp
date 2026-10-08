// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>

#include "stubext.h"

#define private public
#define protected public
#include "views/createvaultview/vaultactivesetunlockmethodview.h"
#undef protected
#undef private
#include "utils/encryption/vaultconfig.h"

DPVAULT_USE_NAMESPACE

class VaultActiveSetUnlockMethodViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new VaultActiveSetUnlockMethodView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultActiveSetUnlockMethodView *view = nullptr;
};

// --- construction ---

TEST_F(VaultActiveSetUnlockMethodViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

// --- clearText ---

TEST_F(VaultActiveSetUnlockMethodViewTest, ClearText_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->clearText());
}

// --- slotPasswordEditing ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotPasswordEditing_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotPasswordEditing());
}

// --- slotPasswordEditFinished ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotPasswordEditFinished_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotPasswordEditFinished());
}

// --- slotRepeatPasswordEditFinished ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotRepeatPasswordEditFinished_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotRepeatPasswordEditFinished());
}

// --- slotRepeatPasswordEditing ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotRepeatPasswordEditing_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotRepeatPasswordEditing());
}

// --- slotGenerateEditChanged ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotGenerateEditChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotGenerateEditChanged("test"));
}

// --- slotTypeChanged ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotTypeChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotTypeChanged(0));
    EXPECT_NO_FATAL_FAILURE(view->slotTypeChanged(1));
}

// --- slotPasswordEditFocusChanged ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotPasswordEditFocusChanged_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotPasswordEditFocusChanged(true));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotPasswordEditFocusChanged_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotPasswordEditFocusChanged(false));
}

// --- slotRepeatPasswordEditFocusChanged ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotRepeatPasswordEditFocusChanged_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotRepeatPasswordEditFocusChanged(true));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotRepeatPasswordEditFocusChanged_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotRepeatPasswordEditFocusChanged(false));
}

// --- setEncryptInfo ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SetEncryptInfo_NoCrash)
{
    EncryptInfo info;
    EXPECT_NO_FATAL_FAILURE(view->setEncryptInfo(info));
}

// --- slotLimiPasswordLength ---

TEST_F(VaultActiveSetUnlockMethodViewTest, SlotLimiPasswordLength_Short_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotLimiPasswordLength("short"));
}

// ---------------------------------------------------------------------------
// PMS sev-2 regression additions (vault creation view)
// ---------------------------------------------------------------------------
#include <type_traits>
#include <DComboBox>
#include "views/createvaultview/vaultbaseview.h"

DWIDGET_USE_NAMESPACE

// PMS:314353 the creation page must be rooted on VaultBaseView (shared creation
// info plumbing), offer both key/transparent encryption modes and start with the
// next button disabled so a half-built page (e.g. full-disk) cannot advance.
TEST_F(VaultActiveSetUnlockMethodViewTest, BUG314353_InitUiOffersBothEncryptModes)
{
    static_assert(std::is_base_of<VaultBaseView, VaultActiveSetUnlockMethodView>::value,
                  "VaultActiveSetUnlockMethodView must derive from VaultBaseView (BUG-314353)");

    ASSERT_NE(view->typeCombo, nullptr);
    EXPECT_EQ(view->typeCombo->count(), 2);
    EXPECT_EQ(view->typeCombo->itemData(0).toInt(), static_cast<int>(EncryptMode::kKeyMode));
    EXPECT_EQ(view->typeCombo->itemData(1).toInt(), static_cast<int>(EncryptMode::kTransparentMode));
    EXPECT_NE(view->nextBtn, nullptr);
    EXPECT_FALSE(view->nextBtn->isEnabled());
}
