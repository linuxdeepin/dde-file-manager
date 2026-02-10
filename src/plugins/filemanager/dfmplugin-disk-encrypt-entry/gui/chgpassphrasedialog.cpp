// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "chgpassphrasedialog.h"
#include "utils/encryptutils.h"

#include <QFormLayout>
#include <QLabel>
#include <QRegularExpression>

using namespace dfmplugin_diskenc;

ChgPassphraseDialog::ChgPassphraseDialog(const QString &device, QWidget *parent)
    : Dtk::Widget::DDialog(parent),
      device(device)
{
    int keyType = device_utils::encKeyType(device);
    encType = tr("passphrase");
    if (keyType == 1)   // PIN
        encType = tr("PIN");

    initUI();

    connect(this, &ChgPassphraseDialog::buttonClicked,
            this, &ChgPassphraseDialog::onButtonClicked);
    connect(recSwitch, &Dtk::Widget::DCommandLinkButton::clicked,
            this, &ChgPassphraseDialog::onRecSwitchClicked);
    connect(oldPass, &Dtk::Widget::DPasswordEdit::textChanged,
            this, &ChgPassphraseDialog::onOldKeyChanged);
    if (dialog_utils::isWayland()) {
        fmDebug() << "Running on Wayland, setting window stay on top flag";
        setWindowFlag(Qt::WindowStaysOnTopHint);
    }
}

QPair<QString, QString> ChgPassphraseDialog::getPassphrase()
{
    QString oldKey = oldPass->text();
    if (usingRecKey)
        oldKey.remove("-");
    return { oldKey, newPass1->text() };
}

bool ChgPassphraseDialog::validateByRecKey()
{
    return usingRecKey;
}

void ChgPassphraseDialog::initUI()
{
    setIcon(QIcon::fromTheme("drive-harddisk-root"));

    int keyType = device_utils::encKeyType(device);
    QString keyTypeStr = tr("passphrase");
    if (keyType == 1)   // PIN
        keyTypeStr = tr("PIN");

    setTitle(tr("Modify %1").arg(keyTypeStr));
    QFrame *content = new QFrame(this);
    QVBoxLayout *contentLay = new QVBoxLayout(content);
    QFormLayout *lay = new QFormLayout();
    contentLay->addLayout(lay);

    oldKeyHint = new QLabel(this);
    oldPass = new Dtk::Widget::DPasswordEdit(this);
    newPass1 = new Dtk::Widget::DPasswordEdit(this);
    newPass2 = new Dtk::Widget::DPasswordEdit(this);

    newPass2->setPlaceholderText(tr("Please enter %1 again").arg(keyTypeStr));

    lay->addRow(oldKeyHint, oldPass);
    lay->addRow(tr("New %1").arg(encType), newPass1);
    lay->addRow(tr("Repeat %1").arg(encType), newPass2);

    recSwitch = new Dtk::Widget::DCommandLinkButton("", this);
    contentLay->addWidget(recSwitch, 0, Qt::AlignRight);

    addContent(content);
    addButton(tr("Cancel"));
    addButton(tr("Confirm"));

    setOnButtonClickedClose(false);

    usingRecKey = true;
    onRecSwitchClicked();

    setMinimumWidth(500);

    oldPass->setFocus();
}

bool ChgPassphraseDialog::validatePasswd()
{
    int keyType = device_utils::encKeyType(device);
    QString keyTypeStr = tr("passphrase");
    if (keyType == 1)   // PIN
        keyTypeStr = tr("PIN");

    auto nonEmpty = [=](Dtk::Widget::DPasswordEdit *editor) {
        QString pwd = editor->text().trimmed();
        if (!pwd.isEmpty()) return true;

        editor->showAlertMessage(tr("%1 cannot be empty").arg(keyTypeStr));
        fmWarning() << "Validation failed: empty" << keyTypeStr << "field";
        return false;
    };

    QString oldKey = oldPass->text().trimmed();
    if (oldKey.isEmpty()) {
        QString msg = tr("%1 cannot be empty");
        if (usingRecKey)
            msg = msg.arg(tr("Recovery key"));
        else
            msg = msg.arg(encType);
        oldPass->showAlertMessage(msg);
        fmWarning() << "Validation failed: empty old key field, using recovery key:" << usingRecKey;
        return false;
    } else if (usingRecKey && oldKey.remove("-").length() != 24) {
        oldPass->showAlertMessage(tr("Recovery key is not valid!"));
        fmWarning() << "Validation failed: invalid recovery key length:" << oldKey.length() << "(expected 24)";
        return false;
    }

    if (!(nonEmpty(newPass1)
          && nonEmpty(newPass2))) {
        fmWarning() << "Validation failed: empty new password fields";
        return false;
    }

    QList<QRegularExpression> regx {
        QRegularExpression { R"([A-Z])" },
        QRegularExpression { R"([a-z])" },
        QRegularExpression { R"([0-9])" },
        QRegularExpression { R"([^A-Za-z0-9])" }
    };

    QString pwd1 = newPass1->text().trimmed();
    QString pwd2 = newPass2->text().trimmed();

    int factor = 0;
    std::for_each(regx.cbegin(), regx.cend(), [&factor, pwd1](const QRegularExpression &reg) {
        if (pwd1.contains(reg))
            factor += 1;
    });

    if (factor < 3 || pwd1.length() < 8) {
        newPass1->showAlertMessage(tr("At least 8 bits, contains 3 types of A-Z, a-z, 0-9 and symbols"));
        fmWarning() << "Validation failed: password complexity insufficient - length:" << pwd1.length() << "factor:" << factor;
        return false;
    }

    if (pwd1 != pwd2) {
        newPass2->showAlertMessage(tr("%1 inconsistency").arg(keyTypeStr));
        fmWarning() << "Validation failed: password mismatch for" << keyTypeStr;
        return false;
    }

    return true;
}

void ChgPassphraseDialog::onButtonClicked(int idx)
{
    if (idx == 1) {
        if (!validatePasswd())
            return;
        accept();
    } else {
        reject();
    }
}

void ChgPassphraseDialog::onRecSwitchClicked()
{
    usingRecKey = !usingRecKey;
    oldPass->clear();
    if (usingRecKey) {
        oldKeyHint->setText(tr("Recovery key"));
        oldPass->setEchoMode(QLineEdit::Normal);
        oldPass->setEchoButtonIsVisible(false);
        recSwitch->setText(tr("Validate with %1").arg(encType));
        oldPass->setPlaceholderText(tr("Please input recovery key"));
        fmDebug() << "Switched to recovery key mode";
    } else {
        oldKeyHint->setText(tr("Old %1").arg(encType));
        oldPass->setEchoMode(QLineEdit::Password);
        oldPass->setEchoButtonIsVisible(true);
        recSwitch->setText(tr("Validate with recovery key"));
        oldPass->setPlaceholderText(tr("At least 8 bits, contains 3 types of A-Z, a-z, 0-9 and symbols"));
        fmDebug() << "Switched to" << encType << "mode";
    }
    newPass1->setPlaceholderText(tr("At least 8 bits, contains 3 types of A-Z, a-z, 0-9 and symbols"));
    oldPass->setFocus();
}

void ChgPassphraseDialog::onOldKeyChanged(const QString &inputs)
{
    if (usingRecKey) {
        QSignalBlocker blocker(sender());
        oldPass->setText(recovery_key_utils::formatRecoveryKey(inputs));
    }
}
