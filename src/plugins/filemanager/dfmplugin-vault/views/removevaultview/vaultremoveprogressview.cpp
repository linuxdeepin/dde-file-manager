// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremoveprogressview.h"
#include "utils/vaultdefine.h"
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
    : QWidget(parent), layout(new QVBoxLayout())
{
    deletingWidget = new QWidget(this);
    QVBoxLayout *deletingLay = new QVBoxLayout;
    vaultRmProgressBar = new DWaterProgress(deletingWidget);
    vaultRmProgressBar->setFixedSize(90, 90);
    hintLabel = new DLabel(tr("Removing..."), deletingWidget);
    deletingLay->addWidget(vaultRmProgressBar, 0, Qt::AlignHCenter);
    deletingLay->addWidget(hintLabel, 0, Qt::AlignHCenter);
    deletingWidget->setLayout(deletingLay);

    deletedWidget = new QWidget(this);
    QVBoxLayout *deletedLay = new QVBoxLayout;
    deleteFinishedImageLabel = new DLabel(deletedWidget);
    deleteFinishedImageLabel->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(100, 100));
    deleteFinishedImageLabel->setAlignment(Qt::AlignHCenter);
    finishedLabel = new DLabel(tr("Deleted successfully"), deletedWidget);
    deletedLay->addWidget(deleteFinishedImageLabel, 0, Qt::AlignHCenter);
    deletedLay->addWidget(finishedLabel, 0, Qt::AlignHCenter);
    deletedWidget->setLayout(deletedLay);
    deletedWidget->setHidden(true);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(deletingWidget, 0, Qt::AlignCenter);
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
            layout->removeWidget(deletingWidget);
            deletingWidget->setHidden(true);
            layout->addWidget(deletedWidget, 0, Qt::AlignCenter);
            deletedWidget->setHidden(false);

            Settings setting(kVaultTimeConfigFile);
            setting.removeGroup(QString("VaultTime"));
            VaultAutoLock::instance()->resetConfig();

            VaultHelper::instance()->updateState(VaultState::kNotExisted);
            // report log
            QVariantMap data;
            data.insert("mode", VaultReportData::kDeleted);
            dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_Commit", QString("Vault"), data);

            isExecuted = true;
            emit setBtnEnable(0, true);
        }
    }
}
