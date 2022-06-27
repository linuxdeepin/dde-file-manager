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
#include "vaultvisiblemanager.h"
#include "utils/policy/policymanager.h"
#include "fileutils/vaultfileinfo.h"
#include "utils/vaulthelper.h"
#include "fileutils/vaultfileiterator.h"
#include "fileutils/vaultfilewatcher.h"
#include "utils/vaultdefine.h"
#include "utils/vaultentryfileentity.h"
#include "events/vaulteventreceiver.h"
#include "events/vaulteventcaller.h"
#include "utils/filemanipulation.h"
#include "utils/policy/policymanager.h"
#include "utils/servicemanager.h"
#include "menus/vaultmenuscene.h"
#include "menus/vaultcomputermenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/common/delegate/delegateservice.h"
#include "services/common/propertydialog/propertydialogservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/file/entry/entities/abstractentryfileentity.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>
#include <dfm-framework/event/event.h>

#include <DSysInfo>

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace dfmplugin_vault;
VaultVisibleManager::VaultVisibleManager(QObject *parent)
    : QObject(parent)
{
}

bool VaultVisibleManager::isVaultEnabled()
{
    if (!DSysInfo::isCommunityEdition()) {   // 如果不是社区版
        DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
        // 如果是专业版
        if (DSysInfo::DeepinType::DeepinProfessional == deepinType && PolicyManager::isVaultVisiable()) {
            return true;
        }
    }
    return false;
}

void VaultVisibleManager::infoRegister()
{
    PolicyManager::instance()->slotVaultPolicy();
    if (isVaultEnabled() && !infoRegisterState) {
        UrlRoute::regScheme(VaultHelper::instance()->scheme(), "/", VaultHelper::instance()->icon(), true, tr("My Vault"));

        //注册Scheme为"vault"的扩展的文件信息
        InfoFactory::regClass<VaultFileInfo>(VaultHelper::instance()->scheme());
        WatcherFactory::regClass<VaultFileWatcher>(VaultHelper::instance()->scheme());
        DirIteratorFactory::regClass<VaultFileIterator>(VaultHelper::instance()->scheme());
        EntryEntityFactor::registCreator<VaultEntryFileEntity>("vault");
        infoRegisterState = true;
    }
}

void VaultVisibleManager::pluginServiceRegister()
{
    if (!serviceRegisterState) {
        connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &VaultVisibleManager::onWindowOpened, Qt::DirectConnection);
        VaultEventReceiver::instance()->connectEvent();
        serviceRegisterState = true;
    }

    if (isVaultEnabled()) {
        ServiceManager::workspaceServiceInstance()->addScheme(VaultHelper::instance()->scheme());

        propertyServIns->registerCustomizePropertyView(VaultHelper::createVaultPropertyDialog, VaultHelper::instance()->scheme());
        propertyServIns->registerBasicViewFiledExpand(ServiceManager::basicViewFieldFunc, VaultHelper::instance()->scheme());
        propertyServIns->registerFilterControlField(VaultHelper::instance()->scheme(), Property::FilePropertyControlFilter::kPermission);

        delegateServIns->registerUrlTransform(VaultHelper::instance()->scheme(), VaultHelper::vaultToLocalUrl);

        dfmplugin_menu_util::menuSceneRegisterScene(VaultComputerMenuCreator::name(), new VaultComputerMenuCreator());
        dfmplugin_menu_util::menuSceneBind(VaultComputerMenuCreator::name(), "ComputerMenu");
        dfmplugin_menu_util::menuSceneRegisterScene(VaultMenuSceneCreator::name(), new VaultMenuSceneCreator);
        WorkspaceService::service()->setWorkspaceMenuScene(VaultHelper::instance()->scheme(), VaultMenuSceneCreator::name());
    }
}

void VaultVisibleManager::addSideBarVaultItem()
{
    if (isVaultEnabled()) {
        SideBar::ItemInfo item;
        item.group = SideBar::DefaultGroup::kDevice;
        item.url = VaultHelper::instance()->rootUrl();
        item.iconName = VaultHelper::instance()->icon().name();
        item.text = tr("My Vault");
        item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        item.contextMenuCb = VaultHelper::contenxtMenuHandle;
        item.cdCb = VaultHelper::siderItemClicked;

        ServiceManager::sideBarServiceInstance()->insertItem(1, item);
    }
}

void VaultVisibleManager::addComputer()
{
    if (isVaultEnabled()) {
        dpfSlotChannel->push("dfmplugin_computer", "slot_AddDevice", tr("Vault"), QUrl("entry:///vault.vault"));
    }
}

void VaultVisibleManager::addFileOperations()
{
    if (isVaultEnabled()) {
        FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
        fileOpeationsHandle->openFiles = &FileManipulation::openFilesHandle;
        fileOpeationsHandle->writeUrlsToClipboard = &FileManipulation::writeToClipBoardHandle;
        fileOpeationsHandle->moveToTash = &FileManipulation::moveToTrashHandle;
        fileOpeationsHandle->deletes = &FileManipulation::deletesHandle;
        fileOpeationsHandle->copy = &FileManipulation::copyHandle;
        fileOpeationsHandle->cut = &FileManipulation::cutHandle;
        fileOpeationsHandle->makeDir = &FileManipulation::mkdirHandle;
        fileOpeationsHandle->touchFile = &FileManipulation::touchFileHandle;
        fileOpeationsHandle->renameFile = &FileManipulation::renameHandle;
        fileOpeationsHandle->renameFiles = &FileManipulation::renameFilesHandle;
        fileOpeationsHandle->renameFilesAddText = &FileManipulation::renameFilesHandleAddText;
        ServiceManager::fileOperationsServIns()->registerOperations(VaultHelper::instance()->scheme(), fileOpeationsHandle);
    }
}

void VaultVisibleManager::onWindowOpened(quint64 winID)
{
    auto window = FMWindowsIns.findWindowById(winID);

    if (window->sideBar())
        addSideBarVaultItem();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &VaultVisibleManager::addSideBarVaultItem, Qt::DirectConnection);

    if (window->workSpace())
        addComputer();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &VaultVisibleManager::addComputer, Qt::DirectConnection);

    addFileOperations();
    VaultEventCaller::sendBookMarkDisabled(VaultHelper::instance()->scheme());
}

void VaultVisibleManager::removeSideBarVaultItem()
{
    ServiceManager::sideBarServiceInstance()->removeItem(VaultHelper::instance()->rootUrl());
}

void VaultVisibleManager::removeComputerVaultItem()
{
    dpfSlotChannel->push("dfmplugin_computer", "slot_RemoveDevice", QUrl("entry:///vault.vault"));
}

VaultVisibleManager *VaultVisibleManager::instance()
{
    static VaultVisibleManager obj;
    return &obj;
}
