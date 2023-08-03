// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include "utils/policy/policymanager.h"
#include "utils/servicemanager.h"
#include "menus/vaultmenuscene.h"
#include "menus/vaultcomputermenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/file/entry/entities/abstractentryfileentity.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

#include <DSysInfo>

using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;
using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
using ItemClickedActionCallback = std::function<void(quint64 windowId, const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
Q_DECLARE_METATYPE(ItemClickedActionCallback);
Q_DECLARE_METATYPE(ContextMenuCallback);
Q_DECLARE_METATYPE(CustomViewExtensionView)
Q_DECLARE_METATYPE(BasicViewFieldFunc)

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultVisibleManager::VaultVisibleManager(QObject *parent)
    : QObject(parent)
{
}

bool VaultVisibleManager::isVaultEnabled()
{
    return PolicyManager::isVaultVisiable();
}

void VaultVisibleManager::infoRegister()
{
    PolicyManager::instance()->slotVaultPolicy();
    if (isVaultEnabled() && !infoRegisterState) {
        UrlRoute::regScheme(VaultHelper::instance()->scheme(), "/", VaultHelper::instance()->icon(), true, tr("My Vault"));

        //注册Scheme为"vault"的扩展的文件信息
        InfoFactory::regClass<VaultFileInfo>(VaultHelper::instance()->scheme());
        WatcherFactory::regClass<VaultFileWatcher>(VaultHelper::instance()->scheme(), WatcherFactory::kNoCache);
        DirIteratorFactory::regClass<VaultFileIterator>(VaultHelper::instance()->scheme());
        EntryEntityFactor::registCreator<VaultEntryFileEntity>("vault");
        infoRegisterState = true;
    }
}

void VaultVisibleManager::pluginServiceRegister()
{
    if (!isVaultEnabled())
        return;

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", VaultHelper::instance()->scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", VaultHelper::instance()->scheme(), VaultMenuSceneCreator::name());

    CustomViewExtensionView customView { VaultHelper::createVaultPropertyDialog };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_CustomView_Register",
                         customView, VaultHelper::instance()->scheme());

    BasicViewFieldFunc func { ServiceManager::basicViewFieldFunc };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicViewExtension_Register",
                         func, VaultHelper::instance()->scheme());
    BasicViewFieldFunc detailViewFunc { ServiceManager::detailViewFieldFunc };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicViewExtension_Register",
                         detailViewFunc, VaultHelper::instance()->scheme());
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicViewExtension_Root_Register",
                         detailViewFunc, VaultHelper::instance()->scheme());

    QStringList &&filtes { kFileChangeTimeField, kFileSizeField };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         VaultHelper::instance()->sourceRootUrlWithSlash().path(), filtes);
    auto computerPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-computer") };
    if (computerPlugin && computerPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        addVaultComputerMenu();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-computer")
                        addVaultComputerMenu();
                },
                Qt::DirectConnection);
    }
    dpfSignalDispatcher->subscribe("dfmplugin_computer", "signal_View_Refreshed",
                                   VaultVisibleManager::instance(), &VaultVisibleManager::onComputerRefresh);
}

void VaultVisibleManager::addVaultComputerMenu()
{
    dfmplugin_menu_util::menuSceneRegisterScene(VaultComputerMenuCreator::name(), new VaultComputerMenuCreator());
    bool ok = dfmplugin_menu_util::menuSceneBind(VaultComputerMenuCreator::name(), "ComputerMenu");
    if (!ok)
        qCritical() << "Vault: add vault computer menu failed";
    dfmplugin_menu_util::menuSceneRegisterScene(VaultMenuSceneCreator::name(), new VaultMenuSceneCreator);
}

void VaultVisibleManager::addSideBarVaultItem()
{
    if (isVaultEnabled()) {
        ItemClickedActionCallback cdCb { VaultHelper::siderItemClicked };
        ContextMenuCallback contextMenuCb { VaultHelper::contenxtMenuHandle };
        Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
        QVariantMap map {
            { "Property_Key_Group", "Group_Device" },
            { "Property_Key_DisplayName", tr("File Vault") },
            { "Property_Key_Icon", VaultHelper::instance()->icon() },
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCb) },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
            { "Property_Key_VisiableControl", "vault" },
            { "Property_Key_ReportName", "Vault" }
        };

        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Insert", 1, VaultHelper::instance()->rootUrl(), map);
    }
}

void VaultVisibleManager::addComputer()
{
    if (isVaultEnabled()) {
        dpfSlotChannel->push("dfmplugin_computer", "slot_Item_Add", tr("Vault"), QUrl("entry:///vault.vault"), 0, false);
    }
}

void VaultVisibleManager::onWindowOpened(quint64 winID)
{
    auto window = FMWindowsIns.findWindowById(winID);

    if (!window)
        return;

    if (window->sideBar())
        addSideBarVaultItem();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &VaultVisibleManager::addSideBarVaultItem, Qt::DirectConnection);

    if (window->workSpace())
        addComputer();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &VaultVisibleManager::addComputer, Qt::DirectConnection);

    VaultEventCaller::sendBookMarkDisabled(VaultHelper::instance()->scheme());
}

void VaultVisibleManager::removeSideBarVaultItem()
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", VaultHelper::instance()->rootUrl());
}

void VaultVisibleManager::removeComputerVaultItem()
{
    dpfSlotChannel->push("dfmplugin_computer", "slot_Item_Remove", QUrl("entry:///vault.vault"));
}

void VaultVisibleManager::onComputerRefresh()
{
    addComputer();
}

VaultVisibleManager *VaultVisibleManager::instance()
{
    static VaultVisibleManager obj;
    return &obj;
}
