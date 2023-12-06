// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebytpmpinwidget.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"

#include <DLabel>
#include <DDialog>

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace dfmplugin_vault;
DWIDGET_USE_NAMESPACE
using namespace PolkitQt1;

VaultRemoveByTpmPinWidget::VaultRemoveByTpmPinWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
}

QStringList VaultRemoveByTpmPinWidget::btnText() const
{
    return { tr("Cancel"), tr("Delete") };
}

QString VaultRemoveByTpmPinWidget::titleText() const
{
    return tr("Delete File Vault");
}

void VaultRemoveByTpmPinWidget::buttonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
    case 0: {
        emit closeDialog();
    } break;
    case 1: {
        const QString pinCode = pinEdit->text();
        QString cipher { "" };

        if (!OperatorCenter::getInstance()->checkPassword(pinCode, cipher)) {
            pinEdit->showAlertMessage(tr("Wrong PIN"));
            return;
        }

        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultRemove,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultRemoveByTpmPinWidget::slotCheckAuthorizationFinished);
    } break;
    default:
        break;
    }
}

void VaultRemoveByTpmPinWidget::showHintInfo()
{
    QString hint { "" };
    if (OperatorCenter::getInstance()->getPasswordHint(hint)) {
        if (!hint.isEmpty()) {
            hint = tr("PIN hint: ") + hint;
            pinEdit->showAlertMessage(hint);
        }
    }
}

void VaultRemoveByTpmPinWidget::slotCheckAuthorizationFinished(Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultRemoveByTpmPinWidget::slotCheckAuthorizationFinished);

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

    emit jumpPage(RemoveWidgetType::kRemoveProgressWidget);
}

void VaultRemoveByTpmPinWidget::initUI()
{
    DLabel *hintInfo = new DLabel(tr("Once deleted, the files in it will be permanently deleted"), this);
    hintInfo->setAlignment(Qt::AlignCenter);
    hintInfo->setWordWrap(true);

    pinEdit =  new DPasswordEdit(this);
    pinEdit->lineEdit()->setPlaceholderText(tr("PIN Code"));
    pinEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    tipsBtn = new DPushButton(this);
    tipsBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    QHBoxLayout *lay = new QHBoxLayout;
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setMargin(0);
    lay->addWidget(pinEdit);
    lay->addWidget(tipsBtn);

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addWidget(hintInfo);
    mainLay->addItem(lay);
    setLayout(mainLay);
}

void VaultRemoveByTpmPinWidget::initConnect()
{
    connect(tipsBtn, &DPushButton::clicked, this, &VaultRemoveByTpmPinWidget::showHintInfo);
}
