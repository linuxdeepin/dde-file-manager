/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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


#include "dfmvaultremovepages.h"
#include "vault/interfaceactivevault.h"
#include "vault/vaultlockmanager.h"
#include "vault/vaulthelper.h"
#include "controllers/vaultcontroller.h"
#include "app/define.h"
#include "dfmvaultremoveprogressview.h"
#include "dfmvaultremovebypasswordview.h"
#include "dfmvaultremovebyrecoverykeyview.h"
#include "accessibility/ac-lib-file-manager.h"
#include "vaulthelper.h"

#include <DLabel>
#include <QFrame>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QVBoxLayout>

using namespace PolkitQt1;

DWIDGET_USE_NAMESPACE

DFMVaultRemovePages::DFMVaultRemovePages(QWidget *parent)
    : DFMVaultPageBase(parent)
    , m_passwordView(new DFMVaultRemoveByPasswordView(this))
    , m_recoverykeyView(new DFMVaultRemoveByRecoverykeyView(this))
    , m_progressView(new DFMVaultRemoveProgressView(this))
    , m_stackedWidget(new QStackedWidget(this))
{
    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_DELETE_WIDGET);

    setIcon(QIcon(":/icons/deepin/builtin/icons/dfm_vault_32px.svg"));
    // 修复bug-41001 提示信息显示不全
    this->setFixedWidth(396);

    // 标题
    DLabel *pTitle = new DLabel(tr("Delete File Vault"), this);
    AC_SET_ACCESSIBLE_NAME(pTitle, AC_VAULT_DELETE_TITLE);
    // 文本水平并垂直居中
    pTitle->setAlignment(Qt::AlignCenter);

    // 信息
    m_pInfo = new QLabel(this);
    AC_SET_ACCESSIBLE_NAME(m_pInfo, AC_VAULT_DELETE_CONTENT);
    // 文本水平并垂直居中
    m_pInfo->setAlignment(Qt::AlignCenter);
    // 修复bug-41001 提示信息显示不全
    m_pInfo->setWordWrap(true);

    // 主界面
    QFrame *mainFrame = new QFrame(this);

    // 修复bug-41001 提示信息显示不全
    m_stackedWidget->setFixedHeight(95);
    m_stackedWidget->addWidget(m_passwordView);
    m_stackedWidget->addWidget(m_recoverykeyView);
    m_stackedWidget->addWidget(m_progressView);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitle);
    mainLayout->addWidget(m_pInfo);
    mainLayout->addWidget(m_stackedWidget);

    mainFrame->setLayout(mainLayout);
    addContent(mainFrame);

    showVerifyWidget();

    // 防止点击按钮隐藏界面
    setOnButtonClickedClose(false);

    initConnect();
}

void DFMVaultRemovePages::initConnect()
{
    connect(this, &DFMVaultRemovePages::buttonClicked, this, &DFMVaultRemovePages::onButtonClicked);
    connect(VaultController::ins(), &VaultController::signalLockVault, this, &DFMVaultRemovePages::onLockVault);
    connect(m_progressView, &DFMVaultRemoveProgressView::removeFinished, this, &DFMVaultRemovePages::onVualtRemoveFinish);
}

void DFMVaultRemovePages::showVerifyWidget()
{
    setInfo(tr("Once deleted, the files in it will be permanently deleted"));

    setCloseButtonVisible(true);
    clearButtons();
    QStringList buttonTexts({tr("Cancel","button"), tr("Use Key","button"), tr("Delete","button")});
    addButton(buttonTexts[0], false);
    //! 1050及以上版本无密钥验证
    if(!VaultController::getVaultVersion())
        addButton(buttonTexts[1], false);
    addButton(buttonTexts[2], true, DDialog::ButtonWarning);
    if(!VaultController::getVaultVersion())
        setDefaultButton(2);
    else
        setDefaultButton(1);
    m_stackedWidget->setCurrentIndex(0);

    AC_SET_ACCESSIBLE_NAME(getButton(0), AC_VAULT_DELETE_CANCEL_BUTTON);
    AC_SET_ACCESSIBLE_NAME(getButton(1), AC_VAULT_DELETE_CHANGE_BUTTON);
    if(!VaultController::getVaultVersion())
        AC_SET_ACCESSIBLE_NAME(getButton(2), AC_VAULT_DELETE_DELETE_BUTTON);

    // 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
        if (strPwdHint.isEmpty()) {
            m_passwordView->setTipsButtonVisible(false);
        } else {
            m_passwordView->setTipsButtonVisible(true);
        }
    }
}

void DFMVaultRemovePages::showRemoveWidget()
{
    setInfo(tr("Removing..."));

    setCloseButtonVisible(false);
    clearButtons();
    addButton(tr("OK","button"), true, ButtonType::ButtonRecommend);
    getButton(0)->setEnabled(false);
    m_stackedWidget->setCurrentIndex(2);
}

void DFMVaultRemovePages::setInfo(const QString &info)
{
    m_pInfo->setText(info);
}

void DFMVaultRemovePages::closeEvent(QCloseEvent *event)
{
    // 重置界面状态
    m_passwordView->clear();
    m_recoverykeyView->clear();
    m_progressView->clear();
    m_bRemoveVault = false;

    // 调用基类关闭事件
    DFMVaultPageBase::closeEvent(event);
}

DFMVaultRemovePages *DFMVaultRemovePages::instance()
{
    static DFMVaultRemovePages s_instance;
    return &s_instance;
}

void DFMVaultRemovePages::showTop()
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::DELETEVAULTPAGE);
    showVerifyWidget();
    activateWindow();
    show();
    raise();
    // 设置当前保险箱处于模态弹窗状态
    DFM_NAMESPACE::VaultHelper::isModel = true;
}

void DFMVaultRemovePages::onButtonClicked(int index)
{
    switch (index) {
    case 0: //点击取消按钮
        close();
        break;
    case 1:
     //! 1050及以上版本无密钥验证
        if(!VaultController::getVaultVersion()){
            { // 切换验证方式
                if (m_stackedWidget->currentIndex() == 0) {
                    getButton(1)->setText(tr("Use Password"));
                    m_stackedWidget->setCurrentIndex(1);
                } else {
                    getButton(1)->setText(tr("Use Key"));
                    m_stackedWidget->setCurrentIndex(0);
                }
            }
            break;
        }
    case 2: { // 删除
        if (m_stackedWidget->currentIndex() == 0) {
            // 密码验证
            QString strPwd = m_passwordView->getPassword();
            QString strCipher("");

            if (!InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
                m_passwordView->showToolTip(tr("Wrong password"), 3000, DFMVaultRemoveByPasswordView::EN_ToolTip::Warning);
                return;
            }
        } else {
            // 密钥验证
            QString strKey = m_recoverykeyView->getRecoverykey();
            strKey.replace("-", "");
            QString strCipher("");

            if (!InterfaceActiveVault::checkUserKey(strKey, strCipher)) {
                m_recoverykeyView->showAlertMessage(tr("Wrong recovery key"));
                return;
            }
        }

        // 用户权限认证(异步授权)
        auto ins = Authority::instance();
        ins->checkAuthorization(VAULT_REMOVE,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &DFMVaultRemovePages::slotCheckAuthorizationFinished);

        QAbstractButton * btn;
        //! 1050及以上版本无密钥验证
        if(!VaultController::getVaultVersion()) {
            btn = getButton(2);
        }else {
            btn = getButton(1);
        }

        // 按钮置灰，防止用户胡乱操作
        if (btn)
            btn->setEnabled(false);
    }
    break;
    default:
        break;
    }
}

void DFMVaultRemovePages::slotCheckAuthorizationFinished(Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
            this, &DFMVaultRemovePages::slotCheckAuthorizationFinished);
    if (isVisible()) {
        VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::DELETEVAULTPAGE);
        if (result == Authority::Yes) {
            m_bRemoveVault = true;
            // 删除前，先置顶保险箱内拷贝、剪贴、压缩任务
            if (DFM_NAMESPACE::VaultHelper::topVaultTasks()) {
                qDebug() << "当前保险箱内是否有拷贝、剪贴、压缩任务，不能删除保险箱！";
            } else {
                // 验证成功，先对保险箱进行上锁
                VaultController::ins()->lockVault();
            }
        }

        QAbstractButton * btn;
        //! 1050及以上版本无密钥验证
        if(!VaultController::getVaultVersion()) {
            btn = getButton(2);
        }else {
            btn = getButton(1);
        }

        // 按钮置灰，防止用户胡乱操作
        if (btn)
            btn->setEnabled(false);
    }

}

void DFMVaultRemovePages::onLockVault(int state)
{
    if (m_bRemoveVault) {
        if (state == 0) {
            // 切换至删除界面
            showRemoveWidget();
            emit accepted();

            QString vaultLockPath = VaultController::ins()->vaultLockPath();
            QString vaultUnlockPath = VaultController::ins()->vaultUnlockPath();
            m_progressView->removeVault(vaultLockPath, vaultUnlockPath);
        } else {
            // error tips
            QString errMsg = tr("Failed to delete file vault");
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }
        m_bRemoveVault = false;
    }
}

void DFMVaultRemovePages::onVualtRemoveFinish(bool result)
{
    if (result) {
        setInfo(tr("Deleted successfully"));
    } else {
        setInfo(tr("Failed to delete"));
    }

    this->getButton(0)->setEnabled(true);
}
