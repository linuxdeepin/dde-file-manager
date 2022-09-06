// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmvaultremovebypasswordview.h"
#include "interfaceactivevault.h"
#include "accessibility/ac-lib-file-manager.h"
#include "vault/vaultconfig.h"

#include <DToolTip>
#include <DPasswordEdit>
#include <DFloatingWidget>

#include <QPushButton>
#include <QHBoxLayout>
#include <QTimer>

DFMVaultRemoveByPasswordView::DFMVaultRemoveByPasswordView(QWidget *parent)
    : QWidget(parent)
{
    //密码输入框
    m_pwdEdit = new DPasswordEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_pwdEdit, AC_VAULT_DELETE_PASSWORD_EDIT);
    m_pwdEdit->lineEdit()->setPlaceholderText(tr("Password"));
    m_pwdEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pwdEdit->setVisible(false);

    // 提示按钮
    m_tipsBtn = new QPushButton(this);
    AC_SET_ACCESSIBLE_NAME(m_tipsBtn, AC_VAULT_DELETE_HINT_BUTTON);
    m_tipsBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    QHBoxLayout *layout = new QHBoxLayout();
    VaultConfig config;
    QString encryptionMethod = config.get(CONFIG_NODE_NAME, CONFIG_KEY_ENCRYPTION_METHOD, QVariant("NoExist")).toString();
    if (encryptionMethod != CONFIG_METHOD_VALUE_TRANSPARENT) {
        layout->addWidget(m_pwdEdit);
        m_pwdEdit->setVisible(true);
    }
    layout->addWidget(m_tipsBtn);
    layout->setContentsMargins(0, 15, 0, 0);
    this->setLayout(layout);

    connect(m_pwdEdit->lineEdit(), &QLineEdit::textChanged, this, &DFMVaultRemoveByPasswordView::onPasswordChanged);
    connect(m_tipsBtn, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint))
        {
            QString hint = tr("Password hint: %1").arg(strPwdHint);
            showToolTip(hint, 3000, EN_ToolTip::Information);
        }
    });
}

DFMVaultRemoveByPasswordView::~DFMVaultRemoveByPasswordView()
{

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
    // 修复bug-51508 激活密码框的警告状态
    m_pwdEdit->setAlert(true);
    m_pwdEdit->showAlertMessage(text, duration);
}

void DFMVaultRemoveByPasswordView::showToolTip(const QString &text, int duration, DFMVaultRemoveByPasswordView::EN_ToolTip enType)
{
    if (!m_tooltip) {
        m_tooltip = new DToolTip(text);
        m_tooltip->setObjectName("AlertTooltip");
        m_tooltip->setWordWrap(true);

        m_frame = new DFloatingWidget;
        m_frame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        m_frame->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        m_frame->setWidget(m_tooltip);
    }
    if (EN_ToolTip::Warning == enType) {
        // 修复bug-51508 激活密码框的警告状态
        m_pwdEdit->setAlert(true);
        m_tooltip->setForegroundRole(DPalette::TextWarning);
    } else {
        m_tooltip->setForegroundRole(DPalette::TextTitle);
    }

    if (parentWidget() && parentWidget()->parentWidget() && parentWidget()->parentWidget()->parentWidget()) {
        m_frame->setParent(parentWidget()->parentWidget()->parentWidget());
    }

    m_tooltip->setText(text);
    if (m_frame->parent()) {
        // 优化调整 调整悬浮框的显示位置
        QWidget *pWidget = static_cast<QWidget *>(m_frame->parent());
        if (pWidget) {
            m_frame->setGeometry(6, pWidget->height() - 78, 68, 26);
        }
        m_frame->show();
        m_frame->adjustSize();
        m_frame->raise();
    }

    if (duration < 0) {
        return;
    }

    QTimer::singleShot(duration, this, [ = ] {
        m_frame->close();
    });
}

void DFMVaultRemoveByPasswordView::setTipsButtonVisible(bool visible)
{
    m_tipsBtn->setVisible(visible);
}

void DFMVaultRemoveByPasswordView::onPasswordChanged(const QString &password)
{
    if (!password.isEmpty()) {
        // 修复bug-51508 取消密码框的警告状态
        m_pwdEdit->setAlert(false);
    }
}
