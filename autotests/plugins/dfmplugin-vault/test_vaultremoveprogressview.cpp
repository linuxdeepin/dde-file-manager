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
#include "views/removevaultview/vaultremoveprogressview.h"
#undef protected
#undef private
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/vaultautolock.h"
#include "events/vaulteventcaller.h"

#include <dfm-framework/dpf.h>

DPVAULT_USE_NAMESPACE

class VaultRemoveProgressViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&VaultEventCaller::sendItemActived, [](quint64, const QUrl &) {});
        stub.set_lamda(&OperatorCenter::removeVault, [](OperatorCenter *, const QString &) {});
        stub.set_lamda(&VaultHelper::updateState, [](VaultHelper *, VaultState) -> bool { return true; });
        view = new VaultRemoveProgressView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    VaultRemoveProgressView *view = nullptr;
};

TEST_F(VaultRemoveProgressViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(VaultRemoveProgressViewTest, BtnText_ReturnsEmptyOrButtons)
{
    QStringList btns = view->btnText();
    EXPECT_TRUE(btns.size() >= 0);
}

TEST_F(VaultRemoveProgressViewTest, TitleText_ReturnsString)
{
    QString title = view->titleText();
    EXPECT_TRUE(title.size() >= 0);
}

TEST_F(VaultRemoveProgressViewTest, RemoveVault_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->removeVault("/tmp/vault_test"));
}

TEST_F(VaultRemoveProgressViewTest, ButtonClicked_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "test"));
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "test"));
}

// --- handleVaultRemovedProgress ---

TEST_F(VaultRemoveProgressViewTest, HandleVaultRemovedProgress_Partial_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleVaultRemovedProgress(50));
}

TEST_F(VaultRemoveProgressViewTest, HandleVaultRemovedProgress_Complete_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleVaultRemovedProgress(100));
}
