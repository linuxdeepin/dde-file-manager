// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulthelper.h"
#include "vaultdefine.h"
#include "pathmanager.h"
#include "views/vaultcreatepage.h"
#include "views/vaultunlockpages.h"
#include "views/vaultremovepages.h"
#include "views/vaultpropertyview/vaultpropertydialog.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaultautolock.h"
#include "utils/servicemanager.h"
#include "utils/fileencrypthandle.h"
#include "events/vaulteventcaller.h"
#include "dbus/vaultdbusutils.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QApplication>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

QUrl VaultHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    QString path = "/";
    url.setPath(path);
    url.setHost("");
    return url;
}

QUrl VaultHelper::sourceRootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    QString path = PathManager::makeVaultLocalPath(QString(""), kVaultDecryptDirName);
    url.setPath(path);
    url.setHost("");
    return url;
}

QUrl VaultHelper::sourceRootUrlWithSlash()
{
    QUrl url;
    url.setScheme(scheme());
    QString path = PathManager::makeVaultLocalPath(QString(""), kVaultDecryptDirName);
    url.setPath(PathManager::addPathSlash(path));
    url.setHost("");
    return url;
}

QUrl VaultHelper::pathToVaultVirtualUrl(const QString &path)
{
    const QString &localPath = instance()->sourceRootUrl().path();
    if (path.contains(localPath)) {
        QString virtualPath = path;
        if (localPath == path)
            virtualPath = virtualPath.replace(0, localPath.length(), "/");
        else
            virtualPath = virtualPath.replace(0, localPath.length(), "");
        QUrl virtualUrl;
        virtualUrl.setPath(virtualPath);
        virtualUrl.setScheme(scheme());
        virtualUrl.setHost("");
        return virtualUrl;
    }
    return QUrl();
}

/*!
 * \brief 用于右侧栏保险箱右键菜单创建
 * \param windowId 窗口ID
 * \param url      保险箱Url
 * \param globalPos 右键菜单显示坐标
 */
void VaultHelper::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    VaultHelper::instance()->appendWinID(windowId);
    DMenu *menu = createMenu();
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(menu), AcName::kAcSidebarVaultMenu);
#endif
    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_MenuData", act->text(), urls);
    }
    delete menu;
}

/*!
 * \brief 用于右侧栏保险箱点击处理
 * \param windowId 窗口ID
 * \param url      保险箱Url
 */
void VaultHelper::siderItemClicked(quint64 windowId, const QUrl &url)
{
    QApplication::restoreOverrideCursor();
    VaultHelper::instance()->appendWinID(windowId);

    switch (instance()->state(PathManager::vaultLockPath())) {
    case VaultState::kNotExisted: {
        VaultHelper::instance()->createVaultDialog();
    } break;
    case VaultState::kEncrypted: {
        VaultHelper::instance()->unlockVaultDialog();
    } break;
    case VaultState::kUnlocked:
        instance()->defaultCdAction(windowId, url);
        recordTime(kjsonGroupName, kjsonKeyInterviewItme);
        break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
        break;
    case VaultState::kNotAvailable: {
        DialogManagerInstance->showErrorDialog(tr("Vault"), tr("Vault not available because cryfs not installed!"));
    } break;
    default:
        break;
    }
}

VaultState VaultHelper::state(const QString &baseDir) const
{
    return FileEncryptHandle::instance()->state(baseDir);
}

bool VaultHelper::updateState(VaultState curState)
{
    return FileEncryptHandle::instance()->updateState(curState);
}

void VaultHelper::defaultCdAction(const quint64 windowId, const QUrl &url)
{
    VaultEventCaller::sendItemActived(windowId, url);
}

void VaultHelper::openNewWindow(const QUrl &url)
{
    VaultEventCaller::sendOpenWindow(url);
}

bool VaultHelper::getVaultVersion() const
{
    VaultConfig config;
    QString strVersion = config.get(kConfigNodeName, kConfigKeyVersion).toString();
    if (!strVersion.isEmpty() && strVersion != kConfigVaultVersion)
        return true;

    return false;
}

void VaultHelper::killVaultTasks()
{
}

quint64 VaultHelper::currentWindowId()
{
    return currentWinID;
}

void VaultHelper::removeWinID(const quint64 &winId)
{
    if (winIDs.contains(winId)) {
        winIDs.removeOne(winId);
    }
}

bool VaultHelper::enableUnlockVault()
{
    const QVariant vRe = DConfigManager::instance()->value(kVaultDConfigName, "enableUnlockVaultInNetwork");
    if (!vRe.isValid())
        return true;

    bool bRe = vRe.toBool();
    if (bRe)
        return true;

    if (VaultDBusUtils::isFullConnectInternet())
        return false;
    return true;
}

void VaultHelper::appendWinID(const quint64 &winId)
{
    currentWinID = winId;
    if (!winIDs.contains(winId))
        winIDs.append(winId);
}

DMenu *VaultHelper::createMenu()
{
    DMenu *menu = new DMenu;
    DMenu *timeMenu = new DMenu(menu);
    switch (instance()->state(PathManager::vaultLockPath())) {
    case VaultState::kNotExisted:
        menu->addAction(QObject::tr("Create Vault"), VaultHelper::instance(), &VaultHelper::createVaultDialog);
        break;
    case VaultState::kEncrypted:
        menu->addAction(QObject::tr("Unlock"), VaultHelper::instance(), &VaultHelper::unlockVaultDialog);
        break;
    case VaultState::kUnlocked: {
        menu->addAction(QObject::tr("Open"), VaultHelper::instance(), &VaultHelper::openWindow);

        menu->addAction(QObject::tr("Open in new window"), VaultHelper::instance(), &VaultHelper::newOpenWindow);

        menu->addSeparator();

        VaultConfig config;
        QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
        if (encryptionMethod == QString(kConfigValueMethodKey) || encryptionMethod == QString(kConfigKeyNotExist)) {
            menu->addAction(QObject::tr("Lock"), []() {
                VaultHelper::instance()->lockVault(false);
            });

            QAction *timeLock = new QAction;
            timeLock->setText(QObject::tr("Auto lock"));
            VaultAutoLock::AutoLockState autoState = VaultAutoLock::instance()->getAutoLockState();
            QAction *actionNever = timeMenu->addAction(QObject::tr("Never"), []() {
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kNever);
            });
            actionNever->setCheckable(true);
            actionNever->setChecked(VaultAutoLock::AutoLockState::kNever == autoState ? true : false);
            timeMenu->addSeparator();
            QAction *actionFiveMins = timeMenu->addAction(QObject::tr("5 minutes"), []() {
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kFiveMinutes);
            });
            actionFiveMins->setCheckable(true);
            actionFiveMins->setChecked(VaultAutoLock::AutoLockState::kFiveMinutes == autoState ? true : false);
            QAction *actionTenMins = timeMenu->addAction(QObject::tr("10 minutes"), []() {
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kTenMinutes);
            });
            actionTenMins->setCheckable(true);
            actionTenMins->setChecked(VaultAutoLock::AutoLockState::kTenMinutes == autoState ? true : false);
            QAction *actionTwentyMins = timeMenu->addAction(QObject::tr("20 minutes"), []() {
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kTwentyMinutes);
            });
            actionTwentyMins->setCheckable(true);
            actionTwentyMins->setChecked(VaultAutoLock::AutoLockState::kTwentyMinutes == autoState ? true : false);
            timeLock->setMenu(timeMenu);

            menu->addMenu(timeMenu);

            menu->addSeparator();
        }

        menu->addAction(QObject::tr("Delete File Vault"), VaultHelper::instance(), &VaultHelper::showRemoveVaultDialog);

        menu->addAction(QObject::tr("Properties"), []() {
            VaultEventCaller::sendVaultProperty(VaultHelper::instance()->rootUrl());
        });
    } break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
    case VaultState::kNotAvailable:
    case VaultState::kUnknow:
        break;
    }

    return menu;
}

QWidget *VaultHelper::createVaultPropertyDialog(const QUrl &url)
{
    static VaultPropertyDialog *vaultDialog = nullptr;
    bool flg = UniversalUtils::urlEquals(VaultHelper::instance()->rootUrl(), url);
    QUrl tempUrl = VaultHelper::instance()->sourceRootUrl();
    tempUrl.setScheme(url.scheme());
    bool flg1 = UniversalUtils::urlEquals(tempUrl, url);
    if (flg || flg1) {
        if (!vaultDialog) {
            vaultDialog = new VaultPropertyDialog();
            vaultDialog->selectFileUrl(url);
            connect(vaultDialog, &VaultPropertyDialog::finished, []() { vaultDialog = nullptr; });
            return vaultDialog;
        }
        return vaultDialog;
    }
    return nullptr;
}

QUrl VaultHelper::vaultToLocalUrl(const QUrl &url)
{
    if (url.scheme() != instance()->scheme())
        return QUrl();
    if (url.path().contains(instance()->sourceRootUrl().path())) {
        QUrl localUrl = QUrl::fromLocalFile(url.path());
        return localUrl;
    } else {
        QUrl localUrl = QUrl::fromLocalFile(instance()->sourceRootUrl().path() + url.path());
        return localUrl;
    }
}

void VaultHelper::createVault(QString &password)
{
    const EncryptType &type = FileEncryptHandle::instance()->encryptAlgoTypeOfGroupPolicy();
    FileEncryptHandle::instance()->createVault(PathManager::vaultLockPath(), PathManager::vaultUnlockPath(), password, type);
}

bool VaultHelper::unlockVault(const QString &password)
{
    return FileEncryptHandle::instance()->unlockVault(PathManager::vaultLockPath(), PathManager::vaultUnlockPath(), password);
}

bool VaultHelper::lockVault(bool isForced)
{
    return FileEncryptHandle::instance()->lockVault(PathManager::vaultUnlockPath(), isForced);
}

void VaultHelper::createVaultDialog()
{
    if (QFile::exists(kVaultBasePathOld + QDir::separator() + QString(kVaultEncrypyDirName) + QDir::separator() + QString(kCryfsConfigFileName))) {
        fmCritical() << "Vault: the old vault not migrate";
        return;
    }
    VaultPageBase *page = new VaultActiveView();
    page->exec();
    if (state(PathManager::vaultLockPath()) == kNotExisted)
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Sidebar_UpdateSelection", currentWinID);
}

void VaultHelper::unlockVaultDialog()
{
    VaultConfig config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
        const QString &password = OperatorCenter::getInstance()->passwordFromKeyring();
        if (!password.isEmpty()) {
            if (unlockVault(password)) {
                VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                         VaultHelper::instance()->rootUrl());
                VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            }
        } else {
            fmWarning() << "Vault: The password from Keyring is empty!";
        }
    } else {
        VaultUnlockPages *page = new VaultUnlockPages();
        page->pageSelect(PageType::kUnlockPage);
        page->exec();
        if (state(PathManager::vaultLockPath()) != kUnlocked)
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Sidebar_UpdateSelection", currentWinID);
    }
}

void VaultHelper::showRemoveVaultDialog()
{
    VaultConfig config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (kConfigValueMethodKey == encryptionMethod || kConfigKeyNotExist == encryptionMethod) {
        VaultRemovePages *page = new VaultRemovePages(qApp->activeWindow());
        page->pageSelect(kPasswordWidget);
        page->exec();
    } else if (kConfigValueMethodTransparent == encryptionMethod) {
        VaultRemovePages *page = new VaultRemovePages(qApp->activeWindow());
        page->pageSelect(kNoneWidget);
        page->exec();
    }
}

void VaultHelper::openWindow()
{
    defaultCdAction(VaultHelper::instance()->currentWindowId(),
                    VaultHelper::instance()->rootUrl());
}

void VaultHelper::openWidWindow(quint64 winID, const QUrl &url)
{
    VaultEventCaller::sendItemActived(winID, url);
}

void VaultHelper::newOpenWindow()
{
    openNewWindow(rootUrl());
    recordTime(kjsonGroupName, kjsonKeyInterviewItme);
}

void VaultHelper::slotlockVault(int state)
{
    if (state == 0) {
        VaultAutoLock::instance()->slotLockVault(state);
        emit VaultHelper::instance()->sigLocked(state);
        QUrl url;
        url.setScheme(QString(Global::Scheme::kComputer));
        url.setPath("/");
        for (quint64 wid : winIDs) {
            defaultCdAction(wid, url);
        }
        recordTime(kjsonGroupName, kjsonKeyLockTime);
    }
}

VaultHelper *VaultHelper::instance()
{
    static VaultHelper vaultHelper;
    return &vaultHelper;
}

void VaultHelper::recordTime(const QString &group, const QString &key)
{
    Settings setting(kVaultTimeConfigFile);
    setting.setValue(group, key, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

bool VaultHelper::isVaultFile(const QUrl &url)
{
    if (url.scheme() == VaultHelper::instance()->scheme()
        || url.path().startsWith(PathManager::makeVaultLocalPath(QString(""), kVaultDecryptDirName)))
        return true;

    return false;
}

bool VaultHelper::urlsToLocal(const QList<QUrl> &origins, QList<QUrl> *urls)
{
    if (!urls)
        return false;
    for (const QUrl &url : origins) {
        if (!isVaultFile(url))
            return false;
        if (url.scheme() == VaultHelper::instance()->scheme())
            (*urls).push_back(vaultToLocalUrl(url));
        else
            (*urls).push_back(url);
    }
    return true;
}

void VaultHelper::showInProgressDailog(QString msg)
{
    //期间有拷贝，压缩任务时，提示不可上锁
    if (msg.contains("Device or resource busy")) {
        DialogManagerInstance->showErrorDialog(tr("Vault"), tr("A task is in progress, so it cannot perform your operation"));
    }
}

VaultHelper::VaultHelper()
{
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalCreateVault, this, &VaultHelper::sigCreateVault);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalUnlockVault, this, &VaultHelper::sigUnlocked);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalLockVault, this, &VaultHelper::slotlockVault);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalReadError, this, &VaultHelper::showInProgressDailog);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalReadOutput, this, &VaultHelper::showInProgressDailog);
}
