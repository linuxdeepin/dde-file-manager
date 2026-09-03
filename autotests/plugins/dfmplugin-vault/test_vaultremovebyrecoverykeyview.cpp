// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>

#include "stubext.h"

#include "views/removevaultview/vaultremovebyrecoverykeyview.h"

DPVAULT_USE_NAMESPACE

class VaultRemoveByRecoverykeyViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new VaultRemoveByRecoverykeyView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultRemoveByRecoverykeyView *view = nullptr;
};

// --- construction ---

TEST_F(VaultRemoveByRecoverykeyViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

// --- btnText ---

TEST_F(VaultRemoveByRecoverykeyViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

// --- titleText ---

TEST_F(VaultRemoveByRecoverykeyViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

// --- getRecoverykey ---

TEST_F(VaultRemoveByRecoverykeyViewTest, GetRecoverykey_DefaultEmpty)
{
    EXPECT_TRUE(view->getRecoverykey().isEmpty());
}

// --- onRecoveryKeyChanged (crashes due to afterRecoveryKeyChanged heavy deps, excluded) ---

// --- showAlertMessage ---

TEST_F(VaultRemoveByRecoverykeyViewTest, ShowAlertMessage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("test message", 100));
}

TEST_F(VaultRemoveByRecoverykeyViewTest, ShowAlertMessage_PersistentDisplay_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("persistent", -1));
}
