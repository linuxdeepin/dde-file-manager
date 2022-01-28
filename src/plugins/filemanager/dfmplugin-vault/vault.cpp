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
#include "vault.h"

#include "utils/vaultfileinfo.h"
#include "utils/vaultfileiterator.h"
#include "utils/vaultfilewatcher.h"
#include "utils/vaulthelper.h"
#include "utils/vaultglobaldefine.h"
#include "utils/vaultentryfileentity.h"
#include "events/vaulteventreceiver.h"

#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/computer/computerservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/windows/windowsservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/file/entry/entities/abstractentryfileentity.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
namespace GlobalPrivateService {
static SideBarService *sideBarService { nullptr };
static WorkspaceService *workspaceService { nullptr };
static ComputerService *computerService { nullptr };
static TitleBarService *titleBarService { nullptr };
static WindowsService *windowsService { nullptr };
}   // namespace GlobalPrivate

void Vault::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(SideBarService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(WorkspaceService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(ComputerService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(TitleBarService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(WindowsService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    GlobalPrivateService::sideBarService = ctx.service<SideBarService>(SideBarService::name());
    if (!GlobalPrivateService::sideBarService) {
        qCritical() << "Failed, init sidebar \"sideBarService\" is empty";
        abort();
    }

    GlobalPrivateService::computerService = ctx.service<ComputerService>(ComputerService::name());
    if (!GlobalPrivateService::computerService) {
        qCritical() << "Failed, init computer \"computerService\" is empty";
        abort();
    }

    GlobalPrivateService::titleBarService = ctx.service<TitleBarService>(TitleBarService::name());

    if (!GlobalPrivateService::titleBarService) {
        qCritical() << "Failed, init titleBar \"titleBarService\" is empty";
        abort();
    }

    GlobalPrivateService::windowsService = ctx.service<WindowsService>(WindowsService::name());

    if (!GlobalPrivateService::windowsService) {
        qCritical() << "Failed, init workspace \"windowsService\" is empty";
        abort();
    }

    if (VaultHelper::state(VaultHelper::vaultLockPath()) == VaultState::kUnlocked) {
        UrlRoute::regScheme(VaultHelper::scheme(), VaultHelper::rootUrl().path(), VaultHelper::icon(), false, tr("My Vault"));
    }

    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<VaultFileInfo>(VaultHelper::scheme());
    WacherFactory::regClass<VaultFileWatcher>(VaultHelper::scheme());
    DirIteratorFactory::regClass<VaultFileIterator>(VaultHelper::scheme());
    EntryEntityFactor::registCreator<VaultEntryFileEntity>("vault");
}

bool Vault::start()
{
    auto &ctx = dpfInstance.serviceContext();
    GlobalPrivateService::workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());

    if (!GlobalPrivateService::workspaceService) {
        qCritical() << "Failed, init workspace \"workspaceService\" is empty";
        abort();
    }
    GlobalPrivateService::workspaceService->addScheme(VaultHelper::scheme());
    connect(GlobalPrivateService::windowsService, &WindowsService::windowOpened, this, &Vault::onWindowOpened, Qt::DirectConnection);
    VaultEventReceiver::instance()->connectEvent();
    return true;
}

dpf::Plugin::ShutdownFlag Vault::stop()
{
    return kSync;
}

void Vault::onWindowOpened(quint64 winID)
{
    auto window = GlobalPrivateService::windowsService->findWindowById(winID);
    if (window->titleBar())
        addCustomCrumbar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Vault::addCustomCrumbar, Qt::DirectConnection);
    if (window->sideBar()) {
        addSideBarVaultItem();
        addComputer();
    } else {
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Vault::addSideBarVaultItem, Qt::DirectConnection);
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Vault::addComputer, Qt::DirectConnection);
    }
}

void Vault::addSideBarVaultItem()
{
    bool vaultEnabled = VaultHelper::isVaultEnabled();
    if (vaultEnabled) {
        SideBar::ItemInfo item;
        item.group = SideBar::DefaultGroup::kDevice;
        QUrl url;
        url.setScheme(VaultHelper::scheme());
        url.setPath(VaultHelper::rootUrl().path());
        url.setHost("");
        item.url = VaultHelper::rootUrl();
        item.iconName = VaultHelper::icon().name();
        item.text = tr("My Vault");
        item.flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        item.contextMenuCb = VaultHelper::contenxtMenuHandle;
        item.cdCb = VaultHelper::siderItemClicked;

        GlobalPrivateService::sideBarService->insertItem(1, item);
    }
}

void Vault::addCustomCrumbar()
{
    bool vaultEnabled = VaultHelper::isVaultEnabled();
    if (vaultEnabled) {
        TitleBar::CustomCrumbInfo crumb;
        crumb.scheme = VaultHelper::scheme();
        crumb.keepAddressBar = false;
        crumb.seperateCb = VaultHelper::seprateUrl;
        GlobalPrivateService::titleBarService->addCustomCrumbar(crumb);
    }
}

void Vault::addComputer()
{
    bool vaultEnabled = VaultHelper::isVaultEnabled();
    if (vaultEnabled)
        GlobalPrivateService::computerService->addDevice(tr("Vault"), QUrl("entry:///vault.vault"));
}

void Vault::removeSideBarVaultItem()
{
    QUrl url;
    url.setScheme(VaultHelper::scheme());
    url.setPath("/");
    GlobalPrivateService::sideBarService->removeItem(url);
}
