// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultvisiblemanager.h"
#include "fileutils/vaultfileinfo.h"
#include "utils/vaulthelper.h"
#include "fileutils/vaultfileiterator.h"
#include "fileutils/vaultfilewatcher.h"
#include "utils/vaultdefine.h"
#include "utils/vaultentryfileentity.h"
#include "events/vaulteventreceiver.h"
#include "events/vaulteventcaller.h"
#include "utils/servicemanager.h"
#include "menus/vaultmenuscene.h"
#include "menus/vaultcomputermenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>

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

void VaultVisibleManager::infoRegister()
{
    if (!infoRegisterState) {
        UrlRoute::regScheme(VaultHelper::instance()->scheme(), "/", VaultHelper::instance()->icon(), true, tr("File Vault"));

        // 注册Scheme为"vault"的扩展的文件信息
        InfoFactory::regClass<VaultFileInfo>(VaultHelper::instance()->scheme());
        WatcherFactory::regClass<VaultFileWatcher>(VaultHelper::instance()->scheme(), WatcherFactory::kNoCache);
        DirIteratorFactory::regClass<VaultFileIterator>(VaultHelper::instance()->scheme());
        infoRegisterState = true;
    }
}

void VaultVisibleManager::pluginServiceRegister()
{
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
}

void VaultVisibleManager::addVaultComputerMenu()
{
    dfmplugin_menu_util::menuSceneRegisterScene(VaultComputerMenuCreator::name(), new VaultComputerMenuCreator());
    bool ok = dfmplugin_menu_util::menuSceneBind(VaultComputerMenuCreator::name(), "ComputerMenu");
    if (!ok)
        fmCritical() << "Vault: add vault computer menu failed";
    dfmplugin_menu_util::menuSceneRegisterScene(VaultMenuSceneCreator::name(), new VaultMenuSceneCreator);
}

void VaultVisibleManager::updateSideBarVaultItem()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        ItemClickedActionCallback cdCb { VaultHelper::siderItemClicked };
        ContextMenuCallback contextMenuCb { VaultHelper::contenxtMenuHandle };
        Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
        QVariantMap map {
            { "Property_Key_DisplayName", tr("File Vault") },
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCb) },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) }
        };

        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", VaultHelper::instance()->rootUrl(), map);
    });
}

void VaultVisibleManager::onWindowOpened(quint64 winID)
{
    fmDebug() << "Vault: Window opened event received, window ID:" << winID;
    auto window = FMWindowsIns.findWindowById(winID);

    if (!window) {
        fmDebug() << "Vault: Window not found for ID:" << winID;
        return;
    }

    if (window->sideBar()) {
        fmDebug() << "Vault: Sidebar already available, updating vault item";
        updateSideBarVaultItem();
    } else {
        fmDebug() << "Vault: Sidebar not ready, connecting to install finished signal";
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &VaultVisibleManager::updateSideBarVaultItem, Qt::DirectConnection);
    }
}

void VaultVisibleManager::removeSideBarVaultItem()
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", VaultHelper::instance()->rootUrl());
}

void VaultVisibleManager::removeComputerVaultItem()
{
    dpfSlotChannel->push("dfmplugin_computer", "slot_Item_Remove", QUrl("entry:///vault.vault"));
}

VaultVisibleManager *VaultVisibleManager::instance()
{
    static VaultVisibleManager obj;
    return &obj;
}
