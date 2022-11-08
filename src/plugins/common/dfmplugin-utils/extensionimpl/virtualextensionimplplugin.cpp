/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "virtualextensionimplplugin.h"
#include "menuimpl/extensionlibmenuscene.h"
#include "pluginsload/extensionpluginmanager.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

namespace dfmplugin_utils {

void VirtualExtensionImplPlugin::initialize()
{
    auto manager = &ExtensionPluginManager::instance();   // run in main thread
    connect(manager, &ExtensionPluginManager::requestInitlaizePlugins, manager, &ExtensionPluginManager::onLoadingPlugins);
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

}   // namespace dfmplugin_utils
