// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovebynonewidget.h"
#include "utils/vaulthelper.h"

#include <DLabel>
#include <DDialog>

#include <QVBoxLayout>

using namespace dfmplugin_vault;
using namespace PolkitQt1;
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
        emit closeDialog();
    } break;
    case 1: {
        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultRemove,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultRemoveByNoneWidget::slotCheckAuthorizationFinished);
    } break;
    default:
        break;
    }
}

void VaultRemoveByNoneWidget::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultRemoveByNoneWidget::slotCheckAuthorizationFinished);

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
