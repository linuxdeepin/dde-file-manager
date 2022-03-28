/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "menu.h"
#include "menuScene/clipboardmenuscene.h"
#include "menuScene/opendirmenuscene.h"
#include "menuScene/openfilemenuscene.h"
#include "menuScene/openwithmenuscene.h"
#include "menuScene/newcreatemenuscene.h"
#include "menuScene/sendtomenuscene.h"

#include <services/common/menu/menuservice.h>

#include <QWidget>

DPMENU_USE_NAMESPACE
DSC_USE_NAMESPACE

void Menu::initialize()
{
}

bool Menu::start()
{
    auto &ctx = dpfInstance.serviceContext();

    // start menu service.
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        QString error;
        bool ret = ctx.load(MenuService::name(), &error);
        Q_ASSERT_X(ret, "Menu Plugin ", error.toStdString().c_str());
    });

    menuServer = ctx.service<MenuService>(MenuService::name());
    Q_ASSERT_X(menuServer, "Menu Plugin", "MenuService not found");

    this->regDefaultScene();

    return true;
}

dpf::Plugin::ShutdownFlag Menu::stop()
{
    return kSync;
}

void Menu::regDefaultScene()
{
    // 注册新建场景
    menuServer->registerScene(NewCreateMenuCreator::name(), new NewCreateMenuCreator);

    // 注册剪切板场景
    menuServer->registerScene(ClipBoardMenuCreator::name(), new ClipBoardMenuCreator);

    // 注册文件夹场景
    menuServer->registerScene(OpenDirMenuCreator::name(), new OpenDirMenuCreator);

    // 注册文件场景
    menuServer->registerScene(OpenFileMenuCreator::name(), new OpenFileMenuCreator);

    menuServer->registerScene(SendToMenuCreator::name(), new SendToMenuCreator);
}
