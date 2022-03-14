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

#include "vaultactiveview.h"
#include "vaultactivestartview.h"
#include "vaultactivesetunlockmethodview.h"
#include "vaultactivesavekeyfileview.h"
#include "vaultactivefinishedview.h"
#include "utils/vaulthelper.h"

#include <QDebug>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QMouseEvent>

DPVAULT_USE_NAMESPACE
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
    setMinimumSize(472, 346);

    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

void VaultActiveView::closeEvent(QCloseEvent *event)
{
    VaultHelper::instance()->setVauleCurrentPageMark(VaultHelper::VaultPageMark::kUnknown);
    //! 响应基类关闭事件
    VaultPageBase::closeEvent(event);
}

void VaultActiveView::showEvent(QShowEvent *event)
{
    VaultHelper::instance()->setVauleCurrentPageMark(VaultHelper::VaultPageMark::kCreateVaultPage);
    VaultPageBase::showEvent(event);
}

void VaultActiveView::slotNextWidget()
{
    if (stackedWidget) {
        int nIndex = stackedWidget->currentIndex();
        int nCount = stackedWidget->count();
        if (nIndex < nCount - 1) {
            int nNextIndex = nIndex + 1;
            stackedWidget->setCurrentIndex(nNextIndex);
        } else {
            close();
        }
    }
}
