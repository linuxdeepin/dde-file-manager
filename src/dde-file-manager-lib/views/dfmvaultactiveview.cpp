// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmvaultactiveview.h"
#include "dfmvaultactivestartview.h"
#include "dfmvaultactivesetunlockmethodview.h"
#include "dfmvaultactivesavekeyfileview.h"
#include "dfmvaultactivefinishedview.h"
#include "accessibility/ac-lib-file-manager.h"
#include "controllers/vaultcontroller.h"
#include "vault/vaultconfig.h"

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
    this->setAttribute(Qt::WA_DeleteOnClose, true);

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

DFMVaultActiveView::~DFMVaultActiveView()
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::UNKNOWN);
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
            if (nIndex == 1) {  // 设置加密方式界面
                VaultConfig config;
                QString encryptionMethod = config.get(CONFIG_NODE_NAME, CONFIG_KEY_ENCRYPTION_METHOD, QVariant("NoExist")).toString();
                if (encryptionMethod == CONFIG_METHOD_VALUE_KEY) {
                    m_pStackedWidget->setCurrentIndex(nIndex + 1);
                } else if (encryptionMethod == CONFIG_METHOD_VALUE_TRANSPARENT) {
                    m_pStackedWidget->setCurrentIndex(nIndex + 2);
                } else if (encryptionMethod == "NoExist") {
                    qWarning() << "Get encryption method failed, cant not next!";
                }
                return;
            }
            m_pStackedWidget->setCurrentIndex(nIndex + 1);
        } else {
            setBeginingState();   // 控件文本恢复初值
            emit accept();
        }
    }
}
