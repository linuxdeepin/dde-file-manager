// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/removevaultview/vaultremovebypasswordview.h"
#include <gtest/gtest.h>

#include <DPasswordEdit>
#include <DToolTip>
#include <DFloatingWidget>
#include <QPushButton>

#include <QTimer>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(UT_VaultRemoveByPasswordView, getPassword)
{
    VaultRemoveByPasswordView view;
    view.pwdEdit->setText("Qwer@1234");
    EXPECT_TRUE(view.getPassword() == "Qwer@1234");
}

TEST(UT_VaultRemoveByPasswordView, clear)
{
    VaultRemoveByPasswordView view;
    view.pwdEdit->setText("Qwer@1234");
    view.clear();
    EXPECT_TRUE(view.pwdEdit->text().isEmpty());
}

TEST(UT_VaultRemoveByPasswordView, showAlertMessage)
{
    stub_ext::StubExt stub;
    typedef void(DPasswordEdit::*FuncType)(const QString &, int);
    stub.set_lamda(static_cast<FuncType>(&DPasswordEdit::showAlertMessage), [] { __DBG_STUB_INVOKE__ });

    VaultRemoveByPasswordView view;
    view.showAlertMessage("warning");
    EXPECT_TRUE(view.pwdEdit->isAlert());
}

TEST(UT_VaultRemoveByPasswordView, showToolTip_Warning)
{
    stub_ext::StubExt stub;
    typedef  void(*FuncType)(int, Qt::TimerType,
                                     const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(QTimer::singleShotImpl), [] { __DBG_STUB_INVOKE__ });

    QWidget widget1, widget2, widget3;
    widget2.setParent(&widget1);
    widget3.setParent(&widget2);
    VaultRemoveByPasswordView view;
    view.setParent(&widget3);
    view.showToolTip("tooltips", -1, VaultRemoveByPasswordView::kWarning);
    EXPECT_TRUE(view.tooltip->text() == "tooltips");
}

TEST(UT_VaultRemoveByPasswordView, showToolTip_Information)
{
    stub_ext::StubExt stub;
    typedef void(*FuncType)(int, Qt::TimerType, const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(QTimer::singleShotImpl), [] { __DBG_STUB_INVOKE__ });

    QWidget widget1, widget2, widget3;
    widget2.setParent(&widget1);
    widget3.setParent(&widget2);
    VaultRemoveByPasswordView view;
    view.setParent(&widget3);
    view.showToolTip("tooltips", 10, VaultRemoveByPasswordView::kInformation);
    EXPECT_TRUE(view.tooltip->text() == "tooltips");
}

TEST(UT_VaultRemoveByPasswordView, setTipsButtonVisible)
{
    VaultRemoveByPasswordView view;
    view.setTipsButtonVisible(false);
    EXPECT_FALSE(view.tipsBtn->isVisible());
}

TEST(UT_VaultRemoveByPasswordView, onPasswordChanged)
{
    VaultRemoveByPasswordView view;
    view.onPasswordChanged("123");
    EXPECT_FALSE(view.pwdEdit->isAlert());
}
