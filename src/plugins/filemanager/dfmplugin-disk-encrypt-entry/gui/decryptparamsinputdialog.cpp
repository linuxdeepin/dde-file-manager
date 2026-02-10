// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "decryptparamsinputdialog.h"
#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <QVBoxLayout>

using namespace dfmplugin_diskenc;
DecryptParamsInputDialog::DecryptParamsInputDialog(const QString &device, QWidget *parent)
    : Dtk::Widget::DDialog(parent), devDesc(device)
{
    initUI();
    connect(recSwitch, &Dtk::Widget::DCommandLinkButton::clicked,
            this, &DecryptParamsInputDialog::onRecSwitchClicked);
    connect(editor, &Dtk::Widget::DPasswordEdit::textChanged,
            this, &DecryptParamsInputDialog::onKeyChanged);
    connect(this, &DecryptParamsInputDialog::buttonClicked,
            this, &DecryptParamsInputDialog::onButtonClicked);
    updateUserHints();
    if (dialog_utils::isWayland()) {
        fmDebug() << "Running on Wayland, setting window stay on top flag";
        setWindowFlag(Qt::WindowStaysOnTopHint);
    }
}

QString DecryptParamsInputDialog::getKey()
{
    QString key = editor->text();
    if (usingRecKey())
        key.remove("-");
    return key;
}

void DecryptParamsInputDialog::setInputPIN(bool pin)
{
    requestPIN = pin;
    updateUserHints();
}

bool DecryptParamsInputDialog::usingRecKey()
{
    return useRecKey;
}

void DecryptParamsInputDialog::onRecSwitchClicked()
{
    useRecKey = !useRecKey;
    editor->clear();
    if (useRecKey) {
        editor->setEchoMode(QLineEdit::Normal);
        editor->setEchoButtonIsVisible(false);
        editor->setPlaceholderText(tr("Please input recovery key to decrypt partition"));
        recSwitch->setText(tr("Validate with %1").arg(requestPIN ? tr("PIN") : tr("passphrase")));
        fmDebug() << "Switched to recovery key mode";
    } else {
        editor->setEchoMode(QLineEdit::Password);
        editor->setEchoButtonIsVisible(true);
        editor->setPlaceholderText(tr("Please input %1 to decrypt partition").arg(requestPIN ? tr("PIN") : tr("passphrase")));
        recSwitch->setText(tr("Validate with recovery key"));
        fmDebug() << "Switched to" << (requestPIN ? "PIN" : "passphrase") << "mode";
    }
    editor->setFocus();
}

void DecryptParamsInputDialog::onKeyChanged(const QString &key)
{
    if (!usingRecKey())
        return;
    QSignalBlocker blocker(sender());
    auto formatted = recovery_key_utils::formatRecoveryKey(key);
    editor->setText(formatted);
}

void DecryptParamsInputDialog::onButtonClicked(int idx)
{
    if (idx != 0) {
        fmInfo() << "Non-confirm button clicked, rejecting dialog";
        reject();
        return;
    }

    if (getKey().isEmpty()) {
        QString keyType = requestPIN ? tr("PIN") : tr("Passphrase");
        if (usingRecKey())
            keyType = tr("Recovery key");
        editor->showAlertMessage(tr("%1 cannot be empty!").arg(keyType));
        fmWarning() << "Validation failed: empty" << keyType << "field";
        return;
    }

    else if (usingRecKey() && getKey().length() != 24) {
        editor->showAlertMessage(tr("Recovery key is not valid!"));
        fmWarning() << "Validation failed: invalid recovery key length:" << getKey().length() << "(expected 24)";
        return;
    }

    accept();
}

void DecryptParamsInputDialog::updateUserHints()
{
    useRecKey = true;
    onRecSwitchClicked();
}

void DecryptParamsInputDialog::initUI()
{
    setIcon(QIcon::fromTheme("drive-harddisk-root"));
    setTitle(tr("Decrypt partition"));
    QFrame *content = new QFrame(this);
    QVBoxLayout *lay = new QVBoxLayout(content);
    editor = new Dtk::Widget::DPasswordEdit(this);
    lay->addWidget(editor);
    recSwitch = new Dtk::Widget::DCommandLinkButton("", this);
    lay->addWidget(recSwitch, 0, Qt::AlignRight);
    addContent(content);
    addButton(tr("Confirm"));
    setOnButtonClickedClose(false);

    editor->setFocus();
}
