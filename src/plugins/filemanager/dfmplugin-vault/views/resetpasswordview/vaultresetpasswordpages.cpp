// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultresetpasswordpages.h"
#include "utils/vaulthelper.h"

#include <QAbstractButton>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultResetPasswordPages::VaultResetPasswordPages(QWidget *parent)
    : VaultPageBase(parent)
    , isOldPasswordView(true)
{
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    setIcon(QIcon::fromTheme("dfm_vault"));
    setSpacing(10);
    setFixedWidth(496);
    connect(this, &VaultResetPasswordPages::buttonClicked, this, &VaultResetPasswordPages::onButtonClicked);
    setOnButtonClickedClose(false);
}

VaultResetPasswordPages::~VaultResetPasswordPages()
{
}

void VaultResetPasswordPages::switchToOldPasswordView()
{
    isOldPasswordView = true;

    if (!oldPasswordView) {
        oldPasswordView = new ResetPasswordByOldPasswordView(this);
        connect(oldPasswordView, &ResetPasswordByOldPasswordView::signalJump, this, &VaultResetPasswordPages::switchToKeyFileView);
        connect(oldPasswordView, &ResetPasswordByOldPasswordView::sigCloseDialog, this, &VaultResetPasswordPages::close);
    }

    if (!getContents().isEmpty() && getContent(0) == oldPasswordView) {
        return;
    }

    if (!getContents().isEmpty()) {
        QWidget *widget = getContent(0);
        if (widget) {
            widget->hide();
        }
        clearContents(false);
    }
    clearButtons();

    disconnect(oldPasswordView, &ResetPasswordByOldPasswordView::sigBtnEnabled, this, nullptr);
    if (keyFileView) {
        disconnect(keyFileView, &ResetPasswordByKeyFileView::sigBtnEnabled, this, nullptr);
    }

    setTitle(oldPasswordView->titleText());
    addContent(oldPasswordView);
    oldPasswordView->show();
    QStringList btnList = oldPasswordView->btnText();
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    connect(oldPasswordView, &ResetPasswordByOldPasswordView::sigBtnEnabled, this, &VaultResetPasswordPages::onSetBtnEnabled);
}

void VaultResetPasswordPages::switchToKeyFileView()
{
    isOldPasswordView = false;

    if (!keyFileView) {
        keyFileView = new ResetPasswordByKeyFileView(this);
        connect(keyFileView, &ResetPasswordByKeyFileView::signalJump, this, &VaultResetPasswordPages::switchToOldPasswordView);
        connect(keyFileView, &ResetPasswordByKeyFileView::sigCloseDialog, this, &VaultResetPasswordPages::close);
    }

    if (!getContents().isEmpty() && getContent(0) == keyFileView) {
        return;
    }

    if (!getContents().isEmpty()) {
        QWidget *widget = getContent(0);
        if (widget) {
            widget->hide();
        }
        clearContents(false);
    }
    clearButtons();

    if (oldPasswordView) {
        disconnect(oldPasswordView, &ResetPasswordByOldPasswordView::sigBtnEnabled, this, nullptr);
    }
    disconnect(keyFileView, &ResetPasswordByKeyFileView::sigBtnEnabled, this, nullptr);

    setTitle(keyFileView->titleText());
    addContent(keyFileView);
    keyFileView->show();
    QStringList btnList = keyFileView->btnText();
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    connect(keyFileView, &ResetPasswordByKeyFileView::sigBtnEnabled, this, &VaultResetPasswordPages::onSetBtnEnabled);
}

void VaultResetPasswordPages::onButtonClicked(int index, const QString &text)
{
    if (isOldPasswordView && oldPasswordView) {
        oldPasswordView->buttonClicked(index, text);
    } else if (!isOldPasswordView && keyFileView) {
        keyFileView->buttonClicked(index, text);
    }
}

void VaultResetPasswordPages::onSetBtnEnabled(int index, const bool &state)
{
    QAbstractButton *btn = getButton(index);
    if (btn) {
        btn->setEnabled(state);
    }
}

