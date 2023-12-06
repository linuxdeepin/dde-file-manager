// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultcreatepage.h"
#include "createvaultview/vaultactivestartview.h"
#include "createvaultview/vaultactivesetunlockmethodview.h"
#include "createvaultview/vaultactivesavekeyfileview.h"
#include "createvaultview/vaultactivefinishedview.h"
#include "utils/policy/policymanager.h"
#include "utils/encryption/vaultconfig.h"
#include "events/vaulteventcaller.h"

#include <dfm-base/utils/windowutils.h>

#include <DSpinner>

#include <QDebug>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QWindow>
#include <QtConcurrent/QtConcurrent>

using namespace dfmplugin_vault;
DWIDGET_USE_NAMESPACE

VaultActiveView::VaultActiveView(QWidget *parent)
    : VaultPageBase(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    if (dfmbase::WindowUtils::isWayLand()) {
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    this->setIcon(QIcon::fromTheme("dfm_vault"));

    stackedWidget = new QStackedWidget(this);

    startVaultWidget = new VaultActiveStartView(this);
    connect(startVaultWidget, &VaultActiveStartView::sigAccepted,
            this, &VaultActiveView::slotNextWidget);

    stackedWidget->addWidget(startVaultWidget);
    stackedWidget->layout()->setMargin(0);

    this->addContent(stackedWidget);

    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

void VaultActiveView::closeEvent(QCloseEvent *event)
{
    if (!allowClose)
        return event->ignore();

    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kUnknown);
    VaultPageBase::closeEvent(event);
}

void VaultActiveView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage);
    VaultPageBase::showEvent(event);
}

void VaultActiveView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        return;

    VaultPageBase::keyPressEvent(event);
}

void VaultActiveView::slotNextWidget()
{
    if (!stackedWidget)
        return;

    QWidget *curWidget = stackedWidget->currentWidget();
    if (!curWidget)
        return;

    if (curWidget == startVaultWidget) {
        QFutureWatcher<bool> watcher;
        QEventLoop loop;
        connect(&watcher, &QFutureWatcher<bool>::finished, this, [&watcher, &loop]{
            if (watcher.result())
                loop.exit(0);
            else
                loop.exit(-1);
        });
        QFuture<bool> future = QtConcurrent::run([]{
            return VaultEventCaller::checkTPMAvailable();
        });
        watcher.setFuture(future);

        DSpinner spinner(this);
        spinner.setFixedSize(50, 50);
        spinner.move((width() - spinner.width())/2, (height() - spinner.height())/2);
        spinner.start();
        spinner.show();

        allowClose = false;
        bool tpmAvailable = (loop.exec() == 0) ? true : false;
        allowClose = true;

        setUnclockMethodWidget = new VaultActiveSetUnlockMethodView(tpmAvailable, this);
        connect(setUnclockMethodWidget, &VaultActiveSetUnlockMethodView::sigAccepted,
                this, &VaultActiveView::slotNextWidget);
        saveKeyFileWidget = new VaultActiveSaveKeyFileView(this);
        connect(saveKeyFileWidget, &VaultActiveSaveKeyFileView::sigAccepted,
                this, &VaultActiveView::slotNextWidget);
        activeVaultFinishedWidget = new VaultActiveFinishedView(this);
        connect(activeVaultFinishedWidget, &VaultActiveFinishedView::sigAccepted,
                this, &VaultActiveView::slotNextWidget);
        connect(activeVaultFinishedWidget, &VaultActiveFinishedView::setAllowClose,
                this, &VaultActiveView::setAllowClose);
        stackedWidget->addWidget(setUnclockMethodWidget);
        stackedWidget->addWidget(saveKeyFileWidget);
        stackedWidget->addWidget(activeVaultFinishedWidget);

        stackedWidget->setCurrentWidget(setUnclockMethodWidget);
    } else if (curWidget == setUnclockMethodWidget) {
        VaultConfig config;
        QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod).toString();
        if (kConfigValueMethodKey == encryptionMethod
                || kConfigValueMethodTpmWithPin == encryptionMethod
                || kConfigValueMethodTpmWithoutPin == encryptionMethod) {
            stackedWidget->setCurrentWidget(saveKeyFileWidget);
        } else if (kConfigValueMethodTransparent == encryptionMethod) {
            stackedWidget->setCurrentWidget(activeVaultFinishedWidget);
        } else {
            fmCritical() << "Get encryption method failed, can't next!";
        }
    } else if (curWidget == saveKeyFileWidget) {
        stackedWidget->setCurrentWidget(activeVaultFinishedWidget);
    } else if (curWidget == activeVaultFinishedWidget) {
        close();
    }
}

void VaultActiveView::setAllowClose(bool value)
{
    allowClose = value;
}
