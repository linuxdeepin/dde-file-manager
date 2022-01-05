/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "mountsecretdiskaskpassworddialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <DPasswordEdit>

MountSecretDiskAskPasswordDialog::MountSecretDiskAskPasswordDialog(const QString &tipMessage, QWidget *parent)
    : DDialog(parent),
      descriptionMessage(tipMessage)
{
    setModal(true);
    initUI();
    initConnect();
}

MountSecretDiskAskPasswordDialog::~MountSecretDiskAskPasswordDialog()
{
}

void MountSecretDiskAskPasswordDialog::initUI()
{
    QStringList buttonTexts;
    buttonTexts << tr("Cancel", "button") << tr("Unlock", "button");

    QFrame *content = new QFrame;

    titleLabel = new QLabel(tr("Input password to decrypt the disk"));
    QFont titlefont;
    titlefont.setPointSize(10);
    titleLabel->setFont(titlefont);
    descriptionLabel = new QLabel(descriptionMessage);
    QFont tipfont;
    tipfont.setPointSize(8);
    descriptionLabel->setFont(tipfont);

    passwordLineEdit = new DPasswordEdit;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(passwordLineEdit);
    mainLayout->addSpacing(10);

    content->setLayout(mainLayout);

    addContent(content);
    addButtons(buttonTexts);
    setSpacing(10);
    setDefaultButton(1);
    setIcon(QIcon::fromTheme("dialog-warning"));
}

void MountSecretDiskAskPasswordDialog::initConnect()
{
    connect(this, SIGNAL(buttonClicked(int, QString)), this, SLOT(handleButtonClicked(int, QString)));
}

void MountSecretDiskAskPasswordDialog::handleButtonClicked(int index, QString text)
{
    Q_UNUSED(text)
    if (index == 1) {
        password = passwordLineEdit->text();
    }
    accept();
}

void MountSecretDiskAskPasswordDialog::showEvent(QShowEvent *event)
{
    passwordLineEdit->setFocus();
    DDialog::showEvent(event);
}

QString MountSecretDiskAskPasswordDialog::getUerInputedPassword() const
{
    return password;
}
