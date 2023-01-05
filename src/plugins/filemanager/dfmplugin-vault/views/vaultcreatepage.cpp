/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "vaultcreatepage.h"
#include "createvaultview/vaultactivestartview.h"
#include "createvaultview/vaultactivesetunlockmethodview.h"
#include "createvaultview/vaultactivesavekeyfileview.h"
#include "createvaultview/vaultactivefinishedview.h"
#include "utils/policy/policymanager.h"
#include "utils/encryption/vaultconfig.h"

#include <QDebug>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QMouseEvent>

using namespace dfmplugin_vault;
VaultActiveView::VaultActiveView(QWidget *parent)
    : VaultPageBase(parent)
{
    this->setIcon(QIcon::fromTheme("dfm_vault"));

    //! 初始化试图容器
    stackedWidget = new QStackedWidget(this);

    //! 初始化内部窗体
    startVaultWidget = new VaultActiveStartView(this);
    connect(startVaultWidget, &VaultActiveStartView::sigAccepted,
            this, &VaultActiveView::slotNextWidget);
    setUnclockMethodWidget = new VaultActiveSetUnlockMethodView(this);
    connect(setUnclockMethodWidget, &VaultActiveSetUnlockMethodView::sigAccepted,
            this, &VaultActiveView::slotNextWidget);
    saveKeyFileWidget = new VaultActiveSaveKeyFileView(this);
    connect(saveKeyFileWidget, &VaultActiveSaveKeyFileView::sigAccepted,
            this, &VaultActiveView::slotNextWidget);
    activeVaultFinishedWidget = new VaultActiveFinishedView(this);
    connect(activeVaultFinishedWidget, &VaultActiveFinishedView::sigAccepted,
            this, &VaultActiveView::slotNextWidget);

    stackedWidget->addWidget(startVaultWidget);
    stackedWidget->addWidget(setUnclockMethodWidget);
    stackedWidget->addWidget(saveKeyFileWidget);
    stackedWidget->addWidget(activeVaultFinishedWidget);
    stackedWidget->layout()->setMargin(0);

    this->addContent(stackedWidget);
    setMinimumSize(472, 400);

    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

void VaultActiveView::closeEvent(QCloseEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kUnknown);
    //! 响应基类关闭事件
    VaultPageBase::closeEvent(event);
}

void VaultActiveView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage);
    VaultPageBase::showEvent(event);
}

void VaultActiveView::slotNextWidget()
{
    if (stackedWidget) {
        int nIndex = stackedWidget->currentIndex();
        int nCount = stackedWidget->count();
        if (nIndex < nCount - 1) {
            if (nIndex == 1) {  // set encryption method view
                VaultConfig config;
                QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
                if (encryptionMethod == QString(kConfigValueMethodKey)) {
                    stackedWidget->setCurrentIndex(++nIndex);
                } else if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
                    stackedWidget->setCurrentIndex(nIndex + 2);
                } else if (encryptionMethod == QString(kConfigKeyNotExist)) {
                    qWarning() << "Vault: Get encryption method failed, can't next!";
                }
                return;

            }
            stackedWidget->setCurrentIndex(++nIndex);
        } else {
            setBeginingState();
            accept();
        }
    }
}

void VaultActiveView::setBeginingState()
{
    stackedWidget->setCurrentIndex(0);
    setUnclockMethodWidget->clearText();
    activeVaultFinishedWidget->setFinishedBtnEnabled(true);
    setCloseButtonVisible(true);
}
