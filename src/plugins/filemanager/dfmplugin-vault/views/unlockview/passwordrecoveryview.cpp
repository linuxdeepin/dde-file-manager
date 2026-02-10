// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "passwordrecoveryview.h"

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

PasswordRecoveryView::PasswordRecoveryView(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

PasswordRecoveryView::~PasswordRecoveryView()
{
}

void PasswordRecoveryView::initUI()
{
    passwordMsg = new DLabel(this);
    passwordMsg->setAlignment(Qt::AlignCenter);

    hintMsg = new DLabel(this);
    hintMsg->setAlignment(Qt::AlignCenter);
    hintMsg->setText(tr("Keep it safe"));

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    vlayout->addWidget(passwordMsg);
    vlayout->addWidget(hintMsg);
    vlayout->addStretch(1);

    this->setLayout(vlayout);
}

QStringList PasswordRecoveryView::btnText()
{
    return { tr("Go to Unlock", "button"), tr("Close", "button") };
}

QString PasswordRecoveryView::titleText()
{
    return QString(tr("Verification Successful"));
}

void PasswordRecoveryView::setResultsPage(QString password)
{
    passwordMsg->setText(tr("Vault password: %1").arg(password));
}

void PasswordRecoveryView::buttonClicked(int index, const QString &text)
{
    fmDebug() << "Vault: Password recovery view button clicked - index:" << index << "text:" << text;
    switch (index) {
    case 0:
        fmDebug() << "Vault: Go to Unlock button clicked, jumping to unlock page";
        emit signalJump(PageType::kUnlockPage);
        break;
    case 1:
        fmDebug() << "Vault: Close button clicked, closing dialog";
        emit sigCloseDialog();
        break;
    }
}
