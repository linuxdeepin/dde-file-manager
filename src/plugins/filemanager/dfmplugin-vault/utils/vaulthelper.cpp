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
#include "dfm-base/base/urlroute.h"
#include "views/vaultactiveview.h"
#include "views/vaultunlockpages.h"
#include "views/vaultremovepages.h"
#include "utils/encryption/vaultconfig.h"
#include "events/vaulteventcaller.h"

#include <dfm-framework/framework.h>

#include <DSysInfo>

#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QStandardPaths>
#include <QStorageInfo>

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DCORE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

bool VaultHelper::isModel = false;

quint64 VaultHelper::winID = 0;

QUrl VaultHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath(makeVaultLocalPath(QString(""), kVaultDecryptDirName));
    url.setHost("");
    return url;
}

void VaultHelper::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    winID = windowId;
    QMenu *menu = createMenu();
    menu->exec(globalPos);
    delete menu;
}

void VaultHelper::siderItemClicked(quint64 windowId, const QUrl &url)
{
    winID = windowId;
    switch (state(vaultLockPath())) {
    case VaultState::kNotExisted: {
        VaultPageBase *page = new VaultActiveView();
        page->exec();
    } break;
    case VaultState::kEncrypted: {
        VaultPageBase *page = new VaultUnlockPages();
        page->exec();
    } break;
    case VaultState::kUnlocked:
        defaultCdAction(url);
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
            qCritical() << "Failed, init sidebar \"sideBarService\" is empty";
            abort();
        }
    }
    return vaultService;
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

QMenu *VaultHelper::createMenu()
{
    QMenu *menu = new QMenu;
    switch (state(vaultLockPath())) {
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

        menu->addAction(QObject::tr("Time Lock"), []() {

        });

        menu->addSeparator();

        menu->addAction(QObject::tr("Delete Vault"), VaultHelper::instance(), &VaultHelper::removeVaultDialog);

        menu->addAction(QObject::tr("Property"), []() {

        });
    } break;
    case VaultState::kUnderProcess:
    case VaultState::kBroken:
    case VaultState::kNotAvailable:
        break;
    }

    return menu;
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

void VaultHelper::newOpenWindow()
{
    QUrl url;
    url.setScheme(VaultHelper::scheme());
    url.setPath("/");
    url.setHost("");
    openNewWindow(rootUrl());
}

void VaultHelper::slotlockVault(int state)
{
    if (state == 0) {
        emit VaultHelper::instance()->sigLocked(state);
        QUrl url;
        url.setScheme(QString("computer"));
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
