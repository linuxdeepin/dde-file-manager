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

#include "vaultremovepages.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"
#include "utils/policy/policymanager.h"
#include "utils/pathmanager.h"
#include "utils/servicemanager.h"
#include "removevaultview/vaultremoveprogressview.h"
#include "removevaultview/vaultremovebypasswordview.h"
#include "removevaultview/vaultremovebyrecoverykeyview.h"
#include "services/filemanager/fileencrypt/fileencryptservice.h"

#include <DLabel>
#include <QFrame>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QVBoxLayout>

using namespace PolkitQt1;

DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
VaultRemovePages::VaultRemovePages(QWidget *parent)
    : VaultPageBase(parent),
      passwordView(new VaultRemoveByPasswordView(this)),
      recoverykeyView(new VaultRemoveByRecoverykeyView(this)),
      progressView(new VaultRemoveProgressView(this)),
      stackedWidget(new QStackedWidget(this))
{
    setIcon(QIcon(":/icons/deepin/builtin/icons/dfm_vault_32px.svg"));
    //! 修复bug-41001 提示信息显示不全
    this->setFixedWidth(396);
    //! 标题
    DLabel *pTitle = new DLabel(tr("Delete File Vault"), this);
    //! 文本水平并垂直居中
    pTitle->setAlignment(Qt::AlignCenter);
    //! 信息
    hintInfo = new QLabel(this);
    //! 文本水平并垂直居中
    hintInfo->setAlignment(Qt::AlignCenter);
    //! 修复bug-41001 提示信息显示不全
    hintInfo->setWordWrap(true);
    //! 主界面
    QFrame *mainFrame = new QFrame(this);

    //! 修复bug-41001 提示信息显示不全
    stackedWidget->setFixedHeight(95);
    stackedWidget->addWidget(passwordView);
    stackedWidget->addWidget(recoverykeyView);
    stackedWidget->addWidget(progressView);

    //! 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitle);
    mainLayout->addWidget(hintInfo);
    mainLayout->addWidget(stackedWidget);

    mainFrame->setLayout(mainLayout);
    addContent(mainFrame);

    showVerifyWidget();

    //! 防止点击按钮隐藏界面
    setOnButtonClickedClose(false);

    initConnect();
}

void VaultRemovePages::initConnect()
{
    connect(this, &VaultRemovePages::buttonClicked, this, &VaultRemovePages::onButtonClicked);
    connect(ServiceManager::fileEncryptServiceInstance(), &FileEncryptService::signalLockVaultState, this, &VaultRemovePages::onLockVault);
    connect(progressView, &VaultRemoveProgressView::removeFinished, this, &VaultRemovePages::onVualtRemoveFinish);
}

void VaultRemovePages::showVerifyWidget()
{
    setInfo(tr("Once deleted, the files in it will be permanently deleted"));

    setCloseButtonVisible(true);
    clearButtons();
    QStringList buttonTexts({ tr("Cancel", "button"), tr("Use Key", "button"), tr("Delete", "button") });
    addButton(buttonTexts[0], false);
    //! 1050及以上版本无密钥验证
    if (!VaultHelper::instance()->getVaultVersion())
        addButton(buttonTexts[1], false);
    addButton(buttonTexts[2], true, DDialog::ButtonWarning);
    if (!VaultHelper::instance()->getVaultVersion())
        setDefaultButton(2);
    else
        setDefaultButton(1);
    stackedWidget->setCurrentIndex(0);

    //! 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
        if (strPwdHint.isEmpty()) {
            passwordView->setTipsButtonVisible(false);
        } else {
            passwordView->setTipsButtonVisible(true);
        }
    }
}

void VaultRemovePages::showRemoveWidget()
{
    setInfo(tr("Removing..."));
    clearButtons();
    addButton(tr("OK", "button"), true, ButtonType::ButtonRecommend);
    getButton(0)->setEnabled(false);
    stackedWidget->setCurrentIndex(2);
}

void VaultRemovePages::setInfo(const QString &info)
{
    hintInfo->setText(info);
}

void VaultRemovePages::closeEvent(QCloseEvent *event)
{
    // 重置界面状态
    passwordView->clear();
    recoverykeyView->clear();
    progressView->clear();
    removeVault = false;

    // 调用基类关闭事件
    VaultPageBase::closeEvent(event);
}

void VaultRemovePages::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kDeleteVaultPage);
    VaultPageBase::showEvent(event);
}

void VaultRemovePages::onButtonClicked(int index)
{
    switch (index) {
    case 0:   //点击取消按钮
        close();
        break;
    case 1:
        //! 1050及以上版本无密钥验证
        if (!VaultHelper::instance()->getVaultVersion()) {
            {   // 切换验证方式
                if (stackedWidget->currentIndex() == 0) {
                    getButton(1)->setText(tr("Use Password"));
                    stackedWidget->setCurrentIndex(1);
                } else {
                    getButton(1)->setText(tr("Use Key"));
                    stackedWidget->setCurrentIndex(0);
                }
            }
            break;
        }
    case 2: {   // 删除
        if (stackedWidget->currentIndex() == 0) {
            // 密码验证
            QString strPwd = passwordView->getPassword();
            QString strCipher("");

            if (!InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
                passwordView->showToolTip(tr("Wrong password"), 3000, VaultRemoveByPasswordView::EN_ToolTip::kWarning);
                return;
            }
        } else {
            // 密钥验证
            QString strKey = recoverykeyView->getRecoverykey();
            strKey.replace("-", "");
            QString strCipher("");

            if (!InterfaceActiveVault::checkUserKey(strKey, strCipher)) {
                recoverykeyView->showAlertMessage(tr("Wrong recovery key"));
                return;
            }
        }

        // 用户权限认证(异步授权)
        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultRemove,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultRemovePages::slotCheckAuthorizationFinished);

        QAbstractButton *btn;
        //! 1050及以上版本无密钥验证
        if (!VaultHelper::instance()->getVaultVersion()) {
            btn = getButton(2);
        } else {
            btn = getButton(1);
        }

        // 按钮置灰，防止用户胡乱操作
        if (btn)
            btn->setEnabled(false);
    } break;
    default:
        break;
    }
}

void VaultRemovePages::slotCheckAuthorizationFinished(Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultRemovePages::slotCheckAuthorizationFinished);
    if (isVisible()) {
        PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kDeleteVaultPage);
        if (result == Authority::Yes) {
            removeVault = true;
            // 删除前，先置顶保险箱内拷贝、剪贴、压缩任务
            if (false) {
                //! todo 待实现
                qDebug() << "当前保险箱内是否有拷贝、剪贴、压缩任务，不能删除保险箱！";
            } else {
                // 验证成功，先对保险箱进行上锁
                VaultHelper::instance()->lockVault();
            }
        }

        QAbstractButton *btn;
        //! 1050及以上版本无密钥验证
        if (!VaultHelper::instance()->getVaultVersion()) {
            btn = getButton(2);
        } else {
            btn = getButton(1);
        }

        // 按钮置灰，防止用户胡乱操作
        if (btn)
            btn->setEnabled(false);
    }
}

void VaultRemovePages::onLockVault(int state)
{
    if (removeVault) {
        if (state == 0) {
            // 切换至删除界面
            showRemoveWidget();

            QString vaultLockPath = PathManager::vaultLockPath();
            QString vaultUnlockPath = PathManager::vaultUnlockPath();
            progressView->removeVault(vaultLockPath, vaultUnlockPath);
        } else {
            // error tips
            QString errMsg = tr("Failed to delete file vault");
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }
        removeVault = false;
    }
}

void VaultRemovePages::onVualtRemoveFinish(bool result)
{
    if (result) {
        setInfo(tr("Deleted successfully"));
    } else {
        setInfo(tr("Failed to delete"));
    }

    this->getButton(0)->setEnabled(true);
}
