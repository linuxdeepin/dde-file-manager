// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultcreatepage.h"
#include "createvaultview/vaultactivestartview.h"
#include "createvaultview/vaultactivesetunlockmethodview.h"
#include "createvaultview/vaultactivesavekeyfileview.h"
#include "createvaultview/vaultactivefinishedview.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QWindow>
#include <QtConcurrent>

inline constexpr int kWidth { 472 };

using namespace dfmplugin_vault;

VaultActiveView::VaultActiveView(QWidget *parent)
    : VaultPageBase(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    if (dfmbase::WindowUtils::isWayLand()) {
        fmDebug() << "Vault: Setting Wayland window properties";
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    this->setIcon(QIcon::fromTheme("safebox"));

    //! 初始化试图容器
    stackedWidget = new QStackedWidget(this);

    //! 初始化内部窗体
    startVaultWidget = new VaultActiveStartView(this);
    connect(startVaultWidget, &VaultActiveStartView::accepted,
            this, &VaultActiveView::slotNextWidget);
    setUnclockMethodWidget = new VaultActiveSetUnlockMethodView(this);
    connect(setUnclockMethodWidget, &VaultActiveSetUnlockMethodView::accepted,
            this, &VaultActiveView::slotNextWidget);
    saveKeyFileWidget = new VaultActiveSaveKeyFileView(this);
    connect(saveKeyFileWidget, &VaultActiveSaveKeyFileView::sigAccepted,
            this, &VaultActiveView::slotNextWidget);
    activeVaultFinishedWidget = new VaultActiveFinishedView(this);
    connect(activeVaultFinishedWidget, &VaultActiveFinishedView::accepted,
            this, &VaultActiveView::slotNextWidget);
    connect(activeVaultFinishedWidget, &VaultActiveFinishedView::reqEncryptVault,
            this, &VaultActiveView::encryptVault);
    connect(VaultHelper::instance(), &VaultHelper::sigCreateVault,
            this, [this](int state) {
                Result ret { state == 0, state == 0 ? "" : tr("Failed to create vault: %1").arg(state) };
                activeVaultFinishedWidget->encryptFinished(ret.result, ret.message);
            });

    stackedWidget->addWidget(startVaultWidget);
    stackedWidget->addWidget(setUnclockMethodWidget);
    stackedWidget->addWidget(saveKeyFileWidget);
    stackedWidget->addWidget(activeVaultFinishedWidget);
    stackedWidget->layout()->setContentsMargins(0, 0, 0, 0);

    this->addContent(stackedWidget);
    setFixedWidth(kWidth);

    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

void VaultActiveView::slotNextWidget()
{
    auto view = qobject_cast<VaultBaseView *>(sender());
    if (view)
        view->setEncryptInfo(encryptInfo);

    if (stackedWidget) {
        int nIndex = stackedWidget->currentIndex();
        int nCount = stackedWidget->count();
        if (nIndex < nCount - 1) {
            if (nIndex == 1) {   // set encryption method view
                if (encryptInfo.mode == EncryptMode::kKeyMode) {
                    fmDebug() << "Vault: Switching to key mode configuration view";
                    // 在切换到保存密钥文件页面之前，预生成恢复密钥
                    QString recoveryKey = OperatorCenter::getInstance()->getRecoveryKey();
                    if (recoveryKey.isEmpty()) {
                        recoveryKey = OperatorCenter::getInstance()->generateRecoveryKeyForNewVault();
                        fmDebug() << "Vault: Recovery key generated successfully";
                    }
                    stackedWidget->setCurrentIndex(++nIndex);
                } else if (encryptInfo.mode == EncryptMode::kTransparentMode) {
                    fmDebug() << "Vault: Switching to transparent mode configuration view";
                    stackedWidget->setCurrentIndex(nIndex + 2);
                }
                return;
            }
            stackedWidget->setCurrentIndex(++nIndex);
        } else {
            fmDebug() << "Vault: Reached final step, completing vault creation process";
            setBeginingState();
            accept();
        }
    }
}

void VaultActiveView::encryptVault()
{
    auto ret = OperatorCenter::getInstance()->createDirAndFile();
    if (!ret.result) {
        fmWarning() << "Vault: Failed to create directory and file:" << ret.message;
        activeVaultFinishedWidget->encryptFinished(false, ret.message);
        return;
    }
    activeVaultFinishedWidget->setProgressValue(10);

    bool result = false;
    switch (encryptInfo.mode) {
    case EncryptMode::kKeyMode:
        fmInfo() << "Vault: Processing key mode encryption";
        result = handleKeyModeEncryption();
        break;
    case EncryptMode::kTransparentMode:
        fmInfo() << "Vault: Processing transparent mode encryption";
        result = handleTransparentModeEncryption();
        break;
    }

    if (!result) {
        fmWarning() << "Vault: Encryption setup failed for mode:" << static_cast<int>(encryptInfo.mode);
        return;
    }

    fmInfo() << "Vault: Encryption setup completed, starting async vault creation";
    asyncCreateVault();
}

void VaultActiveView::setBeginingState()
{
    stackedWidget->setCurrentIndex(0);
    setUnclockMethodWidget->clearText();
    activeVaultFinishedWidget->setFinishedBtnEnabled(true);
    setCloseButtonVisible(true);
}

void VaultActiveView::asyncCreateVault()
{
    auto watcher = new QFutureWatcher<Result>();
    connect(watcher, &QFutureWatcherBase::finished, this, [watcher, this]() {
        auto ret = watcher->result();
        if (!ret.result) {
            fmWarning() << "Vault: Async vault creation failed:" << ret.message;
            activeVaultFinishedWidget->encryptFinished(false, ret.message);
        } else {
            fmInfo() << "Vault: Async vault creation completed successfully";
        }
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run(&VaultActiveView::createVault, this));
}

Result VaultActiveView::createVault()
{
    VaultConfig config;
    QString encrypitonMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encrypitonMethod == QString(kConfigKeyNotExist)) {
        fmWarning() << "Vault: Get encryption method failed!";
        return { false, tr("Failed to create vault: Get encryption method failed!") };
    }
    fmDebug() << "Vault: Using encryption method:" << encrypitonMethod;

    metaObject()->invokeMethod(activeVaultFinishedWidget, "setProgressValue",
                               Qt::QueuedConnection, Q_ARG(int, 40));

    QString password { "" };
    if (encrypitonMethod == QString(kConfigValueMethodKey)) {
        fmDebug() << "Vault: Retrieving password cipher for key method";
        password = OperatorCenter::getInstance()->getSaltAndPasswordCipher();
    } else if (encrypitonMethod == QString(kConfigValueMethodTransparent)) {
        fmDebug() << "Vault: Retrieving password from keyring for transparent method";
        password = OperatorCenter::getInstance()->passwordFromKeyring();
    } else {
        fmWarning() << "Vault: Unknown encryption method:" << encrypitonMethod << ", can't create vault!";
        return { false, tr("Failed to create vault: Unknown encryption method!") };
    }

    metaObject()->invokeMethod(activeVaultFinishedWidget, "setProgressValue",
                               Qt::QueuedConnection, Q_ARG(int, 50));

    if (!password.isEmpty()) {
        fmInfo() << "Vault: Initiating vault creation with retrieved password";
        VaultHelper::instance()->createVault(password);
        OperatorCenter::getInstance()->clearSaltAndPasswordCipher();
        fmDebug() << "Vault: Cleared password cipher from memory";
    } else {
        fmWarning() << "Vault: Retrieved password is empty, failed to create the vault!";
        return { false, tr("Failed to create vault: Retrieved password is empty!") };
    }

    return { true };
}

bool VaultActiveView::handleKeyModeEncryption()
{
    {
        // save configure
        fmDebug() << "Vault: Saving key mode configuration";
        VaultConfig config;
        config.set(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant("Yes"));
        config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodKey));
    }

    auto ret = OperatorCenter::getInstance()->savePasswordAndPasswordHint(encryptInfo.password, encryptInfo.hint);
    if (!ret.result) {
        fmWarning() << "Vault: Failed to save password and hint:" << ret.message;
        activeVaultFinishedWidget->encryptFinished(false, ret.message);
        return false;
    }

    activeVaultFinishedWidget->setProgressValue(20);

    return true;
}

bool VaultActiveView::handleTransparentModeEncryption()
{
    const QString &password = OperatorCenter::getInstance()->autoGeneratePassword(kPasswordLength);
    if (password.isEmpty()) {
        fmCritical() << "Vault: auto Generate password failed!";
        activeVaultFinishedWidget->encryptFinished(false, tr("Auto generate password failed!"));
        return false;
    }

    // save password to keyring
    auto ret = OperatorCenter::getInstance()->savePasswordToKeyring(password);
    if (!ret.result) {
        fmWarning() << "Vault: Failed to save password to keyring:" << ret.message;
        activeVaultFinishedWidget->encryptFinished(false, ret.message);
        return false;
    }

    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant("Yes"));
    config.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigValueMethodTransparent));
    config.set(kConfigNodeName, kConfigKeyVersion, QVariant(kConfigVaultVersion1050));
    activeVaultFinishedWidget->setProgressValue(20);

    return true;
}
