/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "menuservice.h"
#include "private/menuservicehelper.h"

DSC_BEGIN_NAMESPACE

DFMBASE_USE_NAMESPACE

/*!
 * \brief MenuService::createMenu: Create a menu corresponding to the scene.
 * \param parent: The parent of the menu, used to delete the menu when the parent is deleted.
 * \param scene: The name of the scene where the menu needs to be created.
 * \param mode: Create a blank area or normal menu.
 * \param rootUrl: The root URL of the current scene
 * \param foucsUrl: The URL of the focus file where the mouse is when the right-click menu is located.
 * \param selected: The list of file urls selected when the right-click menu is displayed.
 * \param flags: Right-click menu selection mode (eg: single file, single folder,
 *               multiple files, multiple folders, files and folders, etc.)
 * \param customData: some custom data when creating the menu (if needed).
 * \return Return menu
 */
QMenu *MenuService::createMenu(QWidget *parent,
                               const QString &scene,
                               DFMBASE_NAMESPACE::AbstractMenu::MenuMode mode,
                               const QUrl &rootUrl,
                               const QUrl &foucsUrl,
                               const QList<QUrl> selected,
                               bool onDesktop,
                               ExtensionFlags flags,
                               QVariant customData)
{
    auto topClass = DFMBASE_NAMESPACE::MenuFactory::create(scene);
    if (!topClass)
        return nullptr;

    // 获取场景对应前置菜单
    auto tempMenu = topClass->build(parent, mode, rootUrl, foucsUrl, selected, customData);

    // TODO: 暂时直接从menu获取再排序，后续再考虑actions

    // template菜单
    if (flags.testFlag(DFMBASE_NAMESPACE::kTemplateAction)) {
        MenuServiceHelper::templateMenu(tempMenu);
    }

    // 添加oem菜单
    if (flags.testFlag(DFMBASE_NAMESPACE::kDesktopAction)) {
        MenuServiceHelper::desktopFileMenu(tempMenu);
    }

    // 添加conf菜单
    if (flags.testFlag(DFMBASE_NAMESPACE::kConfAction)) {
        MenuServiceHelper::extendCustomMenu(tempMenu, mode, rootUrl, foucsUrl, selected);
    }

    // 添加第三方扩展so菜单
    if (flags.testFlag(DFMBASE_NAMESPACE::kSoAction)) {
        MenuServiceHelper::extensionPluginCustomMenu(tempMenu, mode, rootUrl, foucsUrl, selected);
    }

    return tempMenu;
}

MenuService::MenuService(QObject *parent)
    : dpf::PluginService(parent), dpf::AutoServiceRegister<MenuService>()
{
}

MenuService::~MenuService()
{
}

DSC_END_NAMESPACE
