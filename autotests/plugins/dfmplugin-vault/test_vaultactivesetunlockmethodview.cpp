// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>

#include "stubext.h"

#include "views/createvaultview/vaultactivesetunlockmethodview.h"
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
