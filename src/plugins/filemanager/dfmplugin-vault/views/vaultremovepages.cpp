// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremovepages.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"
#include "utils/policy/policymanager.h"
#include "utils/pathmanager.h"
#include "utils/servicemanager.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"
#include "removevaultview/vaultremoveprogressview.h"
#include "removevaultview/vaultremovebypasswordview.h"
#include "removevaultview/vaultremovebyrecoverykeyview.h"

#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"

#include <dfm-framework/event/event.h>

#include <dfm-base/utils/windowutils.h>

#include <DLabel>

#include <QFrame>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

Q_DECLARE_METATYPE(const char *)

constexpr int kKeyVerifyDeleteBtn = 1;
constexpr int kPassWordDeleteBtn = 2;

using namespace PolkitQt1;
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultRemovePages::VaultRemovePages(QWidget *parent)
    : VaultPageBase(parent),
      passwordView(new VaultRemoveByPasswordView(this)),
      recoverykeyView(new VaultRemoveByRecoverykeyView(this)),
      progressView(new VaultRemoveProgressView(this)),
      stackedWidget(new QStackedWidget(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    if (dfmbase::WindowUtils::isWayLand()) {
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }
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

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(pTitle), AcName::kAcLabelVaultRemoveTitle);
    AddATTag(qobject_cast<QWidget *>(hintInfo), AcName::kAcLabelVaultRemoveContent);
#endif
}

void VaultRemovePages::initConnect()
{
    connect(this, &VaultRemovePages::buttonClicked, this, &VaultRemovePages::onButtonClicked);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalLockVault, this, &VaultRemovePages::onLockVault);
    connect(progressView, &VaultRemoveProgressView::removeFinished, this, &VaultRemovePages::onVaultRemoveFinish);
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
    addButton(tr("OK", "button"), true);
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
    case 1: {   // 删除
        if (stackedWidget->currentIndex() == 0) {
            VaultConfig config;
            const QString &encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
            if (encryptionMethod != QString(kConfigValueMethodTransparent)) {
                // 密码验证
                QString strPwd = passwordView->getPassword();
                QString strCipher("");

                if (!InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
                    passwordView->showToolTip(tr("Wrong password"), 3000, VaultRemoveByPasswordView::EN_ToolTip::kWarning);
                    return;
                }
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

        //! User authorization authentication (asynchronous authorization)
        auto ins = Authority::instance();
        ins->checkAuthorization(kPolkitVaultRemove,
                                UnixProcessSubject(getpid()),
                                Authority::AllowUserInteraction);
        connect(ins, &Authority::checkAuthorizationFinished,
                this, &VaultRemovePages::slotCheckAuthorizationFinished);

        QAbstractButton *btn;
        //! 1050 and above version without key authentication
        if (!VaultHelper::instance()->getVaultVersion()) {
            btn = getButton(kPassWordDeleteBtn);
        } else {
            btn = getButton(kKeyVerifyDeleteBtn);
        }

        //! The button is grayed out to prevent users from operating indiscriminately
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
        QAbstractButton *btn;
        //! 1050 and above version without key authentication
        if (!VaultHelper::instance()->getVaultVersion()) {
            btn = getButton(kPassWordDeleteBtn);
        } else {
            btn = getButton(kKeyVerifyDeleteBtn);
        }
        PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kDeleteVaultPage);
        if (result == Authority::Yes) {
            removeVault = true;
            //! Before deleting, set the copy, clip, and compress task dialog box in the vault as the top-level window.
            if (false) {
                //! todo to be realized
                qDebug() << "Whether there are copying, clipping or compression tasks in the current safe, the vault cannot be deleted!";
            } else {
                //! The verification is successful, first lock the vault
                VaultHelper::instance()->lockVault(false);
            }
            //! The button is grayed out to prevent users from operating indiscriminately
            if (btn)
                btn->setEnabled(false);
        } else {
            if (btn)
                btn->setEnabled(true);
        }
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
            if (state != static_cast<int>(ErrorCode::kResourceBusy)) {
                QString errMsg = tr("Failed to delete file vault");
                DDialog dialog(this);
                dialog.setIcon(QIcon::fromTheme("dialog-warning"));
                dialog.setTitle(errMsg);
                dialog.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
                dialog.exec();
            }
        }
        removeVault = false;
    }
}

void VaultRemovePages::onVaultRemoveFinish(bool result)
{
    using namespace dfmplugin_utils;

    if (result) {
        VaultHelper::instance()->updateState(VaultState::kNotExisted);
        // report log
        QVariantMap data;
        data.insert("mode", VaultReportData::kDeleted);
        dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_Commit", QString("Vault"), data);
        setInfo(tr("Deleted successfully"));
    } else {
        setInfo(tr("Failed to delete"));
    }

    this->getButton(0)->setEnabled(true);
}
