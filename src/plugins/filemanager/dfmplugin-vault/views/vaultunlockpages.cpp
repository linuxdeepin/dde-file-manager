// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultunlockpages.h"
#include "unlockview/retrievepasswordview.h"
#include "unlockview/retrievepasswordview.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaultdefine.h"
#include "utils/vaulthelper.h"
#include "utils/vaultautolock.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>

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
#include <QWindow>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultUnlockPages::VaultUnlockPages(QWidget *parent)
    : VaultPageBase(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    if (WindowUtils::isWayLand()) {
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }
    setIcon(QIcon::fromTheme("dfm_vault"));
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
            widget->deleteLater();
        }
        setTitle(recoveryKeyView->titleText());
        addContent(recoveryKeyView);
        clearButtons();
        const QStringList &btnList = recoveryKeyView->btnText();
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
            widget->deleteLater();
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
        if (retrievePasswordView)
            passwordRecoveryView->setResultsPage(retrievePasswordView->ValidationResults());
        if (!getContents().isEmpty()) {
            QWidget *widget = getContent(0);
            widget->hide();
            clearContents(false);
            widget->deleteLater();
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
