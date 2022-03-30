/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vaultunlockpages.h"
#include "unlockview/retrievepasswordview.h"
#include "unlockview/retrievepasswordview.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaultglobaldefine.h"
#include "utils/vaulthelper.h"
#include "utils/vaultautolock.h"
#include "services/filemanager/fileencrypt/fileencryptservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/settings.h"

#include <DPushButton>
#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DToolTip>
#include <DLabel>
#include <DFontSizeManager>

#include <QDir>
#include <QStandardPaths>
#include <QShowEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QToolTip>
#include <QEvent>
#include <QTimer>
#include <QProcess>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
VaultUnlockPages::VaultUnlockPages(QWidget *parent)
    : VaultPageBase(parent)
{

    setIcon(QIcon::fromTheme("dfm_vault"));
    setMinimumSize(396, 244);
    connect(this, &VaultUnlockPages::buttonClicked, this, &VaultUnlockPages::onButtonClicked);
    setOnButtonClickedClose(false);
}

VaultUnlockPages::~VaultUnlockPages()
{
}

void VaultUnlockPages::pageSelect(PageType page)
{
    switch (page) {
    case kUnlockPage: {
        unlockView = new UnlockView(this);
        if (!getContents().isEmpty())
            clearContents(true);
        setTitle(unlockView->titleText());
        addContent(unlockView);
        clearButtons();
        QStringList btnList = unlockView->btnText();
        addButton(btnList[0], false);
        addButton(btnList[1], true, ButtonType::ButtonRecommend);
        getButton(1)->setEnabled(false);
        connect(unlockView, &UnlockView::signalJump, this, &VaultUnlockPages::pageSelect);
        connect(unlockView, &UnlockView::sigCloseDialog, this, &VaultUnlockPages::close);
        connect(unlockView, &UnlockView::sigBtnEnabled, this, &VaultUnlockPages::onSetBtnEnabled);
    } break;
    case kRecoverPage: {
        recoveryKeyView = new RecoveryKeyView(this);
        if (!getContents().isEmpty()) {
            QWidget *widget = getContent(0);
            widget->hide();
            clearContents(false);
        }
        setTitle(recoveryKeyView->titleText());
        addContent(recoveryKeyView);
        clearButtons();
        QStringList btnList = recoveryKeyView->btnText();
        addButton(btnList[0], false);
        addButton(btnList[1], true, ButtonType::ButtonRecommend);
        connect(recoveryKeyView, &RecoveryKeyView::sigCloseDialog, this, &VaultUnlockPages::close);
        connect(recoveryKeyView, &RecoveryKeyView::sigBtnEnabled, this, &VaultUnlockPages::onSetBtnEnabled);
    } break;
    case kRetrievePage: {
        retrievePasswordView = new RetrievePasswordView(this);
        if (!getContents().isEmpty()) {
            QWidget *widget = getContent(0);
            widget->hide();
            clearContents(false);
        }
        setTitle(retrievePasswordView->titleText());
        addContent(retrievePasswordView);
        clearButtons();
        QStringList btnList = retrievePasswordView->btnText();
        addButton(btnList[0], false);
        addButton(btnList[1], true, ButtonType::ButtonRecommend);
        connect(retrievePasswordView, &RetrievePasswordView::signalJump, this, &VaultUnlockPages::pageSelect);
        connect(retrievePasswordView, &RetrievePasswordView::sigBtnEnabled, this, &VaultUnlockPages::onSetBtnEnabled);
    } break;
    case kPasswordRecoverPage: {
        passwordRecoveryView = new PasswordRecoveryView(this);
        passwordRecoveryView->setResultsPage(retrievePasswordView->ValidationResults());
        if (!getContents().isEmpty()) {
            QWidget *widget = getContent(0);
            widget->hide();
            clearContents(false);
        }
        setTitle(passwordRecoveryView->titleText());
        addContent(passwordRecoveryView);
        clearButtons();
        QStringList btnList = passwordRecoveryView->btnText();
        addButton(btnList[0], false);
        addButton(btnList[1], true, ButtonType::ButtonRecommend);
        connect(passwordRecoveryView, &PasswordRecoveryView::signalJump, this, &VaultUnlockPages::pageSelect);
        connect(passwordRecoveryView, &PasswordRecoveryView::sigCloseDialog, this, &VaultUnlockPages::close);
        connect(passwordRecoveryView, &PasswordRecoveryView::sigBtnEnabled, this, &VaultUnlockPages::onSetBtnEnabled);
    } break;
    }
}

void VaultUnlockPages::setBtnText(int index)
{
    if (getContent(0) == unlockView) {

    } else if (stackedWidget->widget(index) == retrievePasswordView) {
    }
}

void VaultUnlockPages::onButtonClicked(int index, const QString &text)
{
    if (getContent(0) == unlockView) {
        unlockView->buttonClicked(index, text);
    } else if (getContent(0) == retrievePasswordView) {
        retrievePasswordView->buttonClicked(index, text);
    } else if (getContent(0) == recoveryKeyView) {
        recoveryKeyView->buttonClicked(index, text);
    } else if (getContent(0) == passwordRecoveryView) {
        passwordRecoveryView->buttonClicked(index, text);
    }
}

void VaultUnlockPages::onSetBtnEnabled(int index, const bool &state)
{
    getButton(index)->setEnabled(state);
}
