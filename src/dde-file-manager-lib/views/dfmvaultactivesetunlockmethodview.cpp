// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmvaultactivesetunlockmethodview.h"
#include "operatorcenter.h"
#include "accessibility/ac-lib-file-manager.h"
#include "controllers/vaultcontroller.h"
#include "vault/vaultconfig.h"

#include <DPasswordEdit>
#include <DLabel>

#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QToolTip>
#include <QRegExp>
#include <QRegExpValidator>
#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>

DFMVaultActiveSetUnlockMethodView::DFMVaultActiveSetUnlockMethodView(QWidget *parent)
    : QWidget(parent)
    , m_pTypeCombo(nullptr)
    , m_pPassword(nullptr)
    , m_pRepeatPassword(nullptr)
    , m_pPasswordHintLabel(nullptr)
    , m_pTips(nullptr)
//    , m_pOtherMethod(nullptr)
    , m_pNext(nullptr)
{
    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_ACTIVE_SET_PASSWORD_WIDGET);

    // 设置开锁方式标签
    QLabel *pLabel = new QLabel(tr("Set Vault Password"), this);
    AC_SET_ACCESSIBLE_NAME(pLabel, AC_VAULT_ACTIVE_SET_PASSWORD_TITLE);
    QFont font = pLabel->font();
    font.setPixelSize(18);
    pLabel->setFont(font);
    pLabel->setAlignment(Qt::AlignHCenter);

    // 类型
    DLabel *pTypeLabel = new DLabel(tr("Encryption method"), this);
    AC_SET_ACCESSIBLE_NAME(pTypeLabel, AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_LABEL);
    m_pTypeCombo = new QComboBox(this);
    AC_SET_ACCESSIBLE_NAME(m_pTypeCombo, AC_VAULT_ACTIVE_SET_PASSWORD_TYPE_COMBOX);
    QStringList lstItems;
    lstItems << tr("Key encryption") << tr("Transparent encryption");
    m_pTypeCombo->addItems(lstItems);
    connect(m_pTypeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeChanged(int)));

    // 设置只能输入大小写字母、数字和部分符号的正则表达式
    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~\'\"|]+");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);

    // 设置密码
    m_pPasswordLabel = new DLabel(tr("Password"), this);
    AC_SET_ACCESSIBLE_NAME(m_pPasswordLabel, AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_LABEL);
    m_pPassword = new DPasswordEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_pPassword, AC_VAULT_ACTIVE_SET_PASSWORD_PASSWORD_EDIT);
    m_pPassword->lineEdit()->setValidator(validator);   // 设置验证器
    m_pPassword->lineEdit()->setPlaceholderText(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"));
    m_pPassword->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(m_pPassword, &DPasswordEdit::textEdited,
            this, &DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(m_pPassword, &DPasswordEdit::editingFinished,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditFinished);
    connect(m_pPassword, &DPasswordEdit::textChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditing);
    connect(m_pPassword, &DPasswordEdit::focusChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged);


    // 重复密码
    m_pRepeatPasswordLabel = new DLabel(tr("Repeat password"), this);
    AC_SET_ACCESSIBLE_NAME(m_pRepeatPasswordLabel, AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_LABEL);
    m_pRepeatPassword = new DPasswordEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_pRepeatPassword, AC_VAULT_ACTIVE_SET_PASSWORD_REPASSWORD_EDIT);
    m_pRepeatPassword->lineEdit()->setValidator(validator);   // 设置验证器
    m_pRepeatPassword->lineEdit()->setPlaceholderText(tr("Input the password again"));
    m_pRepeatPassword->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(m_pRepeatPassword, &DPasswordEdit::textEdited,
            this, &DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(m_pRepeatPassword, &DPasswordEdit::editingFinished,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished);
    connect(m_pRepeatPassword, &DPasswordEdit::textChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditing);
    connect(m_pRepeatPassword, &DPasswordEdit::focusChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged);

    // 提示信息
    m_pPasswordHintLabel = new DLabel(tr("Password hint"), this);
    AC_SET_ACCESSIBLE_NAME(m_pPasswordHintLabel, AC_VAULT_ACTIVE_SET_PASSWORD_HINT_LABEL);
    m_pTips = new QLineEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_pTips, AC_VAULT_ACTIVE_SET_PASSWORD_HINT_EDIT);
    m_pTips->setMaxLength(14);
    m_pTips->setPlaceholderText(tr("Optional"));

    // 透明加密描述文本
    TransparentEncryptionText = new DLabel("\n" + tr("The file vault will be automatically unlocked when accessed, "
                                              "without verifying the password. "
                                              "Files in it will be inaccessible under other user accounts. "), this);
    AC_SET_ACCESSIBLE_NAME(TransparentEncryptionText, AC_VAULT_ACTIVE_SET_PASSWORD_TRANSPARENT_LABEL);
    TransparentEncryptionText->setVisible(false);
    TransparentEncryptionText->setWordWrap(true);

    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    AC_SET_ACCESSIBLE_NAME(m_pNext, AC_VAULT_ACTIVE_SET_PASSWORD_NEXT_BUTTON);
    m_pNext->setEnabled(false);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSetUnlockMethodView::slotNextBtnClicked);

    // 布局
    play1 = new QGridLayout();
    play1->setMargin(0);

    play1->addWidget(pTypeLabel, 0, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pTypeCombo, 0, 1, 1, 5);

    play1->addWidget(m_pPasswordLabel, 1, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pPassword, 1, 1, 1, 5);

    play1->addWidget(m_pRepeatPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pRepeatPassword, 2, 1, 1, 5);

    play1->addWidget(m_pPasswordHintLabel, 3, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pTips, 3, 1, 1, 5);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel);
    play->addSpacing(15);
    play->addLayout(play1);
    play->addStretch();
    play->addWidget(m_pNext);

    // 创建文件夹与目录
    if (!OperatorCenter::getInstance()->createDirAndFile()) return;
}

void DFMVaultActiveSetUnlockMethodView::clearText()
{
    m_pPassword->clear();
    m_pPassword->hideAlertMessage();
    // 修复BUG-51508 取消密码框警告状态
    m_pPassword->setAlert(false);
    m_pRepeatPassword->clear();
    m_pRepeatPassword->hideAlertMessage();
    // 修复BUG-51508 取消密码框警告状态
    m_pRepeatPassword->setAlert(false);
    m_pTips->clear();
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditing()
{
    bool ok = checkPassword(m_pPassword->text());
    if (ok) {
        if (checkInputInfo()) {
            m_pNext->setEnabled(true);
        }
    }
    m_pNext->setEnabled(false);
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditFinished()
{
    bool ok = checkPassword(m_pPassword->text());
    if (!ok) {
        m_pNext->setEnabled(false);
        // 修复BUG-51508 激活密码框警告状态
        m_pPassword->setAlert(true);
        m_pPassword->showAlertMessage(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"), TIPS_TIME);
    } else {
        if (checkInputInfo()) {
            m_pNext->setEnabled(true);
        }
    }
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged(bool bFocus)
{
    if (bFocus) {
        // 修复BUG-51508 取消密码框警告状态
        m_pPassword->setAlert(false);
        m_pPassword->hideAlertMessage();
    }
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished()
{
    bool ok = checkRepeatPassword();
    if (!ok) {
        // 修复BUG-51508 激活密码框警告状态
        m_pRepeatPassword->setAlert(true);
        m_pRepeatPassword->showAlertMessage(tr("Passwords do not match"), TIPS_TIME);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditing()
{
    const QString &strRepeatPassword = m_pRepeatPassword->text();
    const QString &strPassword = m_pPassword->text();

    bool bSizeMatch = strRepeatPassword.size() == strPassword.size();
    if (bSizeMatch) {
        if (checkPassword(m_pPassword->text())) {
            if (checkRepeatPassword()) {
                m_pNext->setEnabled(true);
                return;
            } else {
                m_pRepeatPassword->showAlertMessage(tr("Passwords do not match"));
            }
        }
    }
    m_pNext->setEnabled(false);
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged(bool bFocus)
{
    if (bFocus) {
        // 修复BUG-51508 取消密码框警告状态
        m_pRepeatPassword->setAlert(false);
        m_pRepeatPassword->hideAlertMessage();
    }
}

void DFMVaultActiveSetUnlockMethodView::slotGenerateEditChanged(const QString &str)
{
    if (!checkPassword(str)) {
        m_pNext->setEnabled(false);
    } else {
        m_pNext->setEnabled(true);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotNextBtnClicked()
{
    VaultConfig config;
    config.set(CONFIG_NODE_NAME, CONFIG_KEY_USE_USER_PASSWORD, QVariant("1"));
    if (m_pTypeCombo->currentIndex() == 0) {    // 密钥加密
        QString strPassword = m_pPassword->text();
        QString strPasswordHint = m_pTips->text();
        if (OperatorCenter::getInstance()->saveSaltAndCiphertext(strPassword, strPasswordHint)
                && OperatorCenter::getInstance()->createKeyNew(strPassword)) {
            // 记录加密方式
            config.set(CONFIG_NODE_NAME, CONFIG_KEY_ENCRYPTION_METHOD, QVariant(CONFIG_METHOD_VALUE_KEY));

            emit sigAccepted();
        }
    } else {    // 透明加密
        // 生成随机密码
        const QString &strPassword = OperatorCenter::getInstance()->autoGeneratePassword(18);
        if (strPassword.isEmpty()) {
            qWarning() << "auto Generate password failed!";
            return;
        }

        // 保存随机密码到Keyring
        if (OperatorCenter::getInstance()->setPasswordToKeyring(strPassword)) {
            // 记录加密方式
            config.set(CONFIG_NODE_NAME, CONFIG_KEY_ENCRYPTION_METHOD, QVariant(CONFIG_METHOD_VALUE_TRANSPARENT));
            // 记录版本信息
            config.set(CONFIG_NODE_NAME, CONFIG_KEY_VERSION, QVariant(CONFIG_VAULT_VERSION_1050));

            emit sigAccepted();
        }
    }
}

void DFMVaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    if (index) { // 透明加密
        play1->removeWidget(m_pPasswordLabel);
        play1->removeWidget(m_pPassword);
        play1->removeWidget(m_pRepeatPasswordLabel);
        play1->removeWidget(m_pRepeatPassword);
        play1->removeWidget(m_pPasswordHintLabel);
        play1->removeWidget(m_pTips);
        m_pPasswordLabel->setVisible(false);
        m_pPassword->setVisible(false);
        m_pRepeatPasswordLabel->setVisible(false);
        m_pRepeatPassword->setVisible(false);
        m_pPasswordHintLabel->setVisible(false);
        m_pTips->setVisible(false);

        play1->addWidget(TransparentEncryptionText, 1, 0, 3, 6);
        TransparentEncryptionText->setVisible(true);

        m_pNext->setEnabled(true);
    } else { // 密钥加密
        play1->removeWidget(TransparentEncryptionText);
        TransparentEncryptionText->setVisible(false);

        play1->addWidget(m_pPasswordLabel, 1, 0, 1, 1, Qt::AlignLeft);
        play1->addWidget(m_pPassword, 1, 1, 1, 5);
        play1->addWidget(m_pRepeatPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
        play1->addWidget(m_pRepeatPassword, 2, 1, 1, 5);
        play1->addWidget(m_pPasswordHintLabel, 3, 0, 1, 1, Qt::AlignLeft);
        play1->addWidget(m_pTips, 3, 1, 1, 5);
        m_pPasswordLabel->setVisible(true);
        m_pPassword->setVisible(true);
        m_pRepeatPasswordLabel->setVisible(true);
        m_pRepeatPassword->setVisible(true);
        m_pPasswordHintLabel->setVisible(true);
        m_pTips->setVisible(true);

        checkInputInfo() ? m_pNext->setEnabled(true) : m_pNext->setEnabled(false);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength(const QString &password)
{
    DPasswordEdit *pPasswordEdit = qobject_cast<DPasswordEdit *>(sender());
    if (password.length() > PASSWORD_LENGHT_MAX) {
        pPasswordEdit->setText(password.mid(0, PASSWORD_LENGHT_MAX));
    }
}

bool DFMVaultActiveSetUnlockMethodView::checkPassword(const QString &password)
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

bool DFMVaultActiveSetUnlockMethodView::checkRepeatPassword()
{
    const QString &strRepeatPassword = m_pRepeatPassword->text();
    const QString &strPassword = m_pPassword->text();
    if (strRepeatPassword != strPassword) {
        return false;
    } else {
        return true;
    }
}

bool DFMVaultActiveSetUnlockMethodView::checkInputInfo()
{
    if (checkPassword(m_pPassword->text()) && checkRepeatPassword()) {
        return true;
    } else {
        return false;
    }
}

void DFMVaultActiveSetUnlockMethodView::showEvent(QShowEvent *event)
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::CREATEVAULTPAGE);
    QWidget::showEvent(event);
}
