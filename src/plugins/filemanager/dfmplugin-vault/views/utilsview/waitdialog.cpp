// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waitdialog.h"
#include "dbus/vaultdbusutils.h"
#include "events/vaulteventcaller.h"
#include "utils/vaulthelper.h"

#include <QAbstractButton>

using namespace dfmplugin_vault;
DWIDGET_USE_NAMESPACE

WaitDialog::WaitDialog(QWidget *parent)
    : DDialog(parent)
{
    initUi();
    initConnect();
}

WaitDialog::~WaitDialog()
{
    if (timer.isActive())
        timer.stop();
}

void WaitDialog::showEvent(QShowEvent *event)
{
    timer.setSingleShot(false);
    timer.start(1000);

    DDialog::showEvent(event);
}

void WaitDialog::closeEvent(QCloseEvent *event)
{
    if (timer.isActive())
        timer.stop();

    DDialog::closeEvent(event);
}

void WaitDialog::handleTimeout()
{
    int state = VaultDBusUtils::getUnlockCompleteState(kVaultBasePath);
    if (state == -1) {
        timer.stop();
        setMessage(tr("Dbus server is unavailble."));
        return;
    }

    UnlockState eState = static_cast<UnlockState>(state);
    if (eState == UnlockState::kUnlockSuccess) {
        timer.stop();
        close();
        VaultEventCaller::sendItemActived(VaultHelper::instance()->currentWindowId(), VaultHelper::instance()->rootUrl());
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
    } else if (eState == UnlockState::kTpmNotAvailable) {
        timer.stop();
        setMessage(tr("Can't unlock vault, TCM/TPM is unavailable."));
    } else if (eState == UnlockState::kUnlockFailed) {
        timer.stop();
        setMessage(tr("Unlock vault failed!"));
    }
}

void WaitDialog::initUi()
{
    moveToCenter();

    setMessage(tr("In the unlock, please later..."));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm"));
    addButtons(buttonTexts);
    setDefaultButton(0);

    VaultDBusUtils::transparentUnlockVault(kVaultBasePath);
}

void WaitDialog::initConnect()
{
    connect(&timer, &QTimer::timeout, this, &WaitDialog::handleTimeout);
}
