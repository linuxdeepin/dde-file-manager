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

#include "vaultactivesetunlockmethodview.h"
#include "utils/encryption/operatorcenter.h"

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

DPVAULT_USE_NAMESPACE
VaultActiveSetUnlockMethodView::VaultActiveSetUnlockMethodView(QWidget *parent)
    : QWidget(parent)
{
    //! 设置开锁方式标签
    QLabel *pLabel = new QLabel(tr("Set Vault Password"), this);
    QFont font = pLabel->font();
    font.setPixelSize(18);
    pLabel->setFont(font);
    pLabel->setAlignment(Qt::AlignHCenter);

    //! 类型
    DLabel *pTypeLabel = new DLabel(tr("Method"), this);
    typeCombo = new QComboBox(this);
    QStringList lstItems;
    lstItems << tr("Manual") /* << tr("Random")*/;
    typeCombo->addItems(lstItems);
    connect(typeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTypeChanged(int)));

    //! 设置只能输入大小写字母、数字和部分符号的正则表达式
    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~\'\"|]+");
    //! 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);

    //! 设置密码
    passwordLabel = new DLabel(tr("Password"), this);
    passwordEdit = new DPasswordEdit(this);
    passwordEdit->lineEdit()->setValidator(validator);   //! 设置验证器
    passwordEdit->lineEdit()->setPlaceholderText(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"));
    passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(passwordEdit, &DPasswordEdit::textEdited,
            this, &VaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(passwordEdit, &DPasswordEdit::editingFinished,
            this, &VaultActiveSetUnlockMethodView::slotPasswordEditFinished);
    connect(passwordEdit, &DPasswordEdit::textChanged,
            this, &VaultActiveSetUnlockMethodView::slotPasswordEditing);
    connect(passwordEdit, &DPasswordEdit::focusChanged,
            this, &VaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged);

    //! 重复密码
    repeatPasswordLabel = new DLabel(tr("Repeat password"), this);
    repeatPasswordEdit = new DPasswordEdit(this);
    repeatPasswordEdit->lineEdit()->setValidator(validator);   //! 设置验证器
    repeatPasswordEdit->lineEdit()->setPlaceholderText(tr("Input the password again"));
    repeatPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(repeatPasswordEdit, &DPasswordEdit::textEdited,
            this, &VaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(repeatPasswordEdit, &DPasswordEdit::editingFinished,
            this, &VaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished);
    connect(repeatPasswordEdit, &DPasswordEdit::textChanged,
            this, &VaultActiveSetUnlockMethodView::slotRepeatPasswordEditing);
    connect(repeatPasswordEdit, &DPasswordEdit::focusChanged,
            this, &VaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged);

    //! 提示信息
    passwordHintLabel = new DLabel(tr("Password hint"), this);
    tipsEdit = new QLineEdit(this);
    tipsEdit->setMaxLength(14);
    tipsEdit->setPlaceholderText(tr("Optional"));

    //! 下一步按钮
    nextBtn = new QPushButton(tr("Next"), this);
    nextBtn->setEnabled(false);
    connect(nextBtn, &QPushButton::clicked,
            this, &VaultActiveSetUnlockMethodView::slotNextBtnClicked);

    //! 布局
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

    //! 创建文件夹与目录
    if (!OperatorCenter::getInstance()->createDirAndFile()) return;
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
    QString strPassword = passwordEdit->text();
    QString strPasswordHint = tipsEdit->text();
    if (OperatorCenter::getInstance()->saveSaltAndCiphertext(strPassword, strPasswordHint)
        && OperatorCenter::getInstance()->createKeyNew(strPassword))
        emit sigAccepted();
}

void VaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    if (index) {   // 随机

        gridLayout->removeWidget(passwordLabel);
        gridLayout->removeWidget(passwordEdit);
        gridLayout->removeWidget(repeatPasswordLabel);
        gridLayout->removeWidget(repeatPasswordEdit);

        passwordLabel->setVisible(false);
        passwordEdit->setVisible(false);
        repeatPasswordLabel->setVisible(false);
        repeatPasswordEdit->setVisible(false);
    } else {   // 手动

        gridLayout->addWidget(passwordLabel, 2, 0, 1, 1, Qt::AlignLeft);
        gridLayout->addWidget(passwordEdit, 2, 1, 1, 5);
        gridLayout->addWidget(repeatPasswordLabel, 3, 0, 1, 1, Qt::AlignLeft);
        gridLayout->addWidget(repeatPasswordEdit, 3, 1, 1, 5);

        passwordLabel->setVisible(true);
        passwordEdit->setVisible(true);
        repeatPasswordLabel->setVisible(true);
        repeatPasswordEdit->setVisible(true);

        //! 检测密码正确性
        slotRepeatPasswordEditFinished();
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
    QWidget::showEvent(event);
}
