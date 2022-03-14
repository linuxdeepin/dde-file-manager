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

#include "vaulthelper.h"
#include "vaultglobaldefine.h"
#include "views/vaultactiveview.h"
#include "views/vaultunlockpages.h"
#include "views/vaultremovepages.h"
#include "views/vaultpropertydialog.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/vaultautolock.h"
#include "events/vaulteventcaller.h"
#include "vaultdbusutils.h"

#include "dfm-base/base/urlroute.h"

#include <dfm-framework/framework.h>

#include <DSysInfo>

#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QApplication>

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DCORE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

quint64 VaultHelper::winID = 0;

QUrl VaultHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath(makeVaultLocalPath(QString(""), kVaultDecryptDirName));
    url.setHost("");
    return url;
}

/*!
 * \brief 用于右侧栏保险箱右键菜单创建
 * \param windowId 窗口ID
 * \param url      保险箱Url
 * \param globalPos 右键菜单显示坐标
 */
void VaultHelper::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    winID = windowId;
    QMenu *menu = createMenu();
    menu->exec(globalPos);
    delete menu;
}

/*!
 * \brief 用于右侧栏保险箱点击处理
 * \param windowId 窗口ID
 * \param url      保险箱Url
 */
void VaultHelper::siderItemClicked(quint64 windowId, const QUrl &url)
{
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    winID = windowId;
    switch (instance()->state(instance()->vaultLockPath())) {
    case VaultState::kNotExisted: {
        VaultPageBase *page = new VaultActiveView();
        page->exec();
    } break;
    case VaultState::kEncrypted: {
        VaultPageBase *page = new VaultUnlockPages();
        page->exec();
    } break;
    case VaultState::kUnlocked:
        instance()->defaultCdAction(url);
        break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
    case VaultState::kNotAvailable:
        break;
    }
}

bool VaultHelper::isVaultEnabled()
{
    if (!DSysInfo::isCommunityEdition()) {   // 如果不是社区版
        DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
        // 如果是专业版
        if (DSysInfo::DeepinType::DeepinProfessional == deepinType /* && VaultController::ins()->isVaultVisiable()*/) {
            return true;
        }
    }
    return false;
}

VaultState VaultHelper::state(QString lockBaseDir)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        // 记录保险箱状态
        return VaultState::kNotAvailable;
    }

    if (lockBaseDir.isEmpty()) {
        lockBaseDir = vaultLockPath() + "cryfs.config";
    } else {
        if (lockBaseDir.endsWith("/"))
            lockBaseDir += "cryfs.config";
        else
            lockBaseDir += "/cryfs.config";
    }

    if (QFile::exists(lockBaseDir)) {
        QStorageInfo info(makeVaultLocalPath());
        QString temp = info.fileSystemType();
        if (info.isValid() && temp == "fuse.cryfs") {
            return VaultState::kUnlocked;
        }
        return VaultState::kEncrypted;
    } else {
        return VaultState::kNotExisted;
    }
}

QString VaultHelper::makeVaultLocalPath(QString path, QString base)
{
    if (base.isEmpty()) {
        base = kVaultDecryptDirName;
    }
    return kVaultBasePath + QDir::separator() + base + (path.startsWith('/') ? "" : "/") + path;
}

QList<TitleBar::CrumbData> VaultHelper::seprateUrl(const QUrl &url)
{
    QList<TitleBar::CrumbData> list;
    QList<QUrl> urls;
    QUrl tempUrl = UrlRoute::pathToReal(url.path());
    tempUrl.setScheme(url.scheme());
    urls.push_back(tempUrl);
    UrlRoute::urlParentList(tempUrl, &urls);

    for (int count = urls.size() - 1; count >= 0; count--) {
        QUrl curUrl { urls.at(count) };
        QStringList pathList { curUrl.path().split("/") };
        TitleBar::CrumbData data { curUrl, pathList.isEmpty() ? "" : pathList.last() };
        if (UrlRoute::isRootUrl(curUrl))
            data.iconName = UrlRoute::icon(curUrl.scheme()).name();
        list.append(data);
    }

    return list;
}

VaultService *VaultHelper::vaultServiceInstance()
{
    static VaultService *vaultService = nullptr;
    if (vaultService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(VaultService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        vaultService = ctx.service<VaultService>(VaultService::name());
        if (!vaultService) {
            qCritical() << "Failed, init vault \"sideBarService\" is empty";
            abort();
        }
    }
    return vaultService;
}

SideBarService *VaultHelper::sideBarServiceInstance()
{
    static SideBarService *sideBarService = nullptr;
    if (sideBarService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(SideBarService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        sideBarService = ctx.service<SideBarService>(SideBarService::name());
        if (!sideBarService) {
            qCritical() << "Failed, init sidebar \"sideBarService\" is empty";
            abort();
        }
    }
    return sideBarService;
}

WindowsService *VaultHelper::windowServiceInstance()
{
    static WindowsService *windowsService = nullptr;
    if (windowsService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(WindowsService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        windowsService = ctx.service<WindowsService>(WindowsService::name());
        if (!windowsService) {
            qCritical() << "Failed, init windows \"sideBarService\" is empty";
            abort();
        }
    }
    return windowsService;
}

ComputerService *VaultHelper::computerServiceInstance()
{
    static ComputerService *computerService = nullptr;
    if (computerService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(ComputerService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        computerService = ctx.service<ComputerService>(ComputerService::name());
        if (!computerService) {
            qCritical() << "Failed, init computer \"computerService\" is empty";
            abort();
        }
    }
    return computerService;
}

TitleBarService *VaultHelper::titleBarServiceInstance()
{
    static TitleBarService *titleBarService = nullptr;
    if (titleBarService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(TitleBarService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        titleBarService = ctx.service<TitleBarService>(TitleBarService::name());
        if (!titleBarService) {
            qCritical() << "Failed, init titlebar \"titleBarService\" is empty";
            abort();
        }
    }
    return titleBarService;
}

WorkspaceService *VaultHelper::workspaceServiceInstance()
{
    static WorkspaceService *workspaceService = nullptr;
    if (workspaceService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(WorkspaceService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
        if (!workspaceService) {
            qCritical() << "Failed, init workspace \"workspaceService\" is empty";
            abort();
        }
    }
    return workspaceService;
}

void VaultHelper::defaultCdAction(const QUrl &url)
{
    VaultEventCaller::sendItemActived(winID, url);
}

void VaultHelper::openNewWindow(const QUrl &url)
{
    VaultEventCaller::sendOpenWindow(url);
}

bool VaultHelper::getVaultVersion()
{
    VaultConfig config;
    QString strVersion = config.get(kConfigNodeName, kConfigKeyVersion).toString();
    if (!strVersion.isEmpty() && strVersion != kConfigVaultVersion)
        return true;

    return false;
}

QString VaultHelper::vaultLockPath()
{
    return makeVaultLocalPath("", kVaultEncrypyDirName);
}

QString VaultHelper::vaultUnlockPath()
{
    return VaultHelper::makeVaultLocalPath("", kVaultDecryptDirName);
}

int VaultHelper::getVaultPolicy()
{
    return VaultDBusUtils::getVaultPolicy();
}

bool VaultHelper::setVaultPolicyState(int policyState)
{
    return VaultDBusUtils::setVaultPolicyState(policyState);
}

VaultHelper::VaultPageMark VaultHelper::getVaultCurrentPageMark()
{
    return recordVaultPageMark;
}

void VaultHelper::setVauleCurrentPageMark(VaultHelper::VaultPageMark mark)
{
    recordVaultPageMark = mark;
}

bool VaultHelper::isVaultVisiable()
{
    return vaultVisiable;
}

void VaultHelper::removeSideBarVaultItem()
{
    sideBarServiceInstance()->removeItem(rootUrl());
}

void VaultHelper::removeComputerVaultItem()
{
    computerServiceInstance()->removeDevice(QUrl("entry:///vault.vault"));
}

void VaultHelper::killVaultTasks()
{
}

QMenu *VaultHelper::createMenu()
{
    QMenu *menu = new QMenu;
    QMenu *timeMenu = new QMenu;
    switch (instance()->state(instance()->vaultLockPath())) {
    case VaultState::kNotExisted:
        menu->addAction(QObject::tr("Create Vault"), VaultHelper::instance(), &VaultHelper::creatVaultDialog);
        break;
    case VaultState::kEncrypted:
        menu->addAction(QObject::tr("Unlock Vault"), VaultHelper::instance(), &VaultHelper::unlockVaultDialog);
        break;
    case VaultState::kUnlocked: {
        menu->addAction(QObject::tr("Open window"), VaultHelper::instance(), &VaultHelper::openWindow);

        menu->addAction(QObject::tr("Open in new window"), VaultHelper::instance(), &VaultHelper::newOpenWindow);

        menu->addSeparator();

        menu->addAction(QObject::tr("Lock Now"), VaultHelper::instance(), &VaultHelper::lockVault);

        QAction *timeLock = new QAction;
        timeLock->setText(QObject::tr("Time Lock"));
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

        menu->addAction(QObject::tr("Delete File Vault"), VaultHelper::instance(), &VaultHelper::removeVaultDialog);

        menu->addAction(QObject::tr("Property"), []() {
            QUrl url;
            url.setScheme(instance()->scheme());
            url.setPath("/");
            url.setHost("");
            VaultEventCaller::sendVaultProperty(url);
        });
    } break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
    case VaultState::kNotAvailable:
        break;
    }

    return menu;
}

QWidget *VaultHelper::createVaultPropertyDialog(const QUrl &url)
{
    static VaultPropertyDialog *vaultDialog = nullptr;
    QUrl rUrl = VaultHelper::instance()->rootUrl();
    bool flg = (rUrl == url);
    if ((UrlRoute::isRootUrl(url) || flg) && !vaultDialog) {
        vaultDialog = new VaultPropertyDialog();
        vaultDialog->selectFileUrl(url);
        return vaultDialog;
    } else if ((UrlRoute::isRootUrl(url) || flg) && vaultDialog) {
        return vaultDialog;
    } else {
        return nullptr;
    }
}

void VaultHelper::createVault(QString &password)
{
    static bool flg = true;
    if (flg) {
        connect(vaultServiceInstance(), &VaultService::signalCreateVaultState, VaultHelper::instance(), &VaultHelper::sigCreateVault);
        flg = false;
    }
    vaultServiceInstance()->createVault(vaultLockPath(), vaultUnlockPath(), password);
}

void VaultHelper::unlockVault(QString &password)
{
    static bool flg = true;
    if (flg) {
        connect(vaultServiceInstance(), &VaultService::signalUnlockVaultState, VaultHelper::instance(), &VaultHelper::sigUnlocked);
        flg = false;
    }
    vaultServiceInstance()->unlockVault(vaultLockPath(), vaultUnlockPath(), password);
}

void VaultHelper::lockVault()
{
    static bool flg = true;
    if (flg) {
        connect(vaultServiceInstance(), &VaultService::signalLockVaultState, VaultHelper::instance(), &VaultHelper::slotlockVault);
        flg = false;
    }
    vaultServiceInstance()->lockVault(vaultUnlockPath());
}

void VaultHelper::creatVaultDialog()
{
    VaultPageBase *page = new VaultActiveView();
    page->exec();
}

void VaultHelper::unlockVaultDialog()
{
    VaultPageBase *page = new VaultUnlockPages();
    page->exec();
}

void VaultHelper::removeVaultDialog()
{
    VaultPageBase *page = new VaultRemovePages();
    page->exec();
}

void VaultHelper::openWindow()
{
    QUrl url;
    url.setScheme(VaultHelper::scheme());
    url.setPath("/");
    url.setHost("");
    defaultCdAction(rootUrl());
}

void VaultHelper::openWidWindow(quint64 winID, const QUrl &url)
{
    this->winID = winID;
    VaultEventCaller::sendItemActived(winID, url);
}

void VaultHelper::newOpenWindow()
{
    QUrl url;
    url.setScheme(VaultHelper::scheme());
    url.setPath("/");
    url.setHost("");
    openNewWindow(rootUrl());
}

void VaultHelper::slotVaultPolicy()
{
    switch (getVaultPolicy()) {
    case 1: {
        switch (getVaultCurrentPageMark()) {
        case VaultPageMark::kUnknown:
            break;
        case VaultPageMark::kCreateVaultPage:
            emit sigCloseWindow();
            break;
        case VaultPageMark::kRetrievePassWordPage:
            emit sigCloseWindow();
            break;
        case VaultPageMark::kVaultPage:
            emit sigCloseWindow();
            break;
        case VaultPageMark::kClipboardPage:
            if (vaultVisiable) {
                lockVault();
                vaultVisiable = false;
                VaultHelper::instance()->removeSideBarVaultItem();
                VaultHelper::instance()->removeComputerVaultItem();
                VaultHelper::killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::kCopyFilePage:
            if (vaultVisiable) {
                lockVault();
                vaultVisiable = false;
                VaultHelper::instance()->removeSideBarVaultItem();
                VaultHelper::instance()->removeComputerVaultItem();
                VaultHelper::killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::kCreateVaultPage1:
        case VaultPageMark::kUnlockVaultPage:
        case VaultPageMark::kDeleteFilePage:
        case VaultPageMark::kDeleteVaultPage:
            setVaultPolicyState(2);
            return;
        }

        lockVault();
        vaultVisiable = false;
        VaultHelper::instance()->removeSideBarVaultItem();
        VaultHelper::instance()->removeComputerVaultItem();

    } break;
    case 2:
        if (!vaultVisiable) {
            vaultVisiable = true;
            VaultHelper::instance()->removeSideBarVaultItem();
            VaultHelper::instance()->removeComputerVaultItem();
        }
        break;
    }
}

void VaultHelper::slotlockVault(int state)
{
    if (state == 0) {
        VaultAutoLock::instance()->slotLockVault(state);
        emit VaultHelper::instance()->sigLocked(state);
        QUrl url;
        url.setScheme(QString(Global::kComputer));
        url.setPath("/");
        defaultCdAction(url);
    }
}

VaultHelper *VaultHelper::instance()
{
    static VaultHelper vaultHelper;
    return &vaultHelper;
}

VaultHelper::VaultHelper()
{
}
