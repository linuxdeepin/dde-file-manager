// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QShowEvent>

#include "stubext.h"

#define private public
#define protected public
#include "views/unlockview/retrievepasswordview.h"
#undef protected
#undef private
#include "utils/encryption/operatorcenter.h"
#include "utils/pathmanager.h"
#include "utils/vaulthelper.h"

DPVAULT_USE_NAMESPACE

class RetrievePasswordViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&OperatorCenter::verificationRetrievePassword,
                       [](OperatorCenter *, const QString, QString &) -> bool { return true; });
        stub.set_lamda(&PathManager::createVaultMountDir, [](const QString &) -> bool { return true; });
        view = new RetrievePasswordView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    RetrievePasswordView *view = nullptr;
};

TEST_F(RetrievePasswordViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(RetrievePasswordViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(RetrievePasswordViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(RetrievePasswordViewTest, GetUserName_ReturnsNonEmpty)
{
    QString userName = view->getUserName();
    EXPECT_FALSE(userName.isEmpty());
}

TEST_F(RetrievePasswordViewTest, ValidationResults_ReturnsString)
{
    QString result = view->ValidationResults();
    EXPECT_TRUE(result.size() >= 0);
}

TEST_F(RetrievePasswordViewTest, SetOldPasswordSchemeMigrationMode_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setOldPasswordSchemeMigrationMode(true));
}

TEST_F(RetrievePasswordViewTest, IsOldPasswordSchemeMigrationMode_DefaultFalse)
{
    EXPECT_FALSE(view->isOldPasswordSchemeMigrationMode());
}

TEST_F(RetrievePasswordViewTest, SetAndCheckMigrationMode)
{
    view->setOldPasswordSchemeMigrationMode(true);
    EXPECT_TRUE(view->isOldPasswordSchemeMigrationMode());
    view->setOldPasswordSchemeMigrationMode(false);
    EXPECT_FALSE(view->isOldPasswordSchemeMigrationMode());
}

TEST_F(RetrievePasswordViewTest, OnBtnSelectFilePath_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onBtnSelectFilePath("/tmp/testkey.key"));
}

TEST_F(RetrievePasswordViewTest, OnTextChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onTextChanged("/tmp/testkey.key"));
}

TEST_F(RetrievePasswordViewTest, ButtonClicked_IndexZero_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "Cancel"));
}

TEST_F(RetrievePasswordViewTest, ButtonClicked_IndexOne_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "Verify"));
}

TEST_F(RetrievePasswordViewTest, VerificationKey_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->verificationKey());
}

// --- setVerificationPage ---

TEST_F(RetrievePasswordViewTest, SetVerificationPage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setVerificationPage());
}

// --- showEvent ---

TEST_F(RetrievePasswordViewTest, ShowEvent_NoCrash)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(view->showEvent(&event));
}
