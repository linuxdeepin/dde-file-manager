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
    switch (index) {
    case 0:
        emit signalJump(PageType::kUnlockPage);
        break;
    case 1:
        emit sigCloseDialog();
        break;
    }
}
