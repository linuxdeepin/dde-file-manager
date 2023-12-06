// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivesetunlockmethodview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/policy/policymanager.h"
#include "utils/encryption/vaultconfig.h"
#include "events/vaulteventcaller.h"

#include <dfm-framework/event/event.h>

#include <DPasswordEdit>
#include <DLabel>
#include <DComboBox>
#include <DSpinner>

#include <QDebug>
#include <QToolTip>
#include <QRegExp>
#include <QRegExpValidator>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveSetUnlockMethodView::VaultActiveSetUnlockMethodView(bool tpmAvailabel, QWidget *parent)
    : QWidget(parent)
    , tmpAvailable(tpmAvailabel)
{
    initUi();
    initConnect();

    if (!OperatorCenter::getInstance()->createDirAndFile())
        fmCritical() << "Vault: create dir and file failed!";
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

    initKeyEncryptWidget();
    initTransEncryptWidget();
    if (tmpAvailable) {
        initTPMWithPinEncryptWidget();
        initTPMWithoutPinEncryptWidget();
    }

    gridLayout = new QGridLayout();
    gridLayout->setMargin(0);
    gridLayout->addWidget(pTypeLabel, 0, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(typeCombo, 0, 1, 1, 5);
    gridLayout->addWidget(keyEncryptWidget, 1, 0, 3, 6);

    errorLabel = new DLabel(this);
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    errorLabel->setPalette(pe);
    errorLabel->setWordWrap(true);
    errorLabel->setText("");

    nextBtn = new DPushButton(tr("Next"), this);
    nextBtn->setEnabled(false);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel);
    play->addSpacing(15);
    play->addLayout(gridLayout);
    play->addWidget(errorLabel, 0, Qt::AlignCenter);
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

void VaultActiveSetUnlockMethodView::initKeyEncryptWidget()
{
    typeCombo->addItem(tr("Key encryption"), QVariant(kConfigValueMethodKey));
    curTypeIndex = 0;

    keyEncryptWidget = new QWidget(this);

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

    QGridLayout *encryptWidgetLay = new QGridLayout(keyEncryptWidget);
    encryptWidgetLay->setMargin(0);
    encryptWidgetLay->addWidget(passwordLabel, 0, 0, 1, 1, Qt::AlignLeft);
    encryptWidgetLay->addWidget(passwordEdit, 0, 1, 1, 5);
    encryptWidgetLay->addWidget(repeatPasswordLabel, 1, 0, 1, 1, Qt::AlignLeft);
    encryptWidgetLay->addWidget(repeatPasswordEdit, 1, 1, 1, 5);
    encryptWidgetLay->addWidget(passwordHintLabel, 2, 0, 1, 1, Qt::AlignLeft);
    encryptWidgetLay->addWidget(tipsEdit, 2, 1, 1, 5);
    keyEncryptWidget->setLayout(encryptWidgetLay);
}

void VaultActiveSetUnlockMethodView::initTransEncryptWidget()
{
    typeCombo->addItem(tr("Transparent encryption"), QVariant(kConfigValueMethodTransparent));

    transEncryptWidget = new QWidget(this);
    transEncryptWidget->setVisible(false);

    transEncryptionText = new DLabel(tr("The file vault will be automatically unlocked when accessed, "
                                        "without verifying the password. "
                                        "Files in it will be inaccessible under other user accounts. "),
                                     transEncryptWidget);
    transEncryptionText->setWordWrap(true);

    QVBoxLayout *transEncryptTextLay = new QVBoxLayout();
    transEncryptTextLay->setContentsMargins(10, 0, 0, 0);
    transEncryptTextLay->addWidget(transEncryptionText);
    transEncryptWidget->setLayout(transEncryptTextLay);
}

void VaultActiveSetUnlockMethodView::initTPMWithoutPinEncryptWidget()
{
    typeCombo->addItem(tr("TPM encryption"), QVariant(kConfigValueMethodTpmWithoutPin));

    tpmWithoutPinEncryptWidget = new QWidget(this);
    tpmWithoutPinEncryptWidget->setVisible(false);

    tpmWithoutPinText = new DLabel(tr("The file vault will be automatically unlocked when accessed, "
                                      "without verifying the pin. "
                                      "Files in it will be inaccessible under other user accounts. "),
                                   transEncryptWidget);
    tpmWithoutPinText->setWordWrap(true);

    QVBoxLayout *tpmWithoutPinLay = new QVBoxLayout();
    tpmWithoutPinLay->setContentsMargins(10, 0, 0, 0);
    tpmWithoutPinLay->addWidget(tpmWithoutPinText);
    tpmWithoutPinEncryptWidget->setLayout(tpmWithoutPinLay);
}

void VaultActiveSetUnlockMethodView::initTPMWithPinEncryptWidget()
{
    typeCombo->addItem(tr("TPM with PIN encryption"), QVariant(kConfigValueMethodTpmWithPin));

    tpmWithPinEncryptWidget = new QWidget(this);
    tpmWithPinEncryptWidget->setVisible(false);

    tpmPinLabel = new DLabel(tr("PIN Code"), tpmWithPinEncryptWidget);
    tpmPinEdit = new DPasswordEdit(tpmWithPinEncryptWidget);
    repeatPinLabel = new DLabel(tr("Repeat PIN"), tpmWithPinEncryptWidget);
    repeatPinEdit = new DPasswordEdit(tpmWithPinEncryptWidget);
    tpmPinHintLabel = new DLabel(tr("PIN Hint"), tpmWithPinEncryptWidget);
    tpmPinHintEdit = new DLineEdit(tpmWithPinEncryptWidget);

    QGridLayout *tpmPinEncryptWidgetLay = new QGridLayout();
    tpmPinEncryptWidgetLay->setMargin(0);
    tpmPinEncryptWidgetLay->addWidget(tpmPinLabel, 0, 0, 1, 1, Qt::AlignLeft);
    tpmPinEncryptWidgetLay->addWidget(tpmPinEdit, 0, 1, 1, 5);
    tpmPinEncryptWidgetLay->addWidget(repeatPinLabel, 1, 0, 1, 1, Qt::AlignLeft);
    tpmPinEncryptWidgetLay->addWidget(repeatPinEdit, 1, 1, 1, 5);
    tpmPinEncryptWidgetLay->addWidget(tpmPinHintLabel, 2, 0, 1, 1, Qt::AlignLeft);
    tpmPinEncryptWidgetLay->addWidget(tpmPinHintEdit, 2, 1, 1, 5);

    tpmWithPinEncryptWidget->setLayout(tpmPinEncryptWidgetLay);
}

void VaultActiveSetUnlockMethodView::initConnect()
{
    connect(typeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeChanged(int)));
    if (keyEncryptWidget) {
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
    }
    if (tpmWithPinEncryptWidget) {
        connect(tpmPinEdit, &DPasswordEdit::editingFinished,
                this, &VaultActiveSetUnlockMethodView::tpmPinEditFinished);
        connect(repeatPinEdit, &DPasswordEdit::editingFinished,
                this, &VaultActiveSetUnlockMethodView::repeatPinEditFinished);
    }
    connect(nextBtn, &DPushButton::clicked,
            this, &VaultActiveSetUnlockMethodView::slotNextBtnClicked);
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

void VaultActiveSetUnlockMethodView::tpmPinEditFinished()
{
    bool checkPin = checkTPMPin(tpmPinEdit->text());
    bool checkRepeat = checkRepeatPin();

    if (!checkPin) {
        tpmPinEdit->setAlert(true);
        tpmPinEdit->showAlertMessage(tr("PIN code cannot be empty"));
    } else {
        tpmPinEdit->setAlert(false);
    }

    if (checkPin && checkRepeat)
        nextBtn->setEnabled(true);
    else
        nextBtn->setEnabled(false);
}

void VaultActiveSetUnlockMethodView::repeatPinEditFinished()
{
    bool checkPin = checkTPMPin(tpmPinEdit->text());
    bool checkRepeat = checkRepeatPin();

    if (!checkRepeat) {
        repeatPinEdit->setAlert(true);
        repeatPinEdit->showAlertMessage(tr("PIN code do not match"));
    } else {
        repeatPinEdit->setAlert(false);
    }

    if (checkPin && checkRepeat)
        nextBtn->setEnabled(true);
    else
        nextBtn->setEnabled(false);
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
    config.set(kConfigNodeName, kConfigKeyVersion, QVariant(kConfigVaultVersion1050));

    const QString type = typeCombo->currentData().toString();
    if (kConfigValueMethodKey == type) {
        if (preprocessKeyEncrypt())
            emit sigAccepted();
    } else if (kConfigValueMethodTransparent == type) {
        if (preprocessTranslateEncrypt())
            emit sigAccepted();
    } else if (kConfigValueMethodTpmWithoutPin == type) {
        if (preprocessTpmWithoutPinEncrypt())
            emit sigAccepted();
    } else if (kConfigValueMethodTpmWithPin == type) {
        if (preprocessTpmWithPinEncrypt())
            emit sigAccepted();
    }
}

void VaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    const QString lastType = typeCombo->itemData(curTypeIndex).toString();
    removeEncryptWidgetByType(lastType);

    errorLabel->setText("");
    curTypeIndex = index;
    const QString curType = typeCombo->itemData(index).toString();
    addEncryptWidgetByType(curType);

    checkInputInfo() ? nextBtn->setEnabled(true) : nextBtn->setEnabled(false);
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
    if (QValidator::Acceptable != res)
        return false;
    return true;
}

bool VaultActiveSetUnlockMethodView::checkRepeatPassword()
{
    const QString &strRepeatPassword = repeatPasswordEdit->text();
    const QString &strPassword = passwordEdit->text();
    if (strRepeatPassword != strPassword)
        return false;
    return true;
}

bool VaultActiveSetUnlockMethodView::checkRepeatPin()
{
    if (tpmPinEdit && repeatPinEdit) {
        if (tpmPinEdit->text() == repeatPinEdit->text())
            return true;
    }
    return false;
}

bool VaultActiveSetUnlockMethodView::checkTPMPin(const QString &pinCode)
{
    if (pinCode.isEmpty())
        return false;
    return true;
}

bool VaultActiveSetUnlockMethodView::checkTPMAlgo()
{
    QString backInfo { "" };

    bool ret = OperatorCenter::getInstance()->checkAndGetTpmAlgo(&tpmHashAlgo, &tpmKeyAlgo, &backInfo);
    if (!backInfo.isEmpty())
        errorLabel->setText(backInfo);

    return ret;
}

bool VaultActiveSetUnlockMethodView::checkInputInfo()
{
    const QString type = typeCombo->currentData().toString();
    if (kConfigValueMethodKey == type) {
        if (checkPassword(passwordEdit->text()) && checkRepeatPassword())
            return true;
        return false;
    } else if (kConfigValueMethodTransparent == type) {
        return true;
    } else if (kConfigValueMethodTpmWithoutPin == type) {
        return checkTPMAlgo();
    } else if (kConfigValueMethodTpmWithPin == type) {
        return checkTPMAlgo() && checkTPMPin(tpmPinEdit->text()) && checkRepeatPin();
    }

    return false;
}

void VaultActiveSetUnlockMethodView::removeEncryptWidgetByType(const QString &type)
{
    if (kConfigValueMethodKey == type) {
        gridLayout->removeWidget(keyEncryptWidget);
        keyEncryptWidget->setVisible(false);
    } else if (kConfigValueMethodTransparent == type) {
        gridLayout->removeWidget(transEncryptWidget);
        transEncryptWidget->setVisible(false);
    } else if (kConfigValueMethodTpmWithoutPin == type) {
        gridLayout->removeWidget(tpmWithoutPinEncryptWidget);
        tpmWithoutPinEncryptWidget->setVisible(false);
    } else if (kConfigValueMethodTpmWithPin == type) {
        gridLayout->removeWidget(tpmWithPinEncryptWidget);
        tpmWithPinEncryptWidget->setVisible(false);
    }
}

void VaultActiveSetUnlockMethodView::addEncryptWidgetByType(const QString &type)
{
    if (kConfigValueMethodTransparent == type) {
        gridLayout->addWidget(transEncryptWidget, 2, 1, 3, 5);
        transEncryptWidget->setVisible(true);
    } else if (kConfigValueMethodKey == type) {
        gridLayout->addWidget(keyEncryptWidget, 2, 0, 3, 6);
        keyEncryptWidget->setVisible(true);
    } else if (kConfigValueMethodTpmWithoutPin == type) {
        gridLayout->addWidget(tpmWithoutPinEncryptWidget, 2, 1, 3, 6);
        tpmWithoutPinEncryptWidget->setVisible(true);
    } else if (kConfigValueMethodTpmWithPin == type) {
        gridLayout->addWidget(tpmWithPinEncryptWidget, 2, 0, 3, 6);
        tpmWithPinEncryptWidget->setVisible(true);
    }
}

bool VaultActiveSetUnlockMethodView::preprocessKeyEncrypt()
{
    nextBtn->setEnabled(false);

    QString strPasswordHint = tipsEdit->text();
    if (!OperatorCenter::getInstance()->saveHintInfo(strPasswordHint)) {
        fmCritical() << "Vault: save hint info failed!";
        nextBtn->setEnabled(true);
        return false;
    }

    QString strPassword = passwordEdit->text();
    if (!OperatorCenter::getInstance()->encryptByPBKDF2AndSaveCipher(strPassword)) {
        fmCritical() << "Vault: save password cipher failed!";
        nextBtn->setEnabled(true);
        return false;
    }

    if (!OperatorCenter::getInstance()->createKeyNew(strPassword)) {
        fmCritical() << "Vault: init retrieve pin code password failed!";
        nextBtn->setEnabled(true);
        return false;
    }

    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodKey));

    OperatorCenter::getInstance()->setCryfsPassword(strPassword);

    nextBtn->setEnabled(true);
    return true;
}

bool VaultActiveSetUnlockMethodView::preprocessTranslateEncrypt()
{
    nextBtn->setEnabled(false);

    const QString &password = OperatorCenter::getInstance()->autoGeneratePassword(kPasswordLength);
    if (password.isEmpty()) {
        fmCritical() << "Vault: auto Generate password failed!";
        nextBtn->setEnabled(true);
        return false;
    }

    // save password to keyring
    if (!OperatorCenter::getInstance()->savePasswordToKeyring(password)) {
        errorLabel->setText(tr("Save password to keyring failed!"));
        nextBtn->setEnabled(true);
        return false;
    }

    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodTransparent));

    nextBtn->setEnabled(true);
    return true;
}

bool VaultActiveSetUnlockMethodView::preprocessTpmWithoutPinEncrypt()
{
    nextBtn->setEnabled(false);

    QString password { "" };
    if (!VaultEventCaller::getRandomByTPM(kTpmPasswordSize, &password)) {
        errorLabel->setText(tr("Create password by tpm failed!"));
        nextBtn->setEnabled(true);
        return false;
    }

    // Used to retrieve pin code and password
    if (!OperatorCenter::getInstance()->createKeyNew(password)) {
        errorLabel->setText(tr("Init retrieve password failed!"));
        nextBtn->setEnabled(true);
        return false;
    }

    const QString pinCode { "" };

    if (tpmHashAlgo.isEmpty() || tpmKeyAlgo.isEmpty()) {
        errorLabel->setText(tr("Tpm algo is empty!"));
        nextBtn->setEnabled(true);
        return false;
    }

    OperatorCenter::getInstance()->setCryfsPassword(password);

    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodTpmWithoutPin));
    config.set(kConfigNodeNameOfTPM, kConfigKeyPrimaryHashAlgo, QVariant(tpmHashAlgo));
    config.set(kConfigNodeNameOfTPM, kConfigKeyPrimaryKeyAlgo, QVariant(tpmKeyAlgo));

    nextBtn->setEnabled(true);
    return true;
}

bool VaultActiveSetUnlockMethodView::preprocessTpmWithPinEncrypt()
{
    nextBtn->setEnabled(false);

    const QString pinHint = tpmPinHintEdit->text();
    if (!OperatorCenter::getInstance()->saveHintInfo(pinHint)) {
        fmCritical() << "Vault: save pin hint failed!";
        nextBtn->setEnabled(true);
        return false;
    }

    // Used to verify user input pin code
    const QString pinCode = tpmPinEdit->text();
    if (!OperatorCenter::getInstance()->encryptByPBKDF2AndSaveCipher(pinCode)) {
        nextBtn->setEnabled(true);
        return false;
    }

    QString password { "" };
    if (!VaultEventCaller::getRandomByTPM(kTpmPasswordSize, &password)) {
        errorLabel->setText(tr("Create password by tpm failed!"));
        nextBtn->setEnabled(true);
        return false;
    }

    if (tpmHashAlgo.isEmpty() || tpmKeyAlgo.isEmpty()) {
        errorLabel->setText(tr("Tpm algo is empty!"));
        nextBtn->setEnabled(true);
        return false;
    }

    // Used to retrieve pin code and password
    const QString pinAndPassword = pinCode + "--" + password;
    if (!OperatorCenter::getInstance()->createKeyNew(pinAndPassword)) {
        errorLabel->setText(tr("Init retrieve pin code password failed!"));
        nextBtn->setEnabled(true);
        return false;
    }

    OperatorCenter::getInstance()->setPinCode(pinCode);
    OperatorCenter::getInstance()->setCryfsPassword(password);

    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodTpmWithPin));
    config.set(kConfigNodeNameOfTPM, kConfigKeyPrimaryHashAlgo, QVariant(tpmHashAlgo));
    config.set(kConfigNodeNameOfTPM, kConfigKeyPrimaryKeyAlgo, QVariant(tpmKeyAlgo));

    return true;
}

void VaultActiveSetUnlockMethodView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage);
    QWidget::showEvent(event);
}
