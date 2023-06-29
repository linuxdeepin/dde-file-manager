// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebypasswordview.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/encryption/vaultconfig.h"

#include <dfm-framework/event/event.h>

#include <DToolTip>
#include <DPasswordEdit>
#include <DFloatingWidget>

#include <QPushButton>
#include <QHBoxLayout>
#include <QTimer>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultRemoveByPasswordView::VaultRemoveByPasswordView(QWidget *parent)
    : QWidget(parent)
{
    //! 密码输入框
    pwdEdit = new DPasswordEdit(this);
    pwdEdit->lineEdit()->setPlaceholderText(tr("Password"));
    pwdEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    pwdEdit->setVisible(false);

    //! 提示按钮
    tipsBtn = new QPushButton(this);
    tipsBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    QHBoxLayout *layout = new QHBoxLayout();
    VaultConfig config;
    const QString &encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encryptionMethod != QString(kConfigValueMethodTransparent)) {
        layout->addWidget(pwdEdit);
        pwdEdit->setVisible(true);
    }
    layout->addWidget(tipsBtn);
    layout->setContentsMargins(0, 15, 0, 0);
    this->setLayout(layout);

    connect(pwdEdit->lineEdit(), &QLineEdit::textChanged, this, &VaultRemoveByPasswordView::onPasswordChanged);
    connect(tipsBtn, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
            QString hint = tr("Password hint: %1").arg(strPwdHint);
            showToolTip(hint, 3000, EN_ToolTip::kInformation);
        }
    });

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(pwdEdit), AcName::kAcEditVaultRemovePassword);
    AddATTag(qobject_cast<QWidget *>(tipsBtn), AcName::kAcBtnVaultRemovePasswordHint);
#endif
}

VaultRemoveByPasswordView::~VaultRemoveByPasswordView()
{
}

QString VaultRemoveByPasswordView::getPassword()
{
    return pwdEdit->text();
}

void VaultRemoveByPasswordView::clear()
{
    //! 重置状态
    pwdEdit->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    pwdEdit->lineEdit()->setPalette(palette);
    pwdEdit->setEchoMode(QLineEdit::Password);
}

void VaultRemoveByPasswordView::showAlertMessage(const QString &text, int duration)
{
    //! 修复bug-51508 激活密码框的警告状态
    pwdEdit->setAlert(true);
    pwdEdit->showAlertMessage(text, duration);
}

void VaultRemoveByPasswordView::showToolTip(const QString &text, int duration, VaultRemoveByPasswordView::EN_ToolTip enType)
{
    if (!tooltip) {
        tooltip = new DToolTip(text);
        tooltip->setObjectName("AlertTooltip");
        tooltip->setWordWrap(true);

        floatWidget = new DFloatingWidget;
        floatWidget->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        floatWidget->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        floatWidget->setWidget(tooltip);
    }
    if (EN_ToolTip::kWarning == enType) {
        //! 修复bug-51508 激活密码框的警告状态
        pwdEdit->setAlert(true);
        tooltip->setForegroundRole(DPalette::TextWarning);
    } else {
        tooltip->setForegroundRole(DPalette::TextTitle);
    }

    if (parentWidget() && parentWidget()->parentWidget() && parentWidget()->parentWidget()->parentWidget()) {
        floatWidget->setParent(parentWidget()->parentWidget()->parentWidget());
    }

    tooltip->setText(text);
    if (floatWidget->parent()) {
        //! 优化调整 调整悬浮框的显示位置
        QWidget *pWidget = static_cast<QWidget *>(floatWidget->parent());
        if (pWidget) {
            floatWidget->setGeometry(6, pWidget->height() - 78, 68, 26);
        }
        floatWidget->show();
        floatWidget->adjustSize();
        floatWidget->raise();
    }

    if (duration < 0) {
        return;
    }

    QTimer::singleShot(duration, this, [=] {
        floatWidget->close();
    });
}

void VaultRemoveByPasswordView::setTipsButtonVisible(bool visible)
{
    tipsBtn->setVisible(visible);
}

void VaultRemoveByPasswordView::onPasswordChanged(const QString &password)
{
    if (!password.isEmpty()) {
        //! 修复bug-51508 取消密码框的警告状态
        pwdEdit->setAlert(false);
    }
}
