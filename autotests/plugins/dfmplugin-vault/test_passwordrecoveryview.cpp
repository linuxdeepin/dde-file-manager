// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>

#include "stubext.h"

#include "views/unlockview/passwordrecoveryview.h"

DPVAULT_USE_NAMESPACE

class PasswordRecoveryViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new PasswordRecoveryView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    PasswordRecoveryView *view = nullptr;
};

TEST_F(PasswordRecoveryViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(PasswordRecoveryViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(PasswordRecoveryViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(PasswordRecoveryViewTest, SetResultsPage_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setResultsPage("mypassword"));
}

TEST_F(PasswordRecoveryViewTest, ButtonClicked_IndexZero_EmitsSignalJump)
{
    QSignalSpy spy(view, &PasswordRecoveryView::signalJump);
    view->buttonClicked(0, "Go to Unlock");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PasswordRecoveryViewTest, ButtonClicked_IndexOne_EmitsCloseDialog)
{
    QSignalSpy spy(view, &PasswordRecoveryView::sigCloseDialog);
    view->buttonClicked(1, "Close");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PasswordRecoveryViewTest, ButtonClicked_InvalidIndex_NoSignal)
{
    QSignalSpy spyJump(view, &PasswordRecoveryView::signalJump);
    QSignalSpy spyClose(view, &PasswordRecoveryView::sigCloseDialog);
    view->buttonClicked(99, "Unknown");
    EXPECT_EQ(spyJump.count(), 0);
    EXPECT_EQ(spyClose.count(), 0);
}
