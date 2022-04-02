/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "fileviewmenuhelper.h"
#include "views/fileview.h"
#include "models/filesortfilterproxymodel.h"
#include "menus/workspacemenuscene.h"
#include "utils/workspacehelper.h"

#include "services/filemanager/workspace/workspace_defines.h"
#include "services/common/menu/menuservice.h"
#include "services/common/menu/menu_defines.h"
#include "dfm-framework/framework.h"

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileViewMenuHelper::FileViewMenuHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

void FileViewMenuHelper::showEmptyAreaMenu()
{
    auto scene = MenuService::service()->createScene(currentMenuScene());
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->rootUrl();
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;

    if (!scene->initialize(params)) {
        delete scene;
        return;
    }

    QMenu menu(this->view);
    scene->create(&menu);
    scene->updateState(&menu);

    QAction *act = menu.exec(QCursor::pos());
    if (act)
        scene->triggered(act);
    delete scene;
}

void FileViewMenuHelper::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    Q_UNUSED(indexFlags)

    auto scene = MenuService::service()->createScene(currentMenuScene());
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->rootUrl();
    params[MenuParamKey::kFocusFile] = view->model()->itemFileInfo(index)->url();
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(view->selectedUrlList());
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = false;

    if (!scene->initialize(params)) {
        delete scene;
        return;
    }

    QMenu menu(this->view);
    scene->create(&menu);
    scene->updateState(&menu);

    QAction *act = menu.exec(QCursor::pos());
    if (act)
        scene->triggered(act);
    delete scene;
}

QString FileViewMenuHelper::currentMenuScene() const
{
    QString scene = WorkspaceHelper::instance()->findMenuScene(view->rootUrl().scheme());
    return scene.isEmpty() ? WorkspaceMenuCreator::name() : scene;
}
