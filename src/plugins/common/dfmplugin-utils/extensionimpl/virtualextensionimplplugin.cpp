// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualextensionimplplugin.h"
#include "menuimpl/extensionlibmenuscene.h"
#include "emblemimpl/extensionemblemmanager.h"
#include "windowimpl/extensionwindowsmanager.h"
#include "fileimpl/extensionfilemanager.h"
#include "pluginsload/extensionpluginmanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

Q_DECLARE_METATYPE(QList<QIcon> *)

namespace dfmplugin_utils {
DFMBASE_USE_NAMESPACE

void VirtualExtensionImplPlugin::initialize()
{
    auto manager = &ExtensionPluginManager::instance();   // run in main thread
    connect(manager, &ExtensionPluginManager::requestInitlaizePlugins, manager, &ExtensionPluginManager::onLoadingPlugins);
    ExtensionEmblemManager::instance().initialize();
    ExtensionWindowsManager::instance().initialize();
    ExtensionFileManager::instance().initialize();

    followEvents();
}

bool VirtualExtensionImplPlugin::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(ExtensionLibMenuSceneCreator::name(), new ExtensionLibMenuSceneCreator);
    bindScene("ExtendMenu");

    return true;
}

void VirtualExtensionImplPlugin::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(ExtensionLibMenuSceneCreator::name(), parentScene);
    } else {
        waitToBind << parentScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &VirtualExtensionImplPlugin::bindSceneOnAdded);
    }
}

void VirtualExtensionImplPlugin::bindSceneOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &VirtualExtensionImplPlugin::bindSceneOnAdded);
        bindScene(newScene);
    }
}

void VirtualExtensionImplPlugin::followEvents()
{
    // `dfmplugin-emblem` is a lazy loedded plugin, cannot follow it whene current init
    auto eventID { DPF_NAMESPACE::Event::instance()->eventType("dfmplugin_emblem", "hook_ExtendEmblems_Fetch") };
    if (eventID != DPF_NAMESPACE::EventTypeScope::kInValid) {
        dpfHookSequence->follow("dfmplugin_emblem", "hook_ExtendEmblems_Fetch",
                                &ExtensionEmblemManager::instance(), &ExtensionEmblemManager::onFetchCustomEmblems);
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this,
                [](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-emblem")
                        dpfHookSequence->follow("dfmplugin_emblem", "hook_ExtendEmblems_Fetch", &ExtensionEmblemManager::instance(), &ExtensionEmblemManager::onFetchCustomEmblems);
                },
                Qt::DirectConnection);
    }
}

}   // namespace dfmplugin_utils
