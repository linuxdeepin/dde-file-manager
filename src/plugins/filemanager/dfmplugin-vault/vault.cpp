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
#include "utils/vaulthelper.h"
#include "utils/vaultfileiterator.h"
#include "utils/vaultfilewatcher.h"
#include "utils/vaultglobaldefine.h"
#include "utils/vaultentryfileentity.h"
#include "events/vaulteventreceiver.h"
#include "events/vaulteventcaller.h"

#include "services/common/delegate/delegateservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/file/entry/entities/abstractentryfileentity.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

void Vault::initialize()
{
    UrlRoute::regScheme(VaultHelper::instance()->scheme(), "/", VaultHelper::instance()->icon(), true, tr("My Vault"));

    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<VaultFileInfo>(VaultHelper::instance()->scheme());
    WatcherFactory::regClass<VaultFileWatcher>(VaultHelper::instance()->scheme());
    DirIteratorFactory::regClass<VaultFileIterator>(VaultHelper::instance()->scheme());
    EntryEntityFactor::registCreator<VaultEntryFileEntity>("vault");
}

bool Vault::start()
{
    addFileOperations();

    VaultHelper::workspaceServiceInstance()->addScheme(VaultHelper::instance()->scheme());
    connect(VaultHelper::windowServiceInstance(), &WindowsService::windowOpened, this, &Vault::onWindowOpened, Qt::DirectConnection);
    VaultEventReceiver::instance()->connectEvent();

    propertyServIns->registerMethod(VaultHelper::createVaultPropertyDialog, VaultHelper::instance()->scheme());
    propertyServIns->registerPropertyPathShowStyle(VaultHelper::instance()->scheme());
    VaultEventCaller::sendBookMarkDisabled(VaultHelper::instance()->scheme());
    return true;
}

dpf::Plugin::ShutdownFlag Vault::stop()
{
    return kSync;
}

void Vault::onWindowOpened(quint64 winID)
{
    delegateServIns->registerUrlTransform(VaultHelper::instance()->scheme(), VaultHelper::vaultToLocalUrl);

    auto window = VaultHelper::windowServiceInstance()->findWindowById(winID);

    if (window->sideBar())
        addSideBarVaultItem();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Vault::addSideBarVaultItem, Qt::DirectConnection);

    if (window->workSpace())
        addComputer();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &Vault::addComputer, Qt::DirectConnection);
}

void Vault::addSideBarVaultItem()
{
    bool vaultEnabled = VaultHelper::instance()->isVaultEnabled();
    if (vaultEnabled) {
        SideBar::ItemInfo item;
        item.group = SideBar::DefaultGroup::kDevice;
        item.url = VaultHelper::instance()->rootUrl();
        item.iconName = VaultHelper::instance()->icon().name();
        item.text = tr("My Vault");
        item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        item.contextMenuCb = VaultHelper::contenxtMenuHandle;
        item.cdCb = VaultHelper::siderItemClicked;

        VaultHelper::sideBarServiceInstance()->insertItem(1, item);
    }
}

void Vault::addComputer()
{
    bool vaultEnabled = VaultHelper::instance()->isVaultEnabled();
    if (vaultEnabled)
        VaultHelper::computerServiceInstance()->addDevice(tr("Vault"), QUrl("entry:///vault.vault"));
}

void Vault::addFileOperations()
{
    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &VaultHelper::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &VaultHelper::writeToClipBoardHandle;
    fileOpeationsHandle->moveToTash = &VaultHelper::moveToTrashHandle;
    fileOpeationsHandle->deletes = &VaultHelper::deletesHandle;
    fileOpeationsHandle->copy = &VaultHelper::copyHandle;
    fileOpeationsHandle->cut = &VaultHelper::cutHandle;
    fileOpeationsHandle->makeDir = &VaultHelper::mkdirHandle;
    fileOpeationsHandle->touchFile = &VaultHelper::touchFileHandle;
    fileOpeationsHandle->renameFile = &VaultHelper::renameHandle;
    VaultHelper::fileOperationsServIns()->registerOperations(VaultHelper::instance()->scheme(), fileOpeationsHandle);
}
