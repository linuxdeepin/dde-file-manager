/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "dfmvaultactiveview.h"
#include "dfmvaultactivestartview.h"
#include "dfmvaultactivesetunlockmethodview.h"
#include "dfmvaultactivesavekeyfileview.h"
#include "dfmvaultactivefinishedview.h"
#include "accessibility/ac-lib-file-manager.h"
#include "controllers/vaultcontroller.h"

#include <QDebug>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QMouseEvent>

DFMVaultActiveView::DFMVaultActiveView(QWidget *parent)
    : DFMVaultPageBase(parent),
      m_pStackedWidget(nullptr),
      m_pStartVaultWidget(nullptr),
      m_pSetUnclockMethodWidget(nullptr),
      m_SaveKeyWidget(nullptr),
      m_ActiveVaultFinishedWidget(nullptr),
      m_SaveKeyFileWidget(nullptr)
{
    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_ACTIVE_WIDGET);
    this->setIcon(QIcon::fromTheme("dfm_vault"));

    // 初始化试图容器
    m_pStackedWidget = new QStackedWidget(this);
    AC_SET_ACCESSIBLE_NAME(m_pStackedWidget, AC_VAULT_ACTIVE_STACK);

    // 初始化内部窗体
    m_pStartVaultWidget = new DFMVaultActiveStartView(this);
    connect(m_pStartVaultWidget, &DFMVaultActiveStartView::sigAccepted,
            this, &DFMVaultActiveView::slotNextWidget);
    m_pSetUnclockMethodWidget = new DFMVaultActiveSetUnlockMethodView(this);
    connect(m_pSetUnclockMethodWidget, &DFMVaultActiveSetUnlockMethodView::sigAccepted,
            this, &DFMVaultActiveView::slotNextWidget);
    m_SaveKeyFileWidget = new DFMVaultActiveSaveKeyFileView(this);
    connect(m_SaveKeyFileWidget, &DFMVaultActiveSaveKeyFileView::sigAccepted,
            this, &DFMVaultActiveView::slotNextWidget);
    m_ActiveVaultFinishedWidget = new DFMVaultActiveFinishedView(this);
    connect(m_ActiveVaultFinishedWidget, &DFMVaultActiveFinishedView::sigAccepted,
            this, &DFMVaultActiveView::slotNextWidget);

    m_pStackedWidget->addWidget(m_pStartVaultWidget);
    m_pStackedWidget->addWidget(m_pSetUnclockMethodWidget);
    m_pStackedWidget->addWidget(m_SaveKeyFileWidget);
    m_pStackedWidget->addWidget(m_ActiveVaultFinishedWidget);
    m_pStackedWidget->layout()->setMargin(0);

    this->addContent(m_pStackedWidget);
    setMinimumSize(472, 346);

    connect(this, &DFMVaultPageBase::accepted, this, &DFMVaultPageBase::enterVaultDir);

    connect(VaultController::ins(), &VaultController::sigCloseWindow, this, &DFMVaultActiveView::close);
}

void DFMVaultActiveView::setBeginingState()
{
    m_pStackedWidget->setCurrentIndex(0);
    m_pSetUnclockMethodWidget->clearText();
    m_ActiveVaultFinishedWidget->setFinishedBtnEnabled(true);
    this->setCloseButtonVisible(true);
}

void DFMVaultActiveView::closeEvent(QCloseEvent *event)
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::UNKNOWN);
    setBeginingState();
    // 响应基类关闭事件
    DFMVaultPageBase::closeEvent(event);
}

void DFMVaultActiveView::showEvent(QShowEvent *event)
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::CREATEVAULTPAGE);
    DFMVaultPageBase::showEvent(event);
}

void DFMVaultActiveView::slotNextWidget()
{
    if (m_pStackedWidget) {
        int nIndex = m_pStackedWidget->currentIndex();
        int nCount = m_pStackedWidget->count();
        if (nIndex < nCount - 1) {
            int nNextIndex = nIndex + 1;
            m_pStackedWidget->setCurrentIndex(nNextIndex);
        } else {
            setBeginingState();   // 控件文本恢复初值
            emit accept();
        }
    }
}
