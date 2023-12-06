// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremoveprogressview.h"
#include "utils/vaultautolock.h"
#include "utils/vaulthelper.h"
#include "utils/encryption/operatorcenter.h"

#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"

#include <dfm-base/base/application/settings.h>

#include <dfm-framework/event/event.h>

#include <DWaterProgress>
#include <DLabel>

#include <QProgressBar>
#include <QFile>
#include <QDir>
#include <QThread>

#include <thread>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;
using namespace dfmplugin_utils;

VaultRemoveProgressView::VaultRemoveProgressView(QWidget *parent)
    : QWidget(parent)
    , vaultRmProgressBar(new DWaterProgress(this))
    , deleteFinishedImageLabel(new DLabel(this))
    , layout(new QVBoxLayout())
{
    hintLabel = new DLabel(tr("Removing..."), this);

    vaultRmProgressBar->setFixedSize(80, 80);

    deleteFinishedImageLabel->setPixmap(QIcon::fromTheme("dfm_vault_active_finish").pixmap(90, 90));
    deleteFinishedImageLabel->setAlignment(Qt::AlignHCenter);
    deleteFinishedImageLabel->hide();

    layout->setMargin(0);
    layout->addSpacing(10);
    layout->addWidget(hintLabel, 1, Qt::AlignHCenter);
    layout->addWidget(vaultRmProgressBar, 1, Qt::AlignHCenter);
    this->setLayout(layout);

    connect(OperatorCenter::getInstance(), &OperatorCenter::fileRemovedProgress,
            this, &VaultRemoveProgressView::handleVaultRemovedProgress);
}

VaultRemoveProgressView::~VaultRemoveProgressView()
{
    vaultRmProgressBar->setValue(0);
    vaultRmProgressBar->stop();
}

QStringList VaultRemoveProgressView::btnText()
{
    return { tr("OK") };
}

QString VaultRemoveProgressView::titleText()
{
    return tr("Delete File Vault");
}

void VaultRemoveProgressView::buttonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
    case 0: {
        emit sigCloseDialog();
    } break;
    default:
        break;
    }
}

void VaultRemoveProgressView::removeVault(const QString &basePath)
{
    vaultRmProgressBar->start();
    isExecuted = false;
    emit setBtnEnable(0, false);
    OperatorCenter::getInstance()->removeVault(basePath);
}

void VaultRemoveProgressView::handleVaultRemovedProgress(int value)
{
    if (vaultRmProgressBar->value() != 100)
        vaultRmProgressBar->setValue(value);

    if (value == 100) {
        if (!isExecuted) {
            vaultRmProgressBar->setValue(value);
            layout->removeWidget(vaultRmProgressBar);
            vaultRmProgressBar->hide();
            layout->addWidget(deleteFinishedImageLabel);
            deleteFinishedImageLabel->show();

            Settings setting(kVaultTimeConfigFile);
            setting.removeGroup(QString("VaultTime"));
            VaultAutoLock::instance()->resetConfig();

            VaultHelper::instance()->updateState(VaultState::kNotExisted);
            // report log
            QVariantMap data;
            data.insert("mode", VaultReportData::kDeleted);
            dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_Commit", QString("Vault"), data);
            hintLabel->setText(tr("Deleted successfully"));

            isExecuted = true;
            emit setBtnEnable(0, true);
        }
    }
}
