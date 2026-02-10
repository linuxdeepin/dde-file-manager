// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivesetunlockmethodview.h"
#include "utils/vaulthelper.h"
#include "utils/encryption/vaultconfig.h"

#include <dfm-framework/event/event.h>

#include <DPasswordEdit>
#include <DLabel>
#include <DComboBox>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DFontSizeManager>

#include <QDebug>
#include <QToolTip>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QGridLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveSetUnlockMethodView::VaultActiveSetUnlockMethodView(QWidget *parent)
    : VaultBaseView(parent)
{
    initUi();
    initConnect();
}

VaultActiveSetUnlockMethodView::~VaultActiveSetUnlockMethodView()
{
    if (!transEncryptTextLay->parent())
        delete transEncryptTextLay;
}

void VaultActiveSetUnlockMethodView::initUi()
{
    fmDebug() << "Vault: Initializing UI components";

    titleLabel = new DLabel(tr("Set Vault Password"), this);
    titleLabel->setAlignment(Qt::AlignHCenter);

    DLabel *pTypeLabel = new DLabel(tr("Encryption method"), this);
    typeCombo = new DComboBox(this);
    typeCombo->addItem(tr("Key encryption"), EncryptMode::kKeyMode);
    typeCombo->addItem(tr("Transparent encryption"), EncryptMode::kTransparentMode);

    QRegularExpression regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~\'\"|]+");
    QValidator *validator = new QRegularExpressionValidator(regx, this);

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

    nextBtn = new DSuggestButton(tr("Next"), this);
    nextBtn->setFixedWidth(200);
    nextBtn->setEnabled(false);

    gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setVerticalSpacing(10);

    gridLayout->addWidget(pTypeLabel, 0, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(typeCombo, 0, 1, 1, 5);

    gridLayout->addWidget(passwordLabel, 1, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(passwordEdit, 1, 1, 1, 5);

    gridLayout->addWidget(repeatPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(repeatPasswordEdit, 2, 1, 1, 5);

    gridLayout->addWidget(passwordHintLabel, 3, 0, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(tipsEdit, 3, 1, 1, 5);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setContentsMargins(0, 0, 0, 0);
    play->addWidget(titleLabel);
    play->addSpacing(15);
    play->addLayout(gridLayout);
    play->addStretch();
    play->addWidget(nextBtn, 0, Qt::AlignCenter);

    initUiForSizeMode();

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(titleLabel), AcName::kAcLabelVaultSetUnlockTitle);
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

    fmDebug() << "Vault: UI initialization completed successfully";
}

void VaultActiveSetUnlockMethodView::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    DFontSizeManager::instance()->bind(titleLabel, DSizeModeHelper::element(DFontSizeManager::SizeType::T7, DFontSizeManager::SizeType::T5), QFont::Medium);
    fmDebug() << "Vault: Font size manager bound with DSizeMode support";
#else
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::SizeType::T5, QFont::Medium);
    fmDebug() << "Vault: Font size manager bound without DSizeMode support";
#endif
}

void VaultActiveSetUnlockMethodView::initConnect()
{
    fmDebug() << "Vault: Initializing signal connections";

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
            this, &VaultActiveSetUnlockMethodView::accepted);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        fmDebug() << "Vault: Size mode changed, reinitializing UI";
        initUiForSizeMode();
    });
#endif

    fmInfo() << "Vault: All signal connections established successfully";
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

void VaultActiveSetUnlockMethodView::setEncryptInfo(EncryptInfo &info)
{
    info.mode = typeCombo->currentData().value<EncryptMode>();
    if (info.mode == EncryptMode::kKeyMode) {
        info.password = passwordEdit->text();
        info.hint = tipsEdit->text();
    } else {
        fmDebug() << "Vault: Transparent mode - no password/hint needed";
    }
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
        fmWarning() << "Vault: Password validation failed on edit finish";
        nextBtn->setEnabled(false);
        //! 修复BUG-51508 激活密码框警告状态
        passwordEdit->setAlert(true);
        passwordEdit->showAlertMessage(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"), TIPS_TIME);
    } else {
        if (checkInputInfo()) {
            fmDebug() << "Vault: All input validated, enabling next button";
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
        fmWarning() << "Vault: Password mismatch detected";
        //! 修复BUG-51508 激活密码框警告状态
        repeatPasswordEdit->setAlert(true);
        repeatPasswordEdit->showAlertMessage(tr("Passwords do not match"), TIPS_TIME);
    } else {
        fmDebug() << "Vault: Password match confirmed";
    }
}

void VaultActiveSetUnlockMethodView::slotRepeatPasswordEditing()
{
    const QString &strRepeatPassword = repeatPasswordEdit->text();
    const QString &strPassword = passwordEdit->text();

    bool bSizeMatch = strRepeatPassword.size() == strPassword.size();
    if (bSizeMatch) {
        fmDebug() << "Vault: Password lengths match, checking validation";
        if (checkPassword(passwordEdit->text())) {
            if (checkRepeatPassword()) {
                nextBtn->setEnabled(true);
                return;
            } else {
                fmDebug() << "Vault: Passwords don't match, showing alert";
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
        fmDebug() << "Vault: Generated password validation failed";
        nextBtn->setEnabled(false);
    } else {
        fmDebug() << "Vault: Generated password valid, enabling next button";
        nextBtn->setEnabled(true);
    }
}

void VaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    fmDebug() << "Vault: Encryption type changed to index:" << index;
    if (index) {   // transparent encrypyion
        fmDebug() << "Vault: Switching to transparent encryption mode";

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
        fmDebug() << "Vault: Switching to key encryption mode";

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
        QString truncated = password.mid(0, PASSWORD_LENGHT_MAX);
        pPasswordEdit->setText(truncated);
        fmWarning() << "Vault: Password truncated from" << password.length() << "to" << PASSWORD_LENGHT_MAX << "characters";
    }
}

bool VaultActiveSetUnlockMethodView::checkPassword(const QString &password)
{
    QString strPassword = password;

    QRegularExpression rx("^(?![^a-z]+$)(?![^A-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegularExpressionValidator v(rx);
    int pos = 0;
    QValidator::State res;
    res = v.validate(strPassword, pos);
    if (QValidator::Acceptable != res) {
        fmDebug() << "Vault: Password failed validation - insufficient complexity or length";
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
        fmDebug() << "Vault: Passwords do not match";
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
