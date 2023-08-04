// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivesetunlockmethodview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/policy/policymanager.h"
#include "utils/encryption/vaultconfig.h"

#include <dfm-framework/event/event.h>

#include <DPasswordEdit>
#include <DLabel>
#include <DComboBox>

#include <QDebug>
#include <QToolTip>
#include <QRegExp>
#include <QRegExpValidator>
#include <QGridLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveSetUnlockMethodView::VaultActiveSetUnlockMethodView(QWidget *parent)
    : QWidget(parent)
{
    initUi();
    initConnect();

    if (!OperatorCenter::getInstance()->createDirAndFile())
        qCritical() << "Vault: create dir and file failed!";
}

void VaultActiveSetUnlockMethodView::initUi()
{
    DLabel *pLabel = new DLabel(tr("Set Vault Password"), this);
    QFont font = pLabel->font();
    font.setPixelSize(18);
    pLabel->setFont(font);
    pLabel->setAlignment(Qt::AlignHCenter);

    DLabel *pTypeLabel = new DLabel(tr("Encryption method"), this);
    typeCombo = new DComboBox(this);
    QStringList lstItems;
    lstItems << tr("Key encryption")  << tr("Transparent encryption");
    typeCombo->addItems(lstItems);

    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~\'\"|]+");
    QValidator *validator = new QRegExpValidator(regx, this);

    passwordLabel = new DLabel(tr("Password"), this);
    passwordEdit = new DPasswordEdit(this);
    passwordEdit->lineEdit()->setValidator(validator);
    passwordEdit->lineEdit()->setPlaceholderText(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"));
    passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    repeatPasswordLabel = new DLabel(tr("Repeat password"), this);
    repeatPasswordEdit = new DPasswordEdit(this);
    repeatPasswordEdit->lineEdit()->setValidator(validator);
    repeatPasswordEdit->lineEdit()->setPlaceholderText(tr("Input the password again"));
    repeatPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    passwordHintLabel = new DLabel(tr("Password hint"), this);
    tipsEdit = new DLineEdit(this);
    tipsEdit->lineEdit()->setMaxLength(14);
    tipsEdit->setPlaceholderText(tr("Optional"));

    // transparent encryption text
    transEncryptTextLay = new QVBoxLayout();
    transEncryptionText = new DLabel(tr("The file vault will be automatically unlocked when accessed, "
                                        "without verifying the password. "
                                        "Files in it will be inaccessible under other user accounts. "),
                                     this);
    transEncryptionText->setVisible(false);
    transEncryptionText->setWordWrap(true);
    transEncryptTextLay->setContentsMargins(10, 0, 0, 0);
    transEncryptTextLay->addWidget(transEncryptionText);

    nextBtn = new DPushButton(tr("Next"), this);
    nextBtn->setEnabled(false);

    gridLayout = new QGridLayout();
    gridLayout->setMargin(0);

    gridLayout->addWidget(pTypeLabel, 0, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(typeCombo, 0, 1, 1, 5);

    gridLayout->addWidget(passwordLabel, 1, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(passwordEdit, 1, 1, 1, 5);

    gridLayout->addWidget(repeatPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(repeatPasswordEdit, 2, 1, 1, 5);

    gridLayout->addWidget(passwordHintLabel, 3, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(tipsEdit, 3, 1, 1, 5);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel);
    play->addSpacing(15);
    play->addLayout(gridLayout);
    play->addStretch();
    play->addWidget(nextBtn);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(pLabel), AcName::kAcLabelVaultSetUnlockTitle);
    AddATTag(qobject_cast<QWidget *>(pTypeLabel), AcName::kAcLabelVaultSetUnlcokMethod);
    AddATTag(qobject_cast<QWidget *>(typeCombo), AcName::kAcComboVaultSetUnlockMethod);
    AddATTag(qobject_cast<QWidget *>(passwordLabel), AcName::kAcLabelVaultSetUnlockPassword);
    AddATTag(qobject_cast<QWidget *>(passwordEdit), AcName::kAcEditVaultSetUnlockPassword);
    AddATTag(qobject_cast<QWidget *>(repeatPasswordLabel), AcName::kAcLabelVaultSetUnlockRepeatPasswrod);
    AddATTag(qobject_cast<QWidget *>(repeatPasswordEdit), AcName::kAcEditVaultSetUnlockRepeatPassword);
    AddATTag(qobject_cast<QWidget *>(passwordHintLabel), AcName::kAcLabelVaultSetUnlockHint);
    AddATTag(qobject_cast<QWidget *>(tipsEdit), AcName::kAcEditVaultSetUnlockHint);
    AddATTag(qobject_cast<QWidget *>(transEncryptionText), AcName::kAcLabelVaultSetUnlockText);
    AddATTag(qobject_cast<QWidget *>(nextBtn), AcName::kAcBtnVaultSetUnlockNext);
#endif
}

void VaultActiveSetUnlockMethodView::initConnect()
{
    connect(typeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeChanged(int)));
    connect(passwordEdit, &DPasswordEdit::textEdited,
            this, &VaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(passwordEdit, &DPasswordEdit::editingFinished,
            this, &VaultActiveSetUnlockMethodView::slotPasswordEditFinished);
    connect(passwordEdit, &DPasswordEdit::textChanged,
            this, &VaultActiveSetUnlockMethodView::slotPasswordEditing);
    connect(passwordEdit, &DPasswordEdit::focusChanged,
            this, &VaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged);
    connect(repeatPasswordEdit, &DPasswordEdit::textEdited,
            this, &VaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(repeatPasswordEdit, &DPasswordEdit::editingFinished,
            this, &VaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished);
    connect(repeatPasswordEdit, &DPasswordEdit::textChanged,
            this, &VaultActiveSetUnlockMethodView::slotRepeatPasswordEditing);
    connect(repeatPasswordEdit, &DPasswordEdit::focusChanged,
            this, &VaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged);
    connect(nextBtn, &DPushButton::clicked,
            this, &VaultActiveSetUnlockMethodView::slotNextBtnClicked);
}

void VaultActiveSetUnlockMethodView::clearText()
{
    passwordEdit->clear();
    passwordEdit->hideAlertMessage();
    //! 修复BUG-51508 取消密码框警告状态
    passwordEdit->setAlert(false);
    repeatPasswordEdit->clear();
    repeatPasswordEdit->hideAlertMessage();
    //! 修复BUG-51508 取消密码框警告状态
    repeatPasswordEdit->setAlert(false);
    tipsEdit->clear();
}

void VaultActiveSetUnlockMethodView::slotPasswordEditing()
{
    bool ok = checkPassword(passwordEdit->text());
    if (ok) {
        if (checkInputInfo()) {
            nextBtn->setEnabled(true);
        }
    }
    nextBtn->setEnabled(false);
}

void VaultActiveSetUnlockMethodView::slotPasswordEditFinished()
{
    bool ok = checkPassword(passwordEdit->text());
    if (!ok) {
        nextBtn->setEnabled(false);
        //! 修复BUG-51508 激活密码框警告状态
        passwordEdit->setAlert(true);
        passwordEdit->showAlertMessage(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"), TIPS_TIME);
    } else {
        if (checkInputInfo()) {
            nextBtn->setEnabled(true);
        }
    }
}

void VaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged(bool bFocus)
{
    if (bFocus) {
        //! 修复BUG-51508 取消密码框警告状态
        passwordEdit->setAlert(false);
        passwordEdit->hideAlertMessage();
    }
}

void VaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished()
{
    bool ok = checkRepeatPassword();
    if (!ok) {
        //! 修复BUG-51508 激活密码框警告状态
        repeatPasswordEdit->setAlert(true);
        repeatPasswordEdit->showAlertMessage(tr("Passwords do not match"), TIPS_TIME);
    }
}

void VaultActiveSetUnlockMethodView::slotRepeatPasswordEditing()
{
    const QString &strRepeatPassword = repeatPasswordEdit->text();
    const QString &strPassword = passwordEdit->text();

    bool bSizeMatch = strRepeatPassword.size() == strPassword.size();
    if (bSizeMatch) {
        if (checkPassword(passwordEdit->text())) {
            if (checkRepeatPassword()) {
                nextBtn->setEnabled(true);
                return;
            } else {
                repeatPasswordEdit->showAlertMessage(tr("Passwords do not match"));
            }
        }
    }
    nextBtn->setEnabled(false);
}

void VaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged(bool bFocus)
{
    if (bFocus) {
        // 修复BUG-51508 取消密码框警告状态
        repeatPasswordEdit->setAlert(false);
        repeatPasswordEdit->hideAlertMessage();
    }
}

void VaultActiveSetUnlockMethodView::slotGenerateEditChanged(const QString &str)
{
    if (!checkPassword(str)) {
        nextBtn->setEnabled(false);
    } else {
        nextBtn->setEnabled(true);
    }
}

void VaultActiveSetUnlockMethodView::slotNextBtnClicked()
{
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant("Yes"));

    if (typeCombo->currentIndex() == 0) {   // key encryption
        QString strPassword = passwordEdit->text();
        QString strPasswordHint = tipsEdit->text();
        if (OperatorCenter::getInstance()->savePasswordAndPasswordHint(strPassword, strPasswordHint)
                && OperatorCenter::getInstance()->createKeyNew(strPassword)) {
            config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodKey));
            emit sigAccepted();
        }
    } else {   // transparent encryption
        const QString &password = OperatorCenter::getInstance()->autoGeneratePassword(kPasswordLength);
        if (password.isEmpty()) {
            qCritical() << "Vault: auto Generate password failed!";
            return;
        }

        // save password to keyring
        if (OperatorCenter::getInstance()->savePasswordToKeyring(password)) {
            config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodTransparent));
            config.set(kConfigNodeName, kConfigKeyVersion, QVariant(kConfigVaultVersion1050));
            emit sigAccepted();
        } else {
            qCritical() << "Vault: save password to keyring failed!";
        }
    }
}

void VaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    if (index) {   // transparent encrypyion
        gridLayout->removeWidget(passwordLabel);
        gridLayout->removeWidget(passwordEdit);
        gridLayout->removeWidget(repeatPasswordLabel);
        gridLayout->removeWidget(repeatPasswordEdit);
        gridLayout->removeWidget(passwordHintLabel);
        gridLayout->removeWidget(tipsEdit);

        passwordLabel->setVisible(false);
        passwordEdit->setVisible(false);
        repeatPasswordLabel->setVisible(false);
        repeatPasswordEdit->setVisible(false);
        passwordHintLabel->setVisible(false);
        tipsEdit->setVisible(false);

        gridLayout->addLayout(transEncryptTextLay, 1, 1, 3, 5);
        transEncryptionText->setVisible(true);

        nextBtn->setEnabled(true);
    } else {   // key encryption
        gridLayout->removeItem(transEncryptTextLay);
        transEncryptionText->setVisible(false);

        gridLayout->addWidget(passwordLabel, 1, 0, 1, 1, Qt::AlignLeft);
        gridLayout->addWidget(passwordEdit, 1, 1, 1, 5);
        gridLayout->addWidget(repeatPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
        gridLayout->addWidget(repeatPasswordEdit, 2, 1, 1, 5);
        gridLayout->addWidget(passwordHintLabel, 3, 0, 1, 1, Qt::AlignLeft);
        gridLayout->addWidget(tipsEdit, 3, 1, 1, 5);

        passwordLabel->setVisible(true);
        passwordEdit->setVisible(true);
        repeatPasswordLabel->setVisible(true);
        repeatPasswordEdit->setVisible(true);
        passwordHintLabel->setVisible(true);
        tipsEdit->setVisible(true);

        checkInputInfo() ? nextBtn->setEnabled(true) : nextBtn->setEnabled(false);
    }
}

void VaultActiveSetUnlockMethodView::slotLimiPasswordLength(const QString &password)
{
    DPasswordEdit *pPasswordEdit = qobject_cast<DPasswordEdit *>(sender());
    if (password.length() > PASSWORD_LENGHT_MAX) {
        pPasswordEdit->setText(password.mid(0, PASSWORD_LENGHT_MAX));
    }
}

bool VaultActiveSetUnlockMethodView::checkPassword(const QString &password)
{
    QString strPassword = password;

    QRegExp rx("^(?![^a-z]+$)(?![^A-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegExpValidator v(rx);
    int pos = 0;
    QValidator::State res;
    res = v.validate(strPassword, pos);
    if (QValidator::Acceptable != res) {
        return false;

    } else {
        return true;
    }
}

bool VaultActiveSetUnlockMethodView::checkRepeatPassword()
{
    const QString &strRepeatPassword = repeatPasswordEdit->text();
    const QString &strPassword = passwordEdit->text();
    if (strRepeatPassword != strPassword) {
        return false;
    } else {
        return true;
    }
}

bool VaultActiveSetUnlockMethodView::checkInputInfo()
{
    if (checkPassword(passwordEdit->text()) && checkRepeatPassword()) {
        return true;
    } else {
        return false;
    }
}

void VaultActiveSetUnlockMethodView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage);
    QWidget::showEvent(event);
}
