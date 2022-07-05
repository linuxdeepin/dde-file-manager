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

#include "vaultactivefinishedview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/servicemanager.h"
#include "utils/policy/policymanager.h"
#include "utils/fileencrypthandle.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/settings.h"

#include <DLabel>
#include <DDialog>
#include <DWaterProgress>
#include <DIconButton>

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QTimer>

using namespace PolkitQt1;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveFinishedView::VaultActiveFinishedView(QWidget *parent)
    : QWidget(parent), tipsLabel(nullptr), encryptFinishedImageLabel(nullptr), tipsThree(nullptr), finishedBtn(nullptr)
{
    // 标题
    QLabel *pLabelTitle = new QLabel(tr("Encrypt File Vault"), this);
    QFont font = pLabelTitle->font();
    font.setPixelSize(18);
    pLabelTitle->setFont(font);
    pLabelTitle->setAlignment(Qt::AlignHCenter);

    // 加密提示
    tipsLabel = new DLabel(tr("Click 'Encrypt' and input the user password."), this);
    tipsLabel->setAlignment(Qt::AlignHCenter);

    // 加密保险箱图片
    encryVaultImageLabel = new DLabel(this);
    encryVaultImageLabel->setPixmap(QIcon::fromTheme("dfm_vault_active_encrypt").pixmap(98, 88));
    encryVaultImageLabel->setAlignment(Qt::AlignHCenter);

    // 进度条
    waterProgress = new DWaterProgress(this);
    waterProgress->setValue(1);
    waterProgress->setFixedSize(98, 98);
    // 进度条提示
    tipsLabelone = new DLabel(tr("Encrypting..."), this);
    tipsLabelone->setAlignment(Qt::AlignHCenter);

    // 加密完成完成图片
    encryptFinishedImageLabel = new DLabel(this);
    encryptFinishedImageLabel->setPixmap(QIcon::fromTheme("dfm_vault_active_finish").pixmap(128, 128));
    encryptFinishedImageLabel->setAlignment(Qt::AlignHCenter);
    // 加密完成提示
    tipsThree = new DLabel(tr("The setup is complete"), this);
    tipsThree->setAlignment(Qt::AlignHCenter);

    // 加密保险箱按钮
    finishedBtn = new QPushButton(tr("Encrypt"), this);
    connect(finishedBtn, &QPushButton::clicked,
            this, &VaultActiveFinishedView::slotEncryptVault);

    // 布局
    widgetOne = new QWidget(this);
    QVBoxLayout *play1 = new QVBoxLayout(widgetOne);
    play1->setMargin(0);
    play1->addWidget(tipsLabel);
    play1->addSpacing(22);
    play1->addWidget(encryVaultImageLabel);

    widgetTow = new QWidget(this);
    QVBoxLayout *play2 = new QVBoxLayout(widgetTow);
    play2->setContentsMargins(0, 22, 0, 0);
    play2->addWidget(waterProgress, 0, Qt::AlignCenter);
    play2->addSpacing(22);
    play2->addWidget(tipsLabelone, 0, Qt::AlignCenter);

    widgetThree = new QWidget(this);
    QVBoxLayout *play3 = new QVBoxLayout(widgetThree);
    play3->setContentsMargins(0, 15, 0, 0);
    play3->addWidget(encryptFinishedImageLabel);
    play3->addSpacing(10);
    play3->addWidget(tipsThree);

    QVBoxLayout *m_pLay = new QVBoxLayout(this);
    m_pLay->setMargin(0);
    m_pLay->addWidget(pLabelTitle);
    m_pLay->addSpacing(10);
    m_pLay->addWidget(widgetOne);
    m_pLay->addWidget(widgetTow, 0, Qt::AlignHCenter);
    m_pLay->addWidget(widgetThree);
    m_pLay->addStretch();
    m_pLay->addWidget(finishedBtn);

    widgetTow->setVisible(false);
    widgetThree->setVisible(false);

    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalCreateVault,
            this, &VaultActiveFinishedView::slotEncryptComplete);

    // 初始化定时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout,
            this, &VaultActiveFinishedView::slotTimeout);
}

void VaultActiveFinishedView::setFinishedBtnEnabled(bool b)
{
    finishedBtn->setEnabled(b);
    finishedBtn->setText(tr("Encrypt"));
    widgetOne->setVisible(true);
    widgetTow->setVisible(false);
    widgetThree->setVisible(false);
}

void VaultActiveFinishedView::slotEncryptComplete(int nState)
{
    if (nState == 0) {   // 创建保险箱成功
        waterProgress->setValue(100);
        waterProgress->stop();
        repaint();
        timer->setSingleShot(true);
        timer->start(500);
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyCreateTime);
    } else {
        QMessageBox::warning(this, QString(), QString(tr("Failed to create file vault: %1").arg(nState)));
    }
}

void VaultActiveFinishedView::slotEncryptVault()
{
    if (finishedBtn->text() == tr("Encrypt")) {
        // 异步授权
        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultCreate,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultActiveFinishedView::slotCheckAuthorizationFinished);
        // 灰化按钮，避免异步时用户再次点击按钮
        finishedBtn->setEnabled(false);
    } else {
        // 切换到保险箱主页面
        VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(), VaultHelper::instance()->rootUrl());
        emit sigAccepted();
    }
}

void VaultActiveFinishedView::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultActiveFinishedView::slotCheckAuthorizationFinished);
    if (isVisible()) {
        PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage1);
        if (result == Authority::Yes) {
            if (finishedBtn->text() == tr("Encrypt")) {
                // 完成按钮灰化
                finishedBtn->setEnabled(false);

                // 进度条
                waterProgress->start();
                widgetOne->setVisible(false);
                widgetTow->setVisible(true);
                widgetThree->setVisible(false);

                std::thread t([]() {
                    // 调用创建保险箱接口
                    // 拿到密码
                    QString strPassword = OperatorCenter::getInstance()->getSaltAndPasswordCipher();
                    if (!strPassword.isEmpty()) {
                        VaultHelper::instance()->createVault(strPassword);
                        OperatorCenter::getInstance()->clearSaltAndPasswordCipher();
                    } else
                        qDebug() << "获取cryfs密码为空，创建保险箱失败！";
                });
                t.detach();
            }
        } else {
            finishedBtn->setEnabled(true);
        }
    }
}

void VaultActiveFinishedView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage1);
    QWidget::showEvent(event);
}

void VaultActiveFinishedView::slotTimeout()
{
    widgetOne->setVisible(false);
    widgetTow->setVisible(false);
    widgetThree->setVisible(true);
    finishedBtn->setText(tr("OK"));
    finishedBtn->setEnabled(true);
    // 显示右上角关闭按钮
    if (parentWidget()) {
        DDialog *pParent = qobject_cast<DDialog *>(parentWidget()->parentWidget());
        if (pParent) {
            pParent->setCloseButtonVisible(true);
        }
    }
}
