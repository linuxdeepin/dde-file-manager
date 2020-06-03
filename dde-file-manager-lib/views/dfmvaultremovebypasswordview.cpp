/*
* Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* Maintainer: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "dfmvaultremovebypasswordview.h"
#include "interfaceactivevault.h"

#include <QPushButton>
#include <QHBoxLayout>

#include <DPasswordEdit>

DFMVaultRemoveByPasswordView::DFMVaultRemoveByPasswordView(QWidget *parent)
    : QWidget (parent)
{
    //密码输入框
    m_pwdEdit = new DPasswordEdit(this);
    m_pwdEdit->lineEdit()->setPlaceholderText(tr("Verify your password"));
    m_pwdEdit->lineEdit()->setMaxLength(24);

    // 提示按钮
    m_tipsBtn = new QPushButton(this);
    m_tipsBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));
    m_tipsBtn->setFixedSize(40, 36);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_pwdEdit);
    layout->addWidget(m_tipsBtn);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    connect(m_pwdEdit->lineEdit(), &QLineEdit::textChanged, this, &DFMVaultRemoveByPasswordView::onPasswordChanged);
    connect(m_tipsBtn, &QPushButton::clicked, [this]{
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)){
            strPwdHint.insert(0, tr("Password hint:"));
            m_pwdEdit->showAlertMessage(strPwdHint);
        }
    });
}

QString DFMVaultRemoveByPasswordView::getPassword()
{
    return m_pwdEdit->text();
}

void DFMVaultRemoveByPasswordView::clear()
{
    // 重置状态
    m_pwdEdit->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    m_pwdEdit->lineEdit()->setPalette(palette);
    m_pwdEdit->setEchoMode(QLineEdit::Password);
}

void DFMVaultRemoveByPasswordView::showAlertMessage(const QString &text, int duration)
{
    m_pwdEdit->lineEdit()->setStyleSheet("background-color:rgb(245, 218, 217)");
    m_pwdEdit->showAlertMessage(text, duration);
}

void DFMVaultRemoveByPasswordView::setTipsButtonVisible(bool visible)
{
    m_tipsBtn->setVisible(visible);
}

void DFMVaultRemoveByPasswordView::onPasswordChanged(const QString &password)
{
    if (!password.isEmpty()){
        QLineEdit edit;
        QPalette palette = edit.palette();
        m_pwdEdit->lineEdit()->setPalette(palette);
    }
}
