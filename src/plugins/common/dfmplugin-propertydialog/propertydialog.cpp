// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertydialog.h"
#include "events/propertyeventreceiver.h"
#include "menu/propertymenuscene.h"
#include "utils/propertydialogmanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

namespace dfmplugin_propertydialog {
DFM_LOG_REGISTER_CATEGORY(DPPROPERTYDIALOG_NAMESPACE)

void PropertyDialog::initialize()
{
    PropertyEventReceiver::instance()->bindEvents();
}

bool PropertyDialog::start()
{
    PropertyDialogManager::instance().addComputerPropertyDialog();

    dfmplugin_menu_util::menuSceneRegisterScene(PropertyMenuCreator::name(), new PropertyMenuCreator);
    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");
    return true;
}

void PropertyDialog::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(PropertyMenuCreator::name(), parentScene);
    } else {
        waitToBind << parentScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &PropertyDialog::bindSceneOnAdded);
    }
}

void PropertyDialog::bindSceneOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &PropertyDialog::bindSceneOnAdded);
        bindScene(newScene);
    }
}
}   // namespace dfmplugin_propertydialog
