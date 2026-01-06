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
#include "views/resetpasswordview/vaultresetpasswordpages.h"
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
    fmWarning() << "Vault: Path does not contain local path, returning empty URL";
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
    fmDebug() << "Vault: Handling context menu for window ID:" << windowId << "URL:" << url.toString() << "Position:" << globalPos;

    VaultHelper::instance()->appendWinID(windowId);
    DMenu *menu = createMenu();
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(menu), AcName::kAcSidebarVaultMenu);
#endif

    fmDebug() << "Vault: Executing context menu";
    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        fmInfo() << "Vault: Context menu action triggered:" << act->text();
        dpfSignalDispatcher->publish("dfmplugin_vault", "signal_ReportLog_MenuData", act->text(), urls);
    } else {
        fmDebug() << "Vault: Context menu closed without action selection";
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

    switch (instance()->state(PathManager::vaultLockPath(), false)) {
    case VaultState::kNotExisted: {
        fmInfo() << "Vault: Vault not existed, showing create dialog";
        VaultHelper::instance()->createVaultDialog();
    } break;
    case VaultState::kEncrypted: {
        fmInfo() << "Vault: Vault encrypted, showing unlock dialog";
        VaultHelper::instance()->unlockVaultDialog();
    } break;
    case VaultState::kUnlocked:
        fmInfo() << "Vault: Vault unlocked, performing default action";
        instance()->defaultCdAction(windowId, url);
        recordTime(kjsonGroupName, kjsonKeyInterviewItme);
        break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
        break;
    case VaultState::kNotAvailable: {
        fmWarning() << "Vault: Vault not available - cryfs not installed";
        DialogManagerInstance->showErrorDialog(tr("Vault"), tr("Vault not available because cryfs not installed!"));
    } break;
    default:
        break;
    }
}

VaultState VaultHelper::state(const QString &baseDir, bool useCache) const
{
    return FileEncryptHandle::instance()->state(baseDir, useCache);
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
    fmDebug() << "Vault: Checking vault version";

    VaultConfig config;
    QString strVersion = config.get(kConfigNodeName, kConfigKeyVersion).toString();
    fmDebug() << "Vault: Current version:" << strVersion << "Expected:" << kConfigVaultVersion;

    if (!strVersion.isEmpty() && strVersion != kConfigVaultVersion) {
        fmInfo() << "Vault: Version mismatch detected";
        return true;
    }

    fmDebug() << "Vault: Version check passed";
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
    fmDebug() << "Vault: Checking if vault unlock is enabled";

    const QVariant vRe = DConfigManager::instance()->value(kVaultDConfigName, "enableUnlockVaultInNetwork");
    if (!vRe.isValid()) {
        fmDebug() << "Vault: Config not found, enabling unlock";
        return true;
    }

    bool bRe = vRe.toBool();
    fmDebug() << "Vault: Config value for enableUnlockVaultInNetwork:" << bRe;

    if (bRe) {
        fmDebug() << "Vault: Unlock enabled in config";
        return true;
    }

    bool isConnected = VaultDBusUtils::isFullConnectInternet();
    fmDebug() << "Vault: Internet connection status:" << isConnected;

    if (isConnected) {
        fmWarning() << "Vault: Unlock disabled due to internet connection";
        return false;
    }

    fmDebug() << "Vault: Unlock enabled (no internet connection)";
    return true;
}

void VaultHelper::appendWinID(const quint64 &winId)
{
    fmDebug() << "Vault: Adding window ID:" << winId;
    currentWinID = winId;
    if (!winIDs.contains(winId))
        winIDs.append(winId);
}

DMenu *VaultHelper::createMenu()
{
    fmDebug() << "Vault: Creating context menu";

    DMenu *menu = new DMenu;
    DMenu *timeMenu = new DMenu(menu);
    switch (instance()->state(PathManager::vaultLockPath())) {
    case VaultState::kNotExisted:
        fmDebug() << "Vault: Adding 'Create Vault' menu item";
        menu->addAction(QObject::tr("Create Vault"), VaultHelper::instance(), &VaultHelper::createVaultDialog);
        break;
    case VaultState::kEncrypted:
        fmDebug() << "Vault: Adding 'Unlock' menu item";
        menu->addAction(QObject::tr("Unlock"), VaultHelper::instance(), &VaultHelper::unlockVaultDialog);
        menu->addSeparator();
        if (OperatorCenter::getInstance()->isNewVaultVersion()) {
            VaultConfig config;
            QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
            // 重置密码功能仅在新版本且非透明加密时提供
            if (encryptionMethod != QString(kConfigValueMethodTransparent)) {
                menu->addAction(QObject::tr("Reset Password"), VaultHelper::instance(), &VaultHelper::showResetPasswordDialog);
            }
        }
        break;
    case VaultState::kUnlocked: {
        fmDebug() << "Vault: Adding unlocked state menu items";

        menu->addAction(QObject::tr("Open"), VaultHelper::instance(), &VaultHelper::openWindow);
        menu->addAction(QObject::tr("Open in new window"), VaultHelper::instance(), &VaultHelper::newOpenWindow);
        menu->addSeparator();

        VaultConfig config;
        QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
        fmDebug() << "Vault: Encryption method:" << encryptionMethod;

        if (encryptionMethod == QString(kConfigValueMethodKey) || encryptionMethod == QString(kConfigKeyNotExist)) {
            fmDebug() << "Vault: Adding key-based encryption menu items";

            menu->addAction(QObject::tr("Lock"), []() {
                fmInfo() << "Vault: Manual lock action triggered";
                VaultHelper::instance()->lockVault(false);
            });

            QAction *timeLock = new QAction;
            timeLock->setText(QObject::tr("Auto lock"));
            VaultAutoLock::AutoLockState autoState = VaultAutoLock::instance()->getAutoLockState();
            fmDebug() << "Vault: Current auto-lock state:" << static_cast<int>(autoState);

            QAction *actionNever = timeMenu->addAction(QObject::tr("Never"), []() {
                fmInfo() << "Vault: Setting auto-lock to Never";
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kNever);
            });
            actionNever->setCheckable(true);
            actionNever->setChecked(VaultAutoLock::AutoLockState::kNever == autoState ? true : false);

            timeMenu->addSeparator();

            QAction *actionFiveMins = timeMenu->addAction(QObject::tr("5 minutes"), []() {
                fmInfo() << "Vault: Setting auto-lock to 5 minutes";
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kFiveMinutes);
            });
            actionFiveMins->setCheckable(true);
            actionFiveMins->setChecked(VaultAutoLock::AutoLockState::kFiveMinutes == autoState ? true : false);

            QAction *actionTenMins = timeMenu->addAction(QObject::tr("10 minutes"), []() {
                fmInfo() << "Vault: Setting auto-lock to 10 minutes";
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kTenMinutes);
            });
            actionTenMins->setCheckable(true);
            actionTenMins->setChecked(VaultAutoLock::AutoLockState::kTenMinutes == autoState ? true : false);

            QAction *actionTwentyMins = timeMenu->addAction(QObject::tr("20 minutes"), []() {
                fmInfo() << "Vault: Setting auto-lock to 20 minutes";
                VaultAutoLock::instance()->autoLock(VaultAutoLock::AutoLockState::kTwentyMinutes);
            });
            actionTwentyMins->setCheckable(true);
            actionTwentyMins->setChecked(VaultAutoLock::AutoLockState::kTwentyMinutes == autoState ? true : false);

            timeLock->setMenu(timeMenu);
            menu->addMenu(timeMenu);
            menu->addSeparator();

            fmDebug() << "Vault: Auto-lock menu items added";
        }

        // 重置密码功能仅在新版本且非透明加密时提供
        if (OperatorCenter::getInstance()->isNewVaultVersion()
            && encryptionMethod != QString(kConfigValueMethodTransparent)) {
            menu->addAction(QObject::tr("Reset Password"), VaultHelper::instance(), &VaultHelper::showResetPasswordDialog);
        }
        menu->addAction(QObject::tr("Delete File Vault"), VaultHelper::instance(), &VaultHelper::showRemoveVaultDialog);
        menu->addAction(QObject::tr("Properties"), []() {
            fmInfo() << "Vault: Properties action triggered";
            VaultEventCaller::sendVaultProperty(VaultHelper::instance()->rootUrl());
        });
    } break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
    case VaultState::kNotAvailable:
    case VaultState::kUnknow:
        break;
    }

    fmDebug() << "Vault: Menu creation completed";
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
            fmDebug() << "Vault: Creating new property dialog";
            vaultDialog = new VaultPropertyDialog();
            vaultDialog->selectFileUrl(url);
            connect(vaultDialog, &VaultPropertyDialog::finished, []() { vaultDialog = nullptr; });
            return vaultDialog;
        }
        fmDebug() << "Vault: Returning existing property dialog";
        return vaultDialog;
    }
    return nullptr;
}

QUrl VaultHelper::vaultToLocalUrl(const QUrl &url)
{
    if (url.scheme() != instance()->scheme()) {
        if (url.scheme() == Global::Scheme::kFile)
            return url;
        fmWarning() << "Vault: URL scheme mismatch, expected:" << instance()->scheme() << "got:" << url.scheme();
        return QUrl();
    }

    if (url.path().contains(instance()->sourceRootUrl().path())) {
        QUrl localUrl = QUrl::fromLocalFile(url.path());
        fmDebug() << "Vault: Direct path conversion";
        return localUrl;
    } else {
        QUrl localUrl = QUrl::fromLocalFile(instance()->sourceRootUrl().path() + url.path());
        fmDebug() << "Vault: Path concatenation";
        return localUrl;
    }
}

void VaultHelper::createVault(QString &password)
{
    fmDebug() << "Vault: Starting vault creation process";
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
    fmDebug() << "Vault: Showing create vault dialog";
    QString oldVaultPath = kVaultBasePathOld + QDir::separator() + QString(kVaultEncrypyDirName) + QDir::separator() + QString(kCryfsConfigFileName);
    if (QFile::exists(oldVaultPath)) {
        fmCritical() << "Vault: the old vault not migrate";
        return;
    }
    VaultPageBase *page = new VaultActiveView();
    page->exec();

    if (state(PathManager::vaultLockPath()) == kNotExisted) {
        fmDebug() << "Vault: Vault still not existed after dialog, updating sidebar selection";
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Sidebar_UpdateSelection", currentWinID);
    }
}

void VaultHelper::unlockVaultDialog()
{
    fmDebug() << "Vault: Showing unlock vault dialog";

    VaultConfig config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    fmDebug() << "Vault: Encryption method:" << encryptionMethod;

    if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
        fmDebug() << "Vault: Using transparent encryption method";

        const QString &password = OperatorCenter::getInstance()->passwordFromKeyring();
        if (!password.isEmpty()) {
            fmDebug() << "Vault: Password retrieved from keyring, attempting unlock";

            if (unlockVault(password)) {
                fmInfo() << "Vault: Automatic unlock successful";
                VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                         VaultHelper::instance()->rootUrl());
                VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            } else {
                fmWarning() << "Vault: Automatic unlock failed";
            }
        } else {
            fmWarning() << "Vault: The password from Keyring is empty!";
        }
    } else {
        fmDebug() << "Vault: Using key-based encryption method, showing unlock pages";
        // 非透明加密方式下，如果当前是老版本保险箱，优先引导用户执行升级/迁移
        bool isNewVersion = OperatorCenter::getInstance()->isNewVaultVersion();
        if (!isNewVersion) {
            DDialog upgradeDialog;
            upgradeDialog.setTitle(QObject::tr("Upgrade File Vault"));

            // 自定义内容：在标题和文本之间插入一张图片
            QFrame *contentFrame = new QFrame(&upgradeDialog);
            QVBoxLayout *layout = new QVBoxLayout(contentFrame);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(12);

            QLabel *iconLabel = new QLabel(contentFrame);
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setPixmap(QIcon::fromTheme("dfm_vault_upgrade").pixmap(64, 64));

            QLabel *textLabel = new QLabel(contentFrame);
            textLabel->setAlignment(Qt::AlignCenter);
            textLabel->setWordWrap(true);
            textLabel->setText(QObject::tr("The file vault encryption scheme has been upgraded.\n"
                                           "You need to upgrade this vault to continue using it."));

            layout->addWidget(iconLabel);
            layout->addWidget(textLabel);

            upgradeDialog.setMessage("");
            upgradeDialog.addContent(contentFrame);

            upgradeDialog.addButton(QObject::tr("Later"), true, DDialog::ButtonNormal);
            upgradeDialog.addButton(QObject::tr("Upgrade now"), true, DDialog::ButtonRecommend);

            int ret = upgradeDialog.exec();
            // 用户选择“稍后处理”或关闭对话框，直接返回，不解锁保险箱
            if (ret != 1) {
                return;
            }

            // 用户选择"立即升级"，打开解锁页面，标记为老密码方案迁移模式
            VaultUnlockPages *page = new VaultUnlockPages();
            page->setOldPasswordSchemeMigrationMode(true);
            page->pageSelect(PageType::kUnlockPage);
            page->exec();
            page->deleteLater();
            return;
        }

        VaultUnlockPages *page = new VaultUnlockPages();
        page->pageSelect(PageType::kUnlockPage);
        page->exec();

        if (state(PathManager::vaultLockPath()) != kUnlocked) {
            fmDebug() << "Vault: Vault not unlocked after dialog, updating sidebar selection";
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Sidebar_UpdateSelection", currentWinID);
        }
    }
}

void VaultHelper::showRemoveVaultDialog()
{
    fmDebug() << "Vault: Showing remove vault dialog";

    VaultConfig config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    fmDebug() << "Vault: Encryption method for removal:" << encryptionMethod;

    if (kConfigValueMethodKey == encryptionMethod || kConfigKeyNotExist == encryptionMethod) {
        fmDebug() << "Vault: Showing password-based removal dialog";

        VaultRemovePages *page = new VaultRemovePages(qApp->activeWindow());
        page->pageSelect(kPasswordWidget);
        page->exec();
    } else if (kConfigValueMethodTransparent == encryptionMethod) {
        fmDebug() << "Vault: Showing transparent removal dialog";

        VaultRemovePages *page = new VaultRemovePages(qApp->activeWindow());
        page->pageSelect(kNoneWidget);
        page->exec();
    }
}

void VaultHelper::showResetPasswordDialog()
{
    VaultResetPasswordPages *page = new VaultResetPasswordPages();
    page->switchToOldPasswordView();
    page->exec();
}

void VaultHelper::openWindow()
{
    defaultCdAction(VaultHelper::instance()->currentWindowId(),
                    VaultHelper::instance()->rootUrl());
}

void VaultHelper::openWidWindow(quint64 winID, const QUrl &url)
{
    fmDebug() << "Vault: Opening vault window, windowId:" << winID << "url:" << url.toString();
    VaultEventCaller::sendItemActived(winID, url);
}

void VaultHelper::newOpenWindow()
{
    openNewWindow(rootUrl());
    recordTime(kjsonGroupName, kjsonKeyInterviewItme);
}

void VaultHelper::slotlockVault(int state)
{
    fmDebug() << "Vault: Lock vault slot called with state:" << state;
    if (state == 0) {
        fmDebug() << "Vault: Vault locked successfully, notifying components";

        VaultAutoLock::instance()->slotLockVault(state);
        emit VaultHelper::instance()->sigLocked(state);
        QUrl url;
        url.setScheme(QString(Global::Scheme::kComputer));
        url.setPath("/");
        for (quint64 wid : winIDs) {
            defaultCdAction(wid, url);
        }
        recordTime(kjsonGroupName, kjsonKeyLockTime);
    } else {
        fmWarning() << "Vault: Lock operation failed with state:" << state;
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
    if (!urls) {
        fmWarning() << "Vault: Output URL list is null";
        return false;
    }

    for (const QUrl &url : origins) {
        if (!isVaultFile(url)) {
            fmWarning() << "Vault: URL is not a vault file:" << url.toString();
            return false;
        }

        if (url.scheme() == VaultHelper::instance()->scheme())
            (*urls).push_back(vaultToLocalUrl(url));
        else
            (*urls).push_back(url);
    }

    return true;
}

void VaultHelper::showInProgressDailog(QString msg)
{
    // 期间有拷贝，压缩任务时，提示不可上锁
    if (msg.contains("Device or resource busy")) {
        fmWarning() << "Vault: Device busy error detected";
        DialogManagerInstance->showErrorDialog(tr("Vault"), tr("A task is in progress, so it cannot perform your operation"));
    }
}

VaultHelper::VaultHelper()
{
    fmDebug() << "Vault: Initializing VaultHelper";

    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalCreateVault, this, &VaultHelper::sigCreateVault);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalUnlockVault, this, &VaultHelper::sigUnlocked);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalLockVault, this, &VaultHelper::slotlockVault);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalReadError, this, &VaultHelper::showInProgressDailog);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalReadOutput, this, &VaultHelper::showInProgressDailog);

    fmDebug() << "Vault: VaultHelper initialization completed";
}
