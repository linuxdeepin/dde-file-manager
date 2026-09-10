// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include "stubext.h"

#include "views/vaultunlockpages.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "events/vaulteventcaller.h"

DPVAULT_USE_NAMESPACE

class VaultUnlockPagesTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&VaultEventCaller::sendItemActived, [](quint64, const QUrl &) {});
        stub.set_lamda(&OperatorCenter::checkPassword, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        stub.set_lamda(&OperatorCenter::isNewVaultVersion, []() -> bool { return false; });
        stub.set_lamda(&PathManager::createVaultMountDir, [](const QString &) -> bool { return true; });
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        pages = new VaultUnlockPages();
    }

    void TearDown() override
    {
        stub.clear();
        delete pages;
    }

    VaultUnlockPages *pages = nullptr;
};

TEST_F(VaultUnlockPagesTest, Constructor_CreatesView)
{
    EXPECT_NE(pages, nullptr);
}

TEST_F(VaultUnlockPagesTest, PageSelect_UnlockPage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(PageType::kUnlockPage));
}

TEST_F(VaultUnlockPagesTest, PageSelect_RecoveryKeyPage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(PageType::kRecoverPage));
}

TEST_F(VaultUnlockPagesTest, PageSelect_RetrievePasswordPage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(PageType::kRetrievePage));
}

TEST_F(VaultUnlockPagesTest, PageSelect_PasswordRecoveryPage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(PageType::kPasswordRecoverPage));
}

TEST_F(VaultUnlockPagesTest, OnButtonClicked_NoCrash)
{
    pages->pageSelect(PageType::kUnlockPage);
    EXPECT_NO_FATAL_FAILURE(pages->onButtonClicked(0, "test"));
}

TEST_F(VaultUnlockPagesTest, OnSetBtnEnabled_NoCrash)
{
    pages->pageSelect(PageType::kUnlockPage);
    EXPECT_NO_FATAL_FAILURE(pages->onSetBtnEnabled(0, true));
}

TEST_F(VaultUnlockPagesTest, SetOldPasswordSchemeMigrationMode_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->setOldPasswordSchemeMigrationMode(true));
}

TEST_F(VaultUnlockPagesTest, IsOldPasswordSchemeMigrationMode_DefaultFalse)
{
    EXPECT_FALSE(pages->isOldPasswordSchemeMigrationMode());
}
