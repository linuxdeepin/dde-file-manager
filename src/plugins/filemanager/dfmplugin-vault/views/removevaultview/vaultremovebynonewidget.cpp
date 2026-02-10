// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebynonewidget.h"
#include "utils/vaulthelper.h"
#include "utils/vaultutils.h"

#include <DLabel>
#include <DDialog>

#include <QVBoxLayout>

using namespace dfmplugin_vault;
DWIDGET_USE_NAMESPACE

VaultRemoveByNoneWidget::VaultRemoveByNoneWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
}

QStringList VaultRemoveByNoneWidget::btnText() const
{
    return { tr("Cancel"), tr("Delete") };
}

QString VaultRemoveByNoneWidget::titleText() const
{
    return tr("Delete File Vault");
}

void VaultRemoveByNoneWidget::buttonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
    case 0: {
        fmDebug() << "Vault: Cancel button clicked, closing dialog";
        emit closeDialog();
    } break;
    case 1: {
        fmDebug() << "Vault: Delete button clicked, requesting authority dialog";
        VaultUtils::instance().showAuthorityDialog(kPolkitVaultRemove);
        connect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
                this, &VaultRemoveByNoneWidget::slotCheckAuthorizationFinished);
    } break;
    default:
        break;
    }
}

void VaultRemoveByNoneWidget::slotCheckAuthorizationFinished(bool result)
{
    fmDebug() << "Vault: Authorization check finished with result:" << result;

    disconnect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
               this, &VaultRemoveByNoneWidget::slotCheckAuthorizationFinished);

    if (!result) {
        fmWarning() << "Vault: Authorization failed, operation cancelled";
        return;
    }

    fmDebug() << "Vault: Authorization successful, attempting to lock vault";
    if (!VaultHelper::instance()->lockVault(false)) {
        fmCritical() << "Vault: Failed to lock vault for removal";
        QString errMsg = tr("Failed to delete file vault");
        DDialog dialog(this);
        dialog.setIcon(QIcon::fromTheme("dde-file-manager"));
        dialog.setTitle(errMsg);
        dialog.addButton(tr("OK"), true, DDialog::ButtonRecommend);
        fmDebug() << "Vault: Showing error dialog for lock failure";
        dialog.exec();
        return;
    }

    QTimer::singleShot(0, this, [this](){
        emit jumpPage(RemoveWidgetType::kRemoveProgressWidget);
    });
}

void VaultRemoveByNoneWidget::initUI()
{
    DLabel *hintInfo = new DLabel(tr("Once deleted, the files in it will be permanently deleted"), this);
    hintInfo->setAlignment(Qt::AlignCenter);
    hintInfo->setWordWrap(true);

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addWidget(hintInfo);
    setLayout(mainLay);
}
