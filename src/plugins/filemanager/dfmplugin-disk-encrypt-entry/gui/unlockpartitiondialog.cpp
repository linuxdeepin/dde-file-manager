// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unlockpartitiondialog.h"
#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QAbstractButton>
#include <DPasswordEdit>
#include <dcommandlinkbutton.h>

using namespace dfmplugin_diskenc;
DWIDGET_USE_NAMESPACE

UnlockPartitionDialog::UnlockPartitionDialog(UnlockType type, QWidget *parent)
    : DDialog(parent),
      currType(type),
      initType(type)
{
    setModal(true);
    initUI();
    initConnect();

    if (dialog_utils::isWayland()) {
        fmDebug() << "Running on Wayland, setting window stay on top flag";
        setWindowFlag(Qt::WindowStaysOnTopHint);
    }
}

UnlockPartitionDialog::~UnlockPartitionDialog()
{
}

void UnlockPartitionDialog::initUI()
{
    setIcon(QIcon::fromTheme("drive-harddisk-root"));

    QFrame *content = new QFrame;
    passwordLineEdit = new DPasswordEdit;
    chgUnlockType = new DCommandLinkButton("");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addSpacing(10);
    mainLayout->addWidget(passwordLineEdit);
    mainLayout->addWidget(chgUnlockType, 0, Qt::AlignRight);
    mainLayout->addSpacing(10);
    content->setLayout(mainLayout);
    addContent(content);

    addButton(tr("Cancel"));
    addButton(tr("Unlock"), true, DDialog::ButtonRecommend);
    auto unlockBtn = getButton(1);
    if (unlockBtn) unlockBtn->setEnabled(false);

    updateUserHint();
    setOnButtonClickedClose(false);

    if (initType == kRec)
        chgUnlockType->setVisible(false);
}

void UnlockPartitionDialog::initConnect()
{
    connect(this, &DDialog::buttonClicked, this, &UnlockPartitionDialog::handleButtonClicked);
    connect(chgUnlockType, &DCommandLinkButton::clicked, this, &UnlockPartitionDialog::switchUnlockType);
    connect(passwordLineEdit, &DPasswordEdit::textChanged, this, [this](const QString &txt) {
        QString newText = txt;
        QSignalBlocker blocker(sender());
        if (currType == kRec) {
            newText = recovery_key_utils::formatRecoveryKey(newText);
            passwordLineEdit->setText(newText);
        }
        auto unlockBtn = getButton(1);
        if (unlockBtn) unlockBtn->setEnabled(newText.length() != 0);
    });
}

void UnlockPartitionDialog::updateUserHint()
{
    setTitle(tr("Unlock encryption partition"));
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setEchoButtonIsVisible(true);

    chgUnlockType->setText(tr("Unlock by recovery key"));
    switch (currType) {
    case kRec: {
        fmInfo() << "Setting up recovery key unlock mode";
        setTitle(tr("Unlock by recovery key"));
        QString text = (initType == kPwd)
                ? tr("Unlock by passphrase")
                : tr("Unlock by PIN");
        chgUnlockType->setText(text);
        passwordLineEdit->setPlaceholderText(tr("Please enter the 24-digit recovery key"));
        passwordLineEdit->setEchoMode(QLineEdit::Normal);
        passwordLineEdit->setEchoButtonIsVisible(false);
        fmDebug() << "Recovery key mode configured, switch text:" << text;
        break;
    }
    case kPwd:
        fmInfo() << "Setting up passphrase unlock mode";
        passwordLineEdit->setPlaceholderText(tr("Please input passphrase to unlock partition"));
        break;
    case kPin:
        fmInfo() << "Setting up PIN unlock mode";
        passwordLineEdit->setPlaceholderText(tr("Please input PIN to unlock partition"));
        break;
    }
}

void UnlockPartitionDialog::handleButtonClicked(int index, QString text)
{
    Q_UNUSED(text)
    if (index == 1) {
        key = passwordLineEdit->text();
        if (currType == kRec) {
            key.remove("-");
            if (key.length() != 24) {
                fmWarning() << "Recovery key validation failed, invalid length:" << key.length();
                passwordLineEdit->showAlertMessage(tr("Recovery key is not valid!"));
                return;
            }
        }
        accept();
        return;
    }
    reject();
}

void UnlockPartitionDialog::switchUnlockType()
{
    if (currType == kRec) {
        currType = initType;
        fmInfo() << "Switched from recovery key to initial type:" << initType;
    } else if (currType == kPin || currType == kPwd) {
        currType = kRec;
        fmInfo() << "Switched to recovery key mode from type:" << (currType == kPin ? "PIN" : "passphrase");
    }

    passwordLineEdit->clear();
    updateUserHint();
}

void UnlockPartitionDialog::showEvent(QShowEvent *event)
{
    passwordLineEdit->setFocus();
    DDialog::showEvent(event);
}

QPair<UnlockPartitionDialog::UnlockType, QString> UnlockPartitionDialog::getUnlockKey() const
{
    return { currType, key };
}
