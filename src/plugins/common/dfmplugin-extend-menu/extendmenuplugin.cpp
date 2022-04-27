/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "extendmenuplugin.h"
#include "extendMenuScene/extendmenuscene.h"
#include "extendMenuScene/extendMenu/dcustomactionparser.h"

#include <services/common/menu/menuservice.h>

#include <QWidget>

DPEXTENDMENU_USE_NAMESPACE
DSC_USE_NAMESPACE

void ExtendMenuPlugin::initialize()
{
    CustomParserIns->delayRefresh();
}

bool ExtendMenuPlugin::start()
{
    menuServer = MenuService::service();
    Q_ASSERT_X(menuServer, "Menu Plugin", "MenuService not found");

    this->regDefaultScene();

    return true;
}

dpf::Plugin::ShutdownFlag ExtendMenuPlugin::stop()
{
    return kSync;
}

void ExtendMenuPlugin::regDefaultScene()
{
    menuServer->registerScene(ExtendMenuCreator::name(), new ExtendMenuCreator);

    bindScene("CanvasMenu");
    bindScene("WorkspaceMenu");
}

void ExtendMenuPlugin::bindScene(const QString &parentScene)
{
    if (MenuService::service()->contains(parentScene)) {
        MenuService::service()->bind(ExtendMenuCreator::name(), parentScene);
    } else {
        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
            if (scene == parentScene)
                MenuService::service()->bind(ExtendMenuCreator::name(), scene);
        },
                Qt::DirectConnection);
    }
}
