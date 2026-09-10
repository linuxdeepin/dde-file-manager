// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include "stubext.h"

#include "views/vaultremovepages.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"
#include "events/vaulteventcaller.h"

DPVAULT_USE_NAMESPACE

class VaultRemovePagesTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&VaultEventCaller::sendItemActived, [](quint64, const QUrl &) {});
        stub.set_lamda(&VaultUtils::showAuthorityDialog, [](VaultUtils *, const QString &) {});
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        stub.set_lamda(&OperatorCenter::removeVault, [](OperatorCenter *, const QString &) {});
        stub.set_lamda(&OperatorCenter::checkPassword, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        pages = new VaultRemovePages();
    }

    void TearDown() override
    {
        stub.clear();
        delete pages;
    }

    VaultRemovePages *pages = nullptr;
};

TEST_F(VaultRemovePagesTest, Constructor_CreatesView)
{
    EXPECT_NE(pages, nullptr);
}

TEST_F(VaultRemovePagesTest, PageSelect_PasswordWidget_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(RemoveWidgetType::kPasswordWidget));
}

TEST_F(VaultRemovePagesTest, PageSelect_RecoveryKeyWidget_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(RemoveWidgetType::kRecoveryKeyWidget));
}

TEST_F(VaultRemovePagesTest, PageSelect_NoneWidget_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(RemoveWidgetType::kNoneWidget));
}

TEST_F(VaultRemovePagesTest, PageSelect_ProgressWidget_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->pageSelect(RemoveWidgetType::kRemoveProgressWidget));
}

TEST_F(VaultRemovePagesTest, OnButtonClicked_NoCrash)
{
    pages->pageSelect(RemoveWidgetType::kPasswordWidget);
    EXPECT_NO_FATAL_FAILURE(pages->onButtonClicked(0, "test"));
}

TEST_F(VaultRemovePagesTest, SetBtnEnable_NoCrash)
{
    pages->pageSelect(RemoveWidgetType::kPasswordWidget);
    EXPECT_NO_FATAL_FAILURE(pages->setBtnEnable(0, true));
}
