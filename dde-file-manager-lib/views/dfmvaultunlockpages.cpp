/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmvaultunlockpages.h"
#include "vault/interfaceactivevault.h"
#include "controllers/vaultcontroller.h"
#include "dfilemanagerwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QToolTip>
#include <QEvent>

#include <DPushButton>
#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DToolTip>
#include <QTimer>

DFMVaultUnlockPages::DFMVaultUnlockPages(QWidget *parent)
    : DFMVaultPageBase(parent)
{
    setIcon(QIcon::fromTheme("dfm_vault"));
    setFixedSize(396, 218);

    // 标题
    QLabel *pTitle = new QLabel(tr("Unlock File Vault"), this);
    QFont font = pTitle->font();
    font.setPixelSize(18);
    pTitle->setFont(font);
    pTitle->setAlignment(Qt::AlignHCenter);

    // 信息
    QLabel *pMessage = new QLabel(tr("Verify your password"), this);
    pMessage->setAlignment(Qt::AlignHCenter);

    // 密码编辑框
    m_passwordEdit = new DPasswordEdit(this);
    m_passwordEdit->lineEdit()->setPlaceholderText(tr("Password"));
    m_passwordEdit->lineEdit()->installEventFilter(this);
    m_passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    // 提示按钮
    m_tipsButton = new QPushButton(this);
    m_tipsButton->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    // 主视图
    QFrame *mainFrame = new QFrame(this);

    // 布局
    QHBoxLayout *play1 = new QHBoxLayout();
    play1->setMargin(0);
    play1->addWidget(m_passwordEdit);
    play1->addWidget(m_tipsButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitle);
    mainLayout->addWidget(pMessage);
    mainLayout->addLayout(play1);

    mainFrame->setLayout(mainLayout);
    addContent(mainFrame);
    setSpacing(0);
    // 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    QStringList btnList({tr("Cancel"), tr("Unlock")});
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    connect(this, &DFMVaultUnlockPages::buttonClicked, this, &DFMVaultUnlockPages::onButtonClicked);
    connect(m_passwordEdit, &DPasswordEdit::textChanged, this, &DFMVaultUnlockPages::onPasswordChanged);
    connect(VaultController::ins(), &VaultController::signalUnlockVault, this, &DFMVaultUnlockPages::onVaultUlocked);
    connect(m_tipsButton, &QPushButton::clicked, [this]{
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)){
            strPwdHint.insert(0, tr("Password hint:"));
            showToolTip(strPwdHint, 3000, EN_ToolTip::Information);
        }
    });
    connect(this, &DFMVaultPageBase::accepted, this, &DFMVaultPageBase::enterVaultDir);
}

void DFMVaultUnlockPages::showEvent(QShowEvent *event)
{
    // 重置所有控件状态
    m_passwordEdit->lineEdit()->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    m_passwordEdit->lineEdit()->setPalette(palette);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_bUnlockByPwd = false;

    // 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)){
        if (strPwdHint.isEmpty()){
            m_tipsButton->hide();
        } else {
            m_tipsButton->show();
        }
    }
    event->accept();
}

void DFMVaultUnlockPages::showToolTip(const QString &text, int duration, DFMVaultUnlockPages::EN_ToolTip enType)
{
    if (!m_tooltip){
        m_tooltip = new DToolTip(text);
        m_tooltip->setObjectName("AlertTooltip");
        m_tooltip->setWordWrap(true);

        m_frame = new DFloatingWidget;
        m_frame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        m_frame->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        m_frame->setWidget(m_tooltip);
    }
    if(EN_ToolTip::Warning == enType)
        m_tooltip->setForegroundRole(DPalette::TextWarning);
    else
        m_tooltip->setForegroundRole(DPalette::TextTitle);

    m_frame->setParent(this);

    m_tooltip->setText(text);
    if(m_frame->parent()){
        m_frame->setGeometry(8, 154, 68, 26);
        m_frame->show();
        m_frame->adjustSize();
        m_frame->raise();
    }

    if (duration < 0) {
        return;
    }

    QTimer::singleShot(duration, this, [=] {
        m_frame->close();
    });
}

DFMVaultUnlockPages *DFMVaultUnlockPages::instance()
{    
    static DFMVaultUnlockPages s_instance;
    return &s_instance;
}

void DFMVaultUnlockPages::onButtonClicked(const int &index)
{
    if (index == 1){
        QString strPwd = m_passwordEdit->text();

        QString strClipher("");
        if (InterfaceActiveVault::checkPassword(strPwd, strClipher)){
            m_bUnlockByPwd = true;
            VaultController::ins()->unlockVault(strClipher);
        }else {
            // 修复bug-51508 激活密码框警告状态
            m_passwordEdit->setAlert(true);
            showToolTip(tr("Wrong password"), 3000, EN_ToolTip::Warning);
        }
        return;
    }

    close();
}

void DFMVaultUnlockPages::onPasswordChanged(const QString &pwd)
{
    if (!pwd.isEmpty()){
        getButton(1)->setEnabled(true);
        // 修复bug-51508 取消密码框警告状态
        m_passwordEdit->setAlert(false);
    }else {
        getButton(1)->setEnabled(false);
    }
}

void DFMVaultUnlockPages::onVaultUlocked(int state)
{
    if (m_bUnlockByPwd){
        if (state == 0){
            emit accepted();
            close();
        }else {
            // error tips
            QString errMsg = tr("Unlock File Vault failed.%1").arg(VaultController::getErrorInfo(state));
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("Ok"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }

        m_bUnlockByPwd = false;
    }
}

