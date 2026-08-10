// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewhintmessage.cpp
 * @brief Unit tests for ViewHintMessage (viewhintmessage.cpp)
 *
 * ViewHintMessage is a QObject controller for floating hint messages. The
 * setter methods and isVisible() are pure data operations that don't need
 * a visible widget. The show() method is NOT called to avoid needing a real
 * host widget.
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/viewhintmessage/viewhintmessage.h>

#include <QObject>
#include <QString>
#include <QList>
#include <QPair>

using namespace dfmbase;

TEST(ViewHintMessageTest, ConstructAndDestructWithoutCrash)
{
    {
        ViewHintMessage msg;
        (void)msg;
    }
    SUCCEED();
}

TEST(ViewHintMessageTest, IsVisibleReturnsFalseBeforeShow)
{
    ViewHintMessage msg;
    EXPECT_FALSE(msg.isVisible());
}

TEST(ViewHintMessageTest, SetIconDoesNotCrash)
{
    ViewHintMessage msg;
    msg.setIcon(QStringLiteral("dialog-warning"));
}

TEST(ViewHintMessageTest, SetTextDoesNotCrash)
{
    ViewHintMessage msg;
    msg.setText(QStringLiteral("test message"));
}

TEST(ViewHintMessageTest, SetActionsDoesNotCrash)
{
    ViewHintMessage msg;
    QList<QPair<QString, QString>> actions;
    actions << QPair<QString, QString>(QStringLiteral("ok"), QStringLiteral("OK"));
    actions << QPair<QString, QString>(QStringLiteral("cancel"), QStringLiteral("Cancel"));
    msg.setActions(actions);
}

TEST(ViewHintMessageTest, SetAutoDismissOnActionDoesNotCrash)
{
    ViewHintMessage msg;
    msg.setAutoDismissOnAction(false);
    msg.setAutoDismissOnAction(true);
}
