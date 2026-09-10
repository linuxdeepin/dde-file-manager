// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include "stubext.h"

#include "views/createvaultview/vaultactivefinishedview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"

#include <dfm-framework/dpf.h>

DPVAULT_USE_NAMESPACE

class VaultActiveFinishedViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&VaultUtils::showAuthorityDialog, [](VaultUtils *, const QString &) {});
        stub.set_lamda(&VaultHelper::defaultCdAction, [](VaultHelper *, quint64, const QUrl &) {});
        stub.set_lamda(&VaultHelper::recordTime, [](const QString &, const QString &) {});
        view = new VaultActiveFinishedView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultActiveFinishedView *view = nullptr;
};

TEST_F(VaultActiveFinishedViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultActiveFinishedViewTest, SetFinishedBtnEnabled_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setFinishedBtnEnabled(true));
}

TEST_F(VaultActiveFinishedViewTest, SetFinishedBtnEnabled_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setFinishedBtnEnabled(false));
}

TEST_F(VaultActiveFinishedViewTest, SetProgressValue_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setProgressValue(50));
}

TEST_F(VaultActiveFinishedViewTest, EncryptFinished_Success_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->encryptFinished(true, "success"));
}

TEST_F(VaultActiveFinishedViewTest, EncryptFinished_Failure_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->encryptFinished(false, "error message"));
}

TEST_F(VaultActiveFinishedViewTest, SlotEncryptVault_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotEncryptVault());
}

TEST_F(VaultActiveFinishedViewTest, SlotTimeout_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotTimeout());
}

TEST_F(VaultActiveFinishedViewTest, SlotCheckAuthorizationFinished_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(true));
}

TEST_F(VaultActiveFinishedViewTest, SlotCheckAuthorizationFinished_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->slotCheckAuthorizationFinished(false));
}
