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

// ---------------------------------------------------------------------------
// PMS sev-2 regression additions (retrieve password view)
// ---------------------------------------------------------------------------
#include <DFileChooserEdit>
#include <DDialog>
#include <QtConcurrent>

DWIDGET_USE_NAMESPACE

// PMS:377985 the key-file chooser must restrict the file dialog to *.key files
// (ExistingFiles mode) and must not allow clearing the selection, otherwise
// users can submit an arbitrary/empty key file path.
TEST_F(RetrievePasswordViewTest, BUG377985_KeyFileChooserFilterContract)
{
    Dtk::Widget::DFileChooserEdit *edit = view->filePathEdit;
    ASSERT_NE(edit, nullptr);

    EXPECT_EQ(edit->nameFilters(), QStringList { QStringLiteral("KEY file(*.key)") });
    EXPECT_EQ(edit->fileMode(), Dtk::Widget::DFileDialog::ExistingFiles);

    ASSERT_NE(edit->lineEdit(), nullptr);
    EXPECT_FALSE(edit->lineEdit()->isClearButtonEnabled());
    EXPECT_TRUE(edit->lineEdit()->isReadOnly());
}

// PMS:310173 a failed key verification must re-enable the file chooser and both
// buttons so the user can retry or cancel instead of getting stuck in the dialog.
TEST_F(RetrievePasswordViewTest, BUG310173_KeyVerificationFailureKeepsDialogCancellable)
{
    stub.set_lamda(&OperatorCenter::verificationRetrievePassword,
                   [](OperatorCenter *, const QString, QString &) -> bool { return false; });

    QFuture<RetrievePasswordView::KeyVerificationResult> future = QtConcurrent::run([]() -> RetrievePasswordView::KeyVerificationResult {
        RetrievePasswordView::KeyVerificationResult result;
        result.isValid = false;
        return result;
    });
    future.waitForFinished();
    view->keyVerificationWatcher->setFuture(future);

    QSignalSpy spy(view, &RetrievePasswordView::sigBtnEnabled);
    ASSERT_TRUE(spy.isValid());
    view->onKeyVerificationFinished();

    EXPECT_TRUE(view->filePathEdit->isEnabled());
    bool btnEnabled = false;
    bool cancelEnabled = false;
    for (const auto &args : spy) {
        if (args.at(0).toInt() == 1 && args.at(1).toBool())
            btnEnabled = true;
        if (args.at(0).toInt() == 0 && args.at(1).toBool())
            cancelEnabled = true;
    }
    EXPECT_TRUE(btnEnabled);
    EXPECT_TRUE(cancelEnabled);

    // leaving the dialog during/after verification must not hang or crash
    QSignalSpy jumpSpy(view, &RetrievePasswordView::signalJump);
    ASSERT_TRUE(jumpSpy.isValid());
    view->buttonClicked(0, QString());
    EXPECT_EQ(jumpSpy.count(), 1);
}
