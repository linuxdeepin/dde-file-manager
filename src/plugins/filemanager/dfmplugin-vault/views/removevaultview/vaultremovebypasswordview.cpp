// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebypasswordview.h"
#include "utils/vaulthelper.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"

#include <dfm-framework/event/event.h>

#include <DToolTip>
#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DDialog>
#include <DFontSizeManager>

#include <QPushButton>
#include <QHBoxLayout>
#include <QTimer>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;
using namespace PolkitQt1;

VaultRemoveByPasswordView::VaultRemoveByPasswordView(QWidget *parent)
    : QWidget(parent)
{
    DLabel *hintInfo = new DLabel(tr("Once deleted, the files in it will be permanently deleted"), this);
    hintInfo->setAlignment(Qt::AlignCenter);
    hintInfo->setWordWrap(true);

    pwdEdit = new DPasswordEdit(this);
    pwdEdit->lineEdit()->setPlaceholderText(tr("Password"));
    pwdEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    tipsBtn = new QPushButton(this);
    tipsBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(pwdEdit);
    layout->addWidget(tipsBtn);
    layout->setContentsMargins(0, 0, 0, 0);

    if (!VaultHelper::instance()->getVaultVersion()) {
        keyDeleteLabel = new DLabel(tr("Key delete"));
        DFontSizeManager::instance()->bind(keyDeleteLabel, DFontSizeManager::T8, QFont::Medium);
        keyDeleteLabel->installEventFilter(this);
        keyDeleteLabel->setForegroundRole(DPalette::ColorType::LightLively);
    }

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addWidget(hintInfo);
    mainLay->addItem(layout);
    if (keyDeleteLabel)
        mainLay->addWidget(keyDeleteLabel, 0, Qt::AlignRight);
    setLayout(mainLay);

    connect(pwdEdit->lineEdit(), &QLineEdit::textChanged, this, &VaultRemoveByPasswordView::onPasswordChanged);
    connect(tipsBtn, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (OperatorCenter::getInstance()->getPasswordHint(strPwdHint)) {
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

QStringList VaultRemoveByPasswordView::btnText() const
{
    return { tr("Cancel"), tr("Delete") };
}

QString VaultRemoveByPasswordView::titleText() const
{
    return tr("Delete File Vault");
}

void VaultRemoveByPasswordView::buttonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
    case 0: {
        emit sigCloseDialog();
    } break;
    case 1: {
        QString strPwd = pwdEdit->text();
        QString strCipher("");
        if (!OperatorCenter::getInstance()->checkPassword(strPwd, strCipher)) {
            showToolTip(tr("Wrong password"), 3000, VaultRemoveByPasswordView::EN_ToolTip::kWarning);
            return;
        }

        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultRemove,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultRemoveByPasswordView::slotCheckAuthorizationFinished);
    } break;
    default:
        break;
    }
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

    if (parentWidget() && parentWidget()->parentWidget()) {
        floatWidget->setParent(parentWidget()->parentWidget());
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

void VaultRemoveByPasswordView::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultRemoveByPasswordView::slotCheckAuthorizationFinished);

    if (Authority::Yes != result)
        return;

    if (!VaultHelper::instance()->lockVault(false)) {
        QString errMsg = tr("Failed to delete file vault");
        DDialog dialog(this);
        dialog.setIcon(QIcon::fromTheme("dialog-warning"));
        dialog.setTitle(errMsg);
        dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);
        dialog.exec();
        return;
    }

    emit signalJump(RemoveWidgetType::kRemoveProgressWidget);
}

bool VaultRemoveByPasswordView::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj && obj == keyDeleteLabel) {
        if (evt->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(evt);
            if (mouseEvent->button() == Qt::LeftButton) {
                emit signalJump(RemoveWidgetType::kRecoveryKeyWidget);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, evt);
}
