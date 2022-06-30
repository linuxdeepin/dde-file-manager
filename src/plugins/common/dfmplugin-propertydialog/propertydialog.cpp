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
#include "propertydialog.h"
#include "events/propertyeventreceiver.h"
#include "utils/propertydialoghelper.h"
#include "menu/propertymenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

void PropertyDialog::initialize()
{
    PropertyEventReceiver::instance()->connectService();
}

bool PropertyDialog::start()
{

    PropertyDialogHelper::propertyServiceInstance()->addComputerPropertyToPropertyService();

    dfmplugin_menu_util::menuSceneRegisterScene(PropertyMenuCreator::name(), new PropertyMenuCreator);
    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");
    return true;
}

dpf::Plugin::ShutdownFlag PropertyDialog::stop()
{
    return kSync;
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
