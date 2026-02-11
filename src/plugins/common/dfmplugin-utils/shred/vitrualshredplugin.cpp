// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vitrualshredplugin.h"
#include "shredutils.h"
#include "shredmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/utils/dialogmanager.h>

#define SHRED_SETTING_GROUP "10_advance.04_shred"
inline constexpr char kShredSettingGroup[] { SHRED_SETTING_GROUP };
inline constexpr char kShredSettingShred[] { "00_file_shred" };

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

void VirtualShredPlugin::initialize()
{
}

bool VirtualShredPlugin::start()
{
    if (DPF_NAMESPACE::LifeCycle::isAllPluginsStarted())
        onAllPluginsStarted();
    else
        connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted, this, &VirtualShredPlugin::onAllPluginsStarted);

    ShredUtils::instance()->initDconfig();
    addShredSettingItem();
    return true;
}

void VirtualShredPlugin::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(ShredMenuCreator::name(), parentScene);
    } else {
        menuScenes << parentScene;
        if (!subscribedEvent)
            subscribedEvent = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &VirtualShredPlugin::onMenuSceneAdded);
    }
}

void VirtualShredPlugin::addShredSettingItem()
{
    SettingJsonGenerator::instance()->addGroup(kShredSettingGroup, tr("File shred"));
    DialogManager::instance()->registerSettingWidget("shredItem", &ShredUtils::createShredSettingItem);
    QVariantMap config {
        { "key", kShredSettingShred },
        { "type", "shredItem" },
        { "default", false }
    };

    QString key = QString("%1.%2").arg(kShredSettingGroup, kShredSettingShred);
    SettingJsonGenerator::instance()->addConfig(key, config);
}

void VirtualShredPlugin::onAllPluginsStarted()
{
    dfmplugin_menu_util::menuSceneRegisterScene(ShredMenuCreator::name(), new ShredMenuCreator);
    bindScene("FileOperatorMenu");
}

void VirtualShredPlugin::onMenuSceneAdded(const QString &scene)
{
    if (menuScenes.contains(scene)) {
        menuScenes.remove(scene);
        dfmplugin_menu_util::menuSceneBind(ShredMenuCreator::name(), scene);

        if (menuScenes.isEmpty()) {
            dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &VirtualShredPlugin::onMenuSceneAdded);
            subscribedEvent = false;
        }
    }
}
