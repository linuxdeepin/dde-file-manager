// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QPlainTextEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QShowEvent>

#include "stubext.h"

#define private public
#include "views/unlockview/recoverykeyview.h"
#undef private

#include "events/vaulteventcaller.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/encryption/interfaceactivevault.h"

DPVAULT_USE_NAMESPACE

class RecoveryKeyViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stubCommonDeps();
        view = new RecoveryKeyView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    void stubCommonDeps()
    {
        stub.set_lamda(&VaultEventCaller::sendItemActived, [](quint64, const QUrl &) {});
        stub.set_lamda(&OperatorCenter::checkPassword, [](OperatorCenter *, const QString &, QString &) -> bool { return true; });
        stub.set_lamda(&OperatorCenter::isNewVaultVersion, []() -> bool { return false; });
        stub.set_lamda(&PathManager::createVaultMountDir, [](const QString &) -> bool { return true; });
        stub.set_lamda(&VaultHelper::lockVault, [](VaultHelper *, bool) -> bool { return true; });
        stub.set_lamda(&InterfaceActiveVault::checkUserKey, [](const QString &, QString &) -> bool { return true; });
    }

    RecoveryKeyView *view = nullptr;
};

TEST_F(RecoveryKeyViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(RecoveryKeyViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(RecoveryKeyViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(RecoveryKeyViewTest, ShowAlertMessage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("test alert", 100));
}

TEST_F(RecoveryKeyViewTest, ShowAlertMessage_Persistent_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->showAlertMessage("persistent", -1));
}

TEST_F(RecoveryKeyViewTest, ButtonClicked_IndexZero_EmitsCloseDialog)
{
    QSignalSpy spy(view, &RecoveryKeyView::sigCloseDialog);
    view->buttonClicked(0, "Cancel");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(RecoveryKeyViewTest, ButtonClicked_IndexOne_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "Unlock"));
}

TEST_F(RecoveryKeyViewTest, ButtonClicked_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(99, "Unknown"));
}

// --- recoveryKeyChanged ---

TEST_F(RecoveryKeyViewTest, RecoveryKeyChanged_Empty_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->recoveryKeyChanged());
}

TEST_F(RecoveryKeyViewTest, RecoveryKeyChanged_WithText_NoCrash)
{
    if (view->recoveryKeyEdit) {
        view->recoveryKeyEdit->setPlainText("abcdefgh");
    }
    EXPECT_NO_FATAL_FAILURE(view->recoveryKeyChanged());
}

TEST_F(RecoveryKeyViewTest, RecoveryKeyChanged_EmitsBtnEnabled)
{
    QSignalSpy spy(view, &RecoveryKeyView::sigBtnEnabled);
    view->recoveryKeyChanged();
    EXPECT_GE(spy.count(), 1);
}

// --- afterRecoveryKeyChanged ---

TEST_F(RecoveryKeyViewTest, AfterRecoveryKeyChanged_EmptyString)
{
    QString key = "";
    int pos = view->afterRecoveryKeyChanged(key);
    EXPECT_EQ(pos, -1);
}

TEST_F(RecoveryKeyViewTest, AfterRecoveryKeyChanged_ShortString)
{
    QString key = "abcd";
    int pos = view->afterRecoveryKeyChanged(key);
    EXPECT_GE(pos, 0);
}

TEST_F(RecoveryKeyViewTest, AfterRecoveryKeyChanged_LongString_AddsDashes)
{
    QString key = "abcdefghijklmnop";
    int pos = view->afterRecoveryKeyChanged(key);
    EXPECT_GT(pos, 0);
    EXPECT_TRUE(key.contains("-"));
}

// --- showEvent ---

TEST_F(RecoveryKeyViewTest, ShowEvent_NoCrash)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(view->showEvent(&event));
}
