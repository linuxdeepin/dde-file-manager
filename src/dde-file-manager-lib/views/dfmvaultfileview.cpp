// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmvaultfileview.h"
#include "controllers/vaultcontroller.h"
#include "controllers/vaulterrorcode.h"
#include "vault/vaultconfig.h"
#include "vault/operatorcenter.h"
#include "vault/vaultdbusresponse.h"
#include "vault/vaultlockmanager.h"
#include "vault/vaultconfig.h"
#include "vault/operatorcenter.h"
#include "vault/vaultdbusresponse.h"
#include "dfmsettings.h"
#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultremovepages.h"
#include "views/dfmvaultactiveview.h"
#include "views/dfilemanagerwindow.h"
#include "dfilesystemmodel.h"
#include "vaultglobaldefine.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "singleton.h"
#include "dialogs/dtaskdialog.h"
#include "dialogs/dialogmanager.h"

#include <QDrag>

DFMVaultFileView::DFMVaultFileView(QWidget *parent)
    : DFileView(parent)
{
    connect(VaultController::ins(), &VaultController::signalLockVault, this, &DFMVaultFileView::onLeaveVault);
    connect(fileSignalManager, &FileSignalManager::requestIgnoreDragEvent, this, &DFMVaultFileView::IgnoreDragEvent);
}

bool DFMVaultFileView::setRootUrl(const DUrl &url)
{
    if(!VaultController::ins()->isVaultVisiable()) {
        DDialog dialog(tr("Hint"), tr("Cannot open this path"), this);
        dialog.addButton(tr("OK"));
        dialog.exec();
        return false;
    }

    VaultController::VaultState enState = VaultController::ins()->state();

    QWidget *wndPtr = widget()->topLevelWidget();
    DFMVaultPageBase *page = nullptr;
    if (enState != VaultController::Unlocked) {
        switch (enState) {
        case VaultController::NotAvailable: {
            qWarning() << "Vault: cryfs not installed!";
            break;
        }
        case VaultController::NotExisted: {
            page = qobject_cast<DFMVaultPageBase *>(new DFMVaultActiveView(wndPtr));
            break;
        }
        case VaultController::Encrypted: {
            // 如果是透明加密方式，直接开锁
            VaultConfig config;
            QString encryptionMethod = config.get(CONFIG_NODE_NAME, CONFIG_KEY_ENCRYPTION_METHOD, QVariant("NoExist")).toString();
            if (encryptionMethod == CONFIG_METHOD_VALUE_TRANSPARENT) {
                if (!VaultDbusResponse::instance()->transparentUnlockVault())
                    return false;
            } else {
                if (url.host() == "certificate") {
                    page = qobject_cast<DFMVaultRecoveryKeyPages *>(new DFMVaultRecoveryKeyPages(wndPtr));
                } else {
                    page = qobject_cast<DFMVaultUnlockPages *>(new DFMVaultUnlockPages(wndPtr));
                }
            }
            break;
        }
        default:
            break;
        }
    } else {
        if (url.host() == "delete") {
            DTaskDialog *pTaskDlg = dialogManager->taskDialog();
            if (pTaskDlg) {
                if (pTaskDlg->haveNotCompletedVaultTask()) {
                    dialogManager->showErrorDialog("", tr("A task is in progress, and you cannot delete the vault"));
                    return false;
                }
            }
            page = qobject_cast<DFMVaultRemovePages *>(new DFMVaultRemovePages(wndPtr));
        }

        //! 记录访问保险箱时间
        if (VaultController::isRootDirectory(url.toLocalFile())) {
            VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::VAULTPAGE);
            DFM_NAMESPACE::DFMSettings setting(VAULT_TIME_CONFIG_FILE);
            setting.setValue(QString("VaultTime"), QString("InterviewTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        }
    }

    if (page) {
        page->setWindowFlag(Qt::Dialog);
        page->setWndPtr(wndPtr);
        page->showTop();
        return false;
    }

    return DFileView::setRootUrl(url);
}

void DFMVaultFileView::onLeaveVault(int state)
{
    if (state == 0 || state == 1 || state == static_cast<int>(ErrorCode::MountdirEncrypted)) {
        this->cd(DUrl(COMPUTER_ROOT));
        VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::UNKNOWN);
    }
}

void DFMVaultFileView::IgnoreDragEvent()
{
    QDrag::cancel();
}

bool DFMVaultFileView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::HoverMove
            || event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::KeyRelease) {

        VaultLockManager::getInstance().refreshAccessTime();
#ifdef AUTOLOCK_TEST
        qDebug() << "type == " << event->type();
#endif
    }

    return DFileView::eventFilter(obj, event);
}

void DFMVaultFileView::onRootUrlDeleted(const DUrl &rootUrl)
{
    // 修复bug-77747 保险箱上锁情况下会跳转到计算机页面，所以这里不用执行跳转
    if (VaultController::ins()->state() == VaultController::Encrypted)
        return;

    DFileView::onRootUrlDeleted(rootUrl);
}
